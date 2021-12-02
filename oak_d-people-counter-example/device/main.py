import argparse
import time
import queue
import signal
import threading
import os
import asyncio
import json
import configparser
import cv2
import depthai as dai
import base64
import logging
import numpy as np
from datetime import datetime
from pathlib import Path
from astarte.astarte import device
from flask import Flask, send_from_directory
from flask_socketio import SocketIO

parser = argparse.ArgumentParser()
parser.add_argument('-vid', '--video', type=str, help="Path to video file to be used for inference")
parser.add_argument('-w', '--width', default=1280, type=int, help="Visualization width. Height is calculated automatically from aspect ratio")
parser.add_argument('-dc', '--detconf', default=0.75, type=float, help="[UNUSED] Minimum detection confidence to for person in the scene. Default is 0.75")
parser.add_argument("-sf", "--settingsfile", default="settings.ini", type=str, help="TODO")
parser.add_argument("-ss", "--settingssection", default="demo.clea.cloud", type=str, help="TODO")
args    = parser.parse_args()


# Arguments parsing
camera                  = not args.video
detection_confidence    = args.detconf
settings_file           = args.settingsfile
settings_section        = args.settingssection
config                  = configparser.ConfigParser ()
config.read (settings_file)


color_picker            = {
    0: (0,255,255),
    1: (0,255,0),
    2: (0,0,255),
    3: (255,0,255)
}


def cos_dist(a, b):
    return np.dot(a, b) / (np.linalg.norm(a) * np.linalg.norm(b))

def frame_norm(frame, bbox):
    return (np.clip(np.array(bbox), 0, 1) * np.array([*frame.shape[:2], *frame.shape[:2]])[::-1]).astype(int)

def to_planar(arr: np.ndarray, shape: tuple) -> np.ndarray:
    resized = cv2.resize(arr, shape)
    return resized.transpose(2,0,1)

def create_pipeline():
    print("Creating pipeline...")
    pipeline = dai.Pipeline()
    pipeline.setOpenVINOVersion(version = dai.OpenVINO.Version.VERSION_2020_1)

    if camera:
        # ColorCamera
        print("Creating Color Camera...")
        cam = pipeline.createColorCamera()
        cam.setPreviewSize(544, 320)
        cam.setResolution(dai.ColorCameraProperties.SensorResolution.THE_1080_P)
        cam.setInterleaved(False)
        cam.setBoardSocket(dai.CameraBoardSocket.RGB)
        cam_xout = pipeline.createXLinkOut()
        cam_xout.setStreamName("cam_out")
        cam.video.link(cam_xout.input)

    # NeuralNetwork
    print("Creating Person Detection Neural Network...")
    detection_nn = pipeline.createMobileNetDetectionNetwork()
    detection_nn.setBlobPath(str(Path("models/person-detection-retail-0013_openvino_2020.1_4shave.blob").resolve().absolute()))
    # Confidence
    detection_nn.setConfidenceThreshold(0.7)
    # Increase threads for detection
    detection_nn.setNumInferenceThreads(2)
    # Specify that network takes latest arriving frame in non-blocking manner
    detection_nn.input.setQueueSize(1)
    detection_nn.input.setBlocking(False)

    detection_nn_xout = pipeline.createXLinkOut()
    detection_nn_xout.setStreamName("detection_nn")

    detection_nn_passthrough = pipeline.createXLinkOut()
    detection_nn_passthrough.setStreamName("detection_passthrough")
    detection_nn_passthrough.setMetadataOnly(True)

    if camera:
        print('linked cam.preview to detection_nn.input')
        cam.preview.link(detection_nn.input)
    else:
        detection_in = pipeline.createXLinkIn()
        detection_in.setStreamName("detection_in")
        detection_in.out.link(detection_nn.input)

    detection_nn.out.link(detection_nn_xout.input)
    detection_nn.passthrough.link(detection_nn_passthrough.input)


    # NeuralNetwork
    print("Creating Person Reidentification Neural Network...")
    reid_in = pipeline.createXLinkIn()
    reid_in.setStreamName("reid_in")
    reid_nn = pipeline.createNeuralNetwork()
    reid_nn.setBlobPath(str(Path("models/person-reidentification-retail-0031_openvino_2020.1_4shave.blob").resolve().absolute()))
    
    # Decrease threads for reidentification
    reid_nn.setNumInferenceThreads(1)
    
    reid_nn_xout = pipeline.createXLinkOut()
    reid_nn_xout.setStreamName("reid_nn")
    reid_in.out.link(reid_nn.input)
    reid_nn.out.link(reid_nn_xout.input)

    print("Pipeline created.")
    return pipeline




##########################
########   Main   ########
class Main:
    def __init__(self):

        self.running            = False
        self.FRAMERATE          = 30.0
        self.device_id          = config[settings_section]["DeviceId"]
        self.realm_name         = config[settings_section]["RealmName"]
        self.pairing_base_url   = config[settings_section]["PairingURL"]
        self.interface_name     = "ai.clea.examples.PeopleCounter"
        self.credentials_secret = config[settings_section]["CredentialSecret"]
        self.screen_zone_map    = {
            0   : "Desk area",          # top-left
            1   : "Coffee area",        # top-right
            2   : "Blackboard area",    # bottom-left
            3   : "Window area",        # bottom-right
        }

        if not camera:
            self.cap = cv2.VideoCapture(args.video)
            self.FRAMERATE = self.cap.get(cv2.CAP_PROP_FPS)

        # Creating queues
        self.frame_queue            = queue.Queue()
        self.visualization_queue    = queue.Queue()
        self.clea_if_queue          = queue.Queue ()
        
        # Creating astarte_device object
        self.astarte_loop           = asyncio.get_event_loop ()
        print ("Connecting to {}@{} as {} ({})".format (self.pairing_base_url, self.realm_name,
                                                        self.device_id, self.credentials_secret))
        self.astarte_device         = device.Device(self.device_id, self.realm_name,
                                                    self.credentials_secret, self.pairing_base_url,
                                                    os.path.curdir+'/astarte_persistence.d',
                                                    ignore_ssl_errors=True, loop=self.astarte_loop)
        self.astarte_device.on_connected                = self.astarte_conection_cb
        self.astarte_device.on_disconnected             = self.astarte_disconnection_cb
        self.astarte_device.on_data_received            = self.astarte_data_cb
        self.astarte_device.on_aggregate_data_received  = self.astarte_aggr_data_cb
        # Defining and adding the interface
        self.interface_descriptor   = { 
            "interface_name": "ai.clea.examples.PeopleCounter", 
            "version_major": 0,
            "version_minor": 1, 
            "type": "datastream",
            "ownership": "device",
            "aggregation": "object",
            "mappings": [ 
            {
                "endpoint": "/%{camera_id}/reading_timestamp",
                "type": "integer",
                "reliability":"unique",
                "retention":"volatile",
                "expiry":60,
                "database_retention_policy": "use_ttl",
                "database_retention_ttl": 28800
            },
            {
                "endpoint": "/%{camera_id}/people_count",
                "type": "integer",
                "reliability":"unique",
                "retention":"volatile",
                "expiry":60,
                "database_retention_policy": "use_ttl",
                "database_retention_ttl": 28800
            },
            {
                "endpoint": "/%{camera_id}/people",
                "type": "stringarray",
                "reliability":"unique",
                "retention":"volatile",
                "expiry":60,
                "database_retention_policy": "use_ttl",
                "database_retention_ttl": 28800
            }
            ]
        }
        self.astarte_device.add_interface (self.interface_descriptor)

        # Creating Flask webserver
        self.flask          = Flask(__name__)
        self.socket_io      = SocketIO(self.flask)
        self.last_b64_frame = None
        self.last_b64_mutex = threading.Lock ()
        #self.flask.logger.setLevel(logging.ERROR)
        logging.getLogger('werkzeug').setLevel(logging.CRITICAL)

        @self.flask.route ("/<path:path>")
        def index (path):
            return send_from_directory ("www", path)

        self.nn_fps = 0




    def is_running(self):
        if self.running:
            if camera:
                return True
            else:
                return self.cap.isOpened()
        return False




    def get_zone_id (self, frame, pt) :
        x       = pt[0]
        y       = pt[1]
        w       = frame.getWidth()
        h       = frame.getHeight()
        zone_id = -1
        if (x<w/2):
            if (y<h/2):
                zone_id = 0     # top-left
            else:
                zone_id = 2     # bottom-left
        else:
            if (y<h/2):
                zone_id = 1     # top-right
            else:
                zone_id = 3     # bottom-right

        return zone_id

    
    def inference_task(self):
        # Queues
        detection_passthrough   = self.device.getOutputQueue("detection_passthrough")
        detection_nn            = self.device.getOutputQueue("detection_nn")

        image_width     = -1
        image_height    = -1
        bboxes          = []
        results         = {}
        results_path    = {}
        next_id         = 0

        # Match up frames and detections
        try:
            prev_passthrough = detection_passthrough.getAll()[0]
            prev_inference = detection_nn.getAll()[0]
        except RuntimeError:
            pass

        fps = 0
        t_fps = time.time()
        while self.is_running():
            try:
                detected_people = []

                # Get current detection
                passthrough = detection_passthrough.getAll()[0]
                inference = detection_nn.getAll()[0]

                # Count NN fps
                fps = fps + 1

                # Combine all frames to current inference
                frames      = []
                raw_frames  = []
                while True:

                    frm = self.frame_queue.get()
                    if camera:
                        # Convert NV12 to BGR
                        yuv = frm.getData().reshape((frm.getHeight() * 3 // 2, frm.getWidth()))
                        cv_frame = cv2.cvtColor(yuv, cv2.COLOR_YUV2BGR_NV12)                      
                    else:
                        # get the frames corresponding to inference
                        cv_frame = np.ascontiguousarray(frm.getData().reshape(3, frm.getHeight(), frm.getWidth()).transpose(1,2,0))

                    image_width         = frm.getWidth ()
                    half_image_width    = int (frm.getWidth ()/2)
                    image_height        = frm.getHeight ()
                    half_image_height   = int(frm.getHeight ()/2)
                    
                    # Dividing the frame into 4 parts
                    cv2.line (cv_frame, (half_image_width, 0), (half_image_width, image_height), (255,0,0), 5)
                    cv2.line (cv_frame, (0, half_image_height), (image_width, half_image_height), (255,0,0), 5)
                    
                    raw_frames.append (frm)
                    frames.append(cv_frame)

                    # Break out once all frames received for the current inference
                    if frm.getSequenceNum() >= prev_passthrough.getSequenceNum() - 1:
                        break

                infered_frame       = frames[0]
                infered_raw_frame   = raw_frames[0]

                # Send bboxes to be infered upon
                for det in inference.detections:
                    raw_bbox = [det.xmin, det.ymin, det.xmax, det.ymax]
                    bbox = frame_norm(infered_frame, raw_bbox)
                    det_frame = infered_frame[bbox[1]:bbox[3], bbox[0]:bbox[2]]
                    nn_data = dai.NNData()
                    nn_data.setLayer("data", to_planar(det_frame, (48, 96)))
                    self.device.getInputQueue("reid_in").send(nn_data)

                 
                # Retrieve infered bboxes
                for det in inference.detections:
                    result_id   = -1
                    zone_id     = -1
                    conf        = 0
                    
                    raw_bbox    = [det.xmin, det.ymin, det.xmax, det.ymax]
                    bbox        = frame_norm(infered_frame, raw_bbox)
                    # Center of detected person
                    x_person    = (bbox[0] + bbox[2]) // 2
                    y_person    = (bbox[1] + bbox[3]) // 2

                    reid_result = self.device.getOutputQueue("reid_nn").get().getFirstLayerFp16()

                    for person_id in results:
                        conf = cos_dist(reid_result, results[person_id])
                        #print ("person_id: {}\t\tconf: {}".format (person_id, conf))
                        if conf > 0.7:
                            result_id = person_id
                            results[person_id] = reid_result
                            break
                    else:
                        result_id = next_id
                        results[result_id] = reid_result
                        results_path[result_id] = []
                        next_id += 1

                    # Building the payload which will be sent to clea and computing
                    #   at which part the person belong looking in 'inferred_frame'
                    
                    zone_id = self.get_zone_id (infered_raw_frame, (x_person, y_person))
                    payload = {
                        'id'            : result_id,
                        'confidence'    : round(conf, 5),
                        'pos_zone'      : {
                                        'id'    : zone_id,
                                        'name'  : self.screen_zone_map[zone_id]
                        }
                    }
                    detected_people.append (payload)
                    
                    # Displaying frame and inferred data
                    for frame in frames:
                        cv2.rectangle(frame, (bbox[0], bbox[1]), (bbox[2], bbox[3]), (10, 245, 10), 2)
                        
                        results_path[result_id].append([x_person, y_person])
                        cv2.putText(frame, str(zone_id), (x_person, y_person),
                                        cv2.FONT_HERSHEY_TRIPLEX, 1.0, color_picker[zone_id])
                        if len(results_path[result_id]) > 1:
                            cv2.polylines(frame, [np.array(results_path[result_id], dtype=np.int32)], False, (255, 0, 0), 2)

                
                # Sending inferred results to clea_if thread
                self.clea_if_queue.put (detected_people)
                
                # Send frame to visualization thread
                for frame in frames:
                    cv2.putText(frame, 'People count: '+str(len(detected_people)), (5,40), cv2.FONT_HERSHEY_DUPLEX, 1.0, (255,0,0), 2)

                    #if self.visualization_queue.full():
                    #    self.visualization_queue.get_nowait()
                    self.visualization_queue.put(frame)
            

                # Move current to prev
                prev_passthrough = passthrough
                prev_inference = inference

                if time.time() - t_fps >= 1.0:
                    self.nn_fps = round(fps / (time.time() - t_fps), 2)
                    fps = 0
                    t_fps = time.time()

            except RuntimeError:
                continue




    def input_task(self):
        seq_num = 0
        while self.is_running():

            # Send images to next stage

            # if camera - receive frames from camera
            if camera:
                try:
                    # TODO Handle disconnection exception
                    frame = self.device.getOutputQueue('cam_out').get()
                    self.frame_queue.put(frame)
                except RuntimeError:
                    #pipeline        = create_pipeline()
                    #self.device = dai.Device(pipeline)
                    continue
            
            # else if video - send frames down to NN
            else:

                # Get frame from video capture
                read_correctly, vid_frame = self.cap.read()
                if not read_correctly:
                    break

                # Send to NN and to inference thread
                frame_nn = dai.ImgFrame()
                frame_nn.setSequenceNum(seq_num)
                frame_nn.setWidth(544)
                frame_nn.setHeight(320)
                frame_nn.setData(to_planar(vid_frame, (544, 320)))
                self.device.getInputQueue("detection_in").send(frame_nn)

                # if high quality, send original frames
                frame_orig = dai.ImgFrame()
                frame_orig.setSequenceNum(seq_num)
                frame_orig.setWidth(vid_frame.shape[1])
                frame_orig.setHeight(vid_frame.shape[0])
                frame_orig.setData(to_planar(vid_frame, (vid_frame.shape[1], vid_frame.shape[0])))
                self.frame_queue.put(frame_orig)

                seq_num = seq_num + 1

                # Sleep at video framerate
                time.sleep(1.0 / self.FRAMERATE)
        # Stop execution after input task doesn't have
        # any extra data anymore




    def clea_if_task (self):
        while self.is_running():
            detected_people = None
            timestamp       = int(datetime.timestamp(datetime.now()))
            mappings_prefix = '/'+self.device_id+'/'
            payload         = {
                'reading_timestamp' : timestamp,
                'people_count'      : 0,
                'people'            : []
            }
            
            while (not self.clea_if_queue.empty()) :
                detected_people    = self.clea_if_queue.get (block=False)

            if detected_people != None :
                for p in detected_people :
                    payload['people'].append (json.dumps(p))
                payload['people_count'] = len (detected_people)

            #print(payload)

            if not self.astarte_device.is_connected() :
                print ("Clea connection absent! Cannot publish data :(")
            else :
                self.astarte_device.send_aggregate (self.interface_name, '/'+self.device_id,
                                                    payload, timestamp)

            time.sleep(int(config[settings_section]["PublishInterval"]))




    def image_emitter_task (self) :
        delay_ms            = 64
        send_timeout_ms     = 128
        last_sent_timestamp = int(time.time_ns() / 1000000)

        while (self.is_running()) :
            curr_timestamp  = int(time.time_ns() / 1000000)
            got_frame   = False
            last_frame  = None
            while not self.visualization_queue.empty () :
                last_frame  = self.visualization_queue.get()
                got_frame   = True
    
            if got_frame and curr_timestamp - last_sent_timestamp > send_timeout_ms :
                aspect_ratio    = last_frame.shape[1] / last_frame.shape[0]
                new_size        = (int(args.width),  int(args.width / aspect_ratio))
                resized_frame   = cv2.resize(last_frame, new_size)

                retval, buffer  = cv2.imencode('.jpg', last_frame)
                b64_frame       = base64.b64encode (buffer).decode ('utf-8')
                self.socket_io.emit ("new_data", b64_frame, broadcast=True)
                last_sent_timestamp = int(time.time_ns() / 1000000)
    
    
    
    
    def astarte_conection_cb (self, d) :
        print ('\n================\nDevice connected\n================\n\n')
        self.run()


    def astarte_disconnection_cb (self, d) :
        print ('\n===================\nDevice disconnected\n\n')
        # TODO


    def astarte_data_cb (self, data) :
        print ('\n=====================\nReceived new message!\n\n')
        # TODO


    def astarte_aggr_data_cb (self, data) :
        print ('\n=====================\nReceived new message!\n\n')
        # TODO

            


    def run(self):

        self.running    = True
        try :
            pipeline        = create_pipeline()
        except BaseException as e :
            print ("\n\nCatched this exception during pipeline creation: {}".format (e))
            exit ()

        # Connect to the device
        print("Starting pipeline...")
        try :
            self.device = dai.Device(pipeline)
            threads = [
                threading.Thread (target=self.input_task),
                threading.Thread (target=self.inference_task),
                threading.Thread (target=self.clea_if_task),
                threading.Thread (target=self.image_emitter_task)
            ]
            for t in threads:
                t.start()
                time.sleep (0.5)

            # Running flask and socketIO
            self.socket_io.run (self.flask, host= '0.0.0.0')

            self.running = False
            self.astarte_loop.stop()
        except BaseException as e:
            print ("\n\nCatched this exception during setup: {}".format (e))
            exit ()

        for thread in threads:
            thread.join()
            
        # cleanup
        if not camera:
            self.cap.release()
        
        print ("Bye bye!\n\n")


# ==================================================
# ==================================================



# Create the application
app = Main()


try :
    app.astarte_device.connect ()
    app.astarte_loop.run_forever ()
except BaseException as e:
    print ('====================================\n{}\n'.format(e))