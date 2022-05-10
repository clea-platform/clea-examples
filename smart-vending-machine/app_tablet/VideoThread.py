import cv2
import numpy as np
import pandas as pd
import time

from PyQt5.QtCore import pyqtSignal, QThread, QTimer

from deepface import DeepFace
from deepface.detectors import FaceDetector
from deepface.extendedmodels import Age
from deepface.commons import functions, realtime, distance as dst

from utils.definitions import emotions, genders
import os
os.environ['KMP_DUPLICATE_LIB_OK'] = 'True'
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'


DEBUG = False

pause_time = 5.0


def get_bigger_face(locs):
    if len(locs) == 1:
        return 0
    max_area = 0
    selected = 0
    for i, box in enumerate(locs):
        (startX, startY, endX, endY) = box
        area = abs(endX-startX) * abs(endY-startY)
        if area > max_area:
            max_area = area
            selected = i
    return selected


class VideoThread(QThread):

    updated = pyqtSignal()  # in order to work it has to be defined out of the contructor

    def __init__(self, frame_threshold=50, time_threshold=10, source=0):
        super().__init__()

        self.currentFrame = None

        self.active = False
        self.pause = False
        self.source = source
        self.current_user = {}
        self.detector_backend = 'opencv'
        self.enable_face_analysis = True
        self.frame_threshold = frame_threshold
        self.time_threshold = time_threshold
        self.init_models()

    def init_models(self):
        self.face_detector = FaceDetector.build_model(self.detector_backend)
        print("Detector backend is ", self.detector_backend)
        # --------------------------
        tic = time.time()
        self.emotion_model = DeepFace.build_model('Emotion')
        print("Emotion model loaded")
        self.age_model = DeepFace.build_model('Age')
        print("Age model loaded")
        self.gender_model = DeepFace.build_model('Gender')
        print("Gender model loaded")
        toc = time.time()
        print("Facial attibute analysis models loaded in ", toc - tic, " seconds")

    def get_current_frame(self):
        """Getter for the currentFrame attribute"""
        return self.currentFrame

    def get_info_user(self):
        return self.current_user

    def set_frame_threshold(self, thr):
        self.frame_threshold = thr

    def set_time_threshold(self, thr):
        self.time_threshold = thr

    def deactivate(self):
        """Method called to stop and deactivate the face recognition Thread"""
        self.active = False
        self.exit()

    def pause_loop(self):
        self.pause = True

    def unpause(self):
        self.pause = False

    def run(self):
        """Main loop of this Thread"""
        self.active = True
        camera = cv2.VideoCapture(self.source)

        freeze = False
        face_detected = False
        face_included_frames = 0  # freeze screen if face detected sequantially 5 frames
        freezed_frame = 0
        tic = time.time()

        while self.active:
            # Grab a single frame of video
            if not self.pause:

                ret, frame = camera.read()

                if frame is None:
                    break

                frame = cv2.flip(cv2.cvtColor(frame, cv2.COLOR_BGR2RGB), 1)

                raw_img = frame.copy()
                resolution = frame.shape
                resolution_x = frame.shape[1]
                resolution_y = frame.shape[0]

                if freeze == False:

                    try:
                        # faces store list of detected_face and region pair
                        faces = FaceDetector.detect_faces(self.face_detector, self.detector_backend, frame, align=False)
                    except:  # to avoid exception if no face detected
                        faces = []

                    if len(faces) == 0:
                        face_included_frames = 0
                else:
                    faces = []

                detected_faces = []
                face_index = 0
                for face, (x, y, w, h) in faces:
                    if w > 130:  # discard small detected faces

                        face_detected = True
                        if face_index == 0:
                            face_included_frames = face_included_frames + 1  # increase frame for a single face

                        cv2.rectangle(frame, (x, y), (x + w, y + h), (67, 67, 67), 1)  # draw rectangle to main image

                        cv2.putText(frame, str(self.frame_threshold - face_included_frames), (int(x + w / 4), int(y + h / 1.5)),
                                    cv2.FONT_HERSHEY_SIMPLEX, 4, (255, 255, 255), 2)

                        # -------------------------------------

                        detected_faces.append((x, y, w, h))
                        face_index = face_index + 1

                        # -------------------------------------

                if face_detected==True and face_included_frames==self.frame_threshold and freeze==False:
                    freeze = True
                    base_img = raw_img.copy()
                    detected_faces_final = detected_faces.copy()
                    tic = time.time()

                if freeze == True:

                    toc = time.time()
                    if (toc - tic) < self.time_threshold:

                        if freezed_frame == 0:
                            freeze_img = base_img.copy()
                            # freeze_img = np.zeros(resolution, np.uint8) #here, np.uint8 handles showing white area issue

                            for detected_face in detected_faces_final:

                                x = detected_face[0]
                                y = detected_face[1]
                                w = detected_face[2]
                                h = detected_face[3]

                                cv2.rectangle(freeze_img, (x, y), (x + w, y + h), (67, 67, 67),
                                              1)  # draw rectangle to main image

                                # apply deep learning for custom_face
                                custom_face = base_img[y:y + h, x:x + w]

                                # facial attribute analysis
                                if self.enable_face_analysis == True:

                                    gray_img = functions.preprocess_face(img=custom_face, target_size=(48, 48),
                                                                         grayscale=True,
                                                                         enforce_detection=False, detector_backend='opencv')

                                    emotion_labels = ['Anger', 'Disgust', 'Fear', 'Happy', 'Sad', 'Surprise', 'Neutral']
                                    emotion_predictions = self.emotion_model.predict(gray_img)[0, :]
                                    sum_of_predictions = emotion_predictions.sum()

                                    mood_items = []
                                    for i in range(0, len(emotion_labels)):
                                        mood_item = []
                                        emotion_label = emotion_labels[i]
                                        emotion_prediction = 100 * emotion_predictions[i] / sum_of_predictions
                                        mood_item.append(emotion_label)
                                        mood_item.append(emotion_prediction)
                                        mood_items.append(mood_item)

                                    if emotion_labels[np.argmax(emotion_predictions)] == "Fear" or \
                                            emotion_labels[np.argmax(emotion_predictions)] == "Disgust":
                                        self.current_user['emotion'] = emotion_labels[np.argsort(emotion_predictions)[-2]]
                                    else:
                                        self.current_user['emotion'] = emotion_labels[np.argmax(emotion_predictions)]

                                    emotion_df = pd.DataFrame(mood_items, columns=["emotion", "score"])
                                    emotion_df = emotion_df.sort_values(by=["score"], ascending=False).reset_index(
                                        drop=True)

                                    # background of mood box

                                    # transparency
                                    overlay = freeze_img.copy()
                                    opacity = 0.4

                                    # if x + w + pivot_img_size < resolution_x:
                                    #     # right
                                    #     cv2.rectangle(freeze_img
                                    #                   # , (x+w,y+20)
                                    #                   , (x + w, y)
                                    #                   , (x + w + pivot_img_size, y + h)
                                    #                   , (64, 64, 64), cv2.FILLED)
                                    #
                                    #     cv2.addWeighted(overlay, opacity, freeze_img, 1 - opacity, 0, freeze_img)
                                    #
                                    # elif x - pivot_img_size > 0:
                                    #     # left
                                    #     cv2.rectangle(freeze_img
                                    #                   # , (x-pivot_img_size,y+20)
                                    #                   , (x - pivot_img_size, y)
                                    #                   , (x, y + h)
                                    #                   , (64, 64, 64), cv2.FILLED)
                                    #
                                    #     cv2.addWeighted(overlay, opacity, freeze_img, 1 - opacity, 0, freeze_img)
                                    #
                                    # for index, instance in emotion_df.iterrows():
                                    #     emotion_label = "%s " % (instance['emotion'])
                                    #     emotion_score = instance['score'] / 100
                                    #
                                    #     bar_x = 35  # this is the size if an emotion is 100%
                                    #     bar_x = int(bar_x * emotion_score)
                                    #
                                    #     if x + w + pivot_img_size < resolution_x:
                                    #
                                    #         text_location_y = y + 20 + (index + 1) * 20
                                    #         text_location_x = x + w
                                    #
                                    #         if text_location_y < y + h:
                                    #             cv2.putText(freeze_img, emotion_label, (text_location_x, text_location_y),
                                    #                         cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
                                    #
                                    #             cv2.rectangle(freeze_img
                                    #                           , (x + w + 70, y + 13 + (index + 1) * 20)
                                    #                           , (x + w + 70 + bar_x, y + 13 + (index + 1) * 20 + 5)
                                    #                           , (255, 255, 255), cv2.FILLED)
                                    #
                                    #     elif x - pivot_img_size > 0:
                                    #
                                    #         text_location_y = y + 20 + (index + 1) * 20
                                    #         text_location_x = x - pivot_img_size
                                    #
                                    #         if text_location_y <= y + h:
                                    #             cv2.putText(freeze_img, emotion_label, (text_location_x, text_location_y),
                                    #                         cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
                                    #
                                    #             cv2.rectangle(freeze_img
                                    #                           , (x - pivot_img_size + 70, y + 13 + (index + 1) * 20)
                                    #                           , (x - pivot_img_size + 70 + bar_x,
                                    #                              y + 13 + (index + 1) * 20 + 5)
                                    #                           , (255, 255, 255), cv2.FILLED)

                                # -------------------------------

                                face_224 = functions.preprocess_face(img=custom_face, target_size=(224, 224),
                                                                     grayscale=False,
                                                                     enforce_detection=False, detector_backend='opencv')

                                age_predictions = self.age_model.predict(face_224)[0, :]
                                apparent_age = Age.findApparentAge(age_predictions)

                                # -------------------------------

                                gender_prediction = self.gender_model.predict(face_224)[0, :]

                                if np.argmax(gender_prediction) == 0:
                                    gender = genders[0]     # Female
                                elif np.argmax(gender_prediction) == 1:
                                    gender = genders[1]     # Male

                                analysis_report = str(int(apparent_age)) + " " + gender

                                self.current_user['age'] = apparent_age
                                self.current_user['gender'] = gender

                                # -------------------------------

                                info_box_color = (46, 200, 255)

                                # # top
                                # if y - pivot_img_size + int(pivot_img_size / 5) > 0:
                                #
                                #     triangle_coordinates = np.array([
                                #         (x + int(w / 2), y)
                                #         , (x + int(w / 2) - int(w / 10), y - int(pivot_img_size / 3))
                                #         , (x + int(w / 2) + int(w / 10), y - int(pivot_img_size / 3))
                                #     ])
                                #
                                #     cv2.drawContours(freeze_img, [triangle_coordinates], 0, info_box_color, -1)
                                #
                                #     cv2.rectangle(freeze_img,
                                #                   (x + int(w / 5), y - pivot_img_size + int(pivot_img_size / 5)),
                                #                   (x + w - int(w / 5), y - int(pivot_img_size / 3)), info_box_color,
                                #                   cv2.FILLED)
                                #
                                #     cv2.putText(freeze_img, analysis_report,
                                #                 (x + int(w / 3.5), y - int(pivot_img_size / 2.1)),
                                #                 cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 111, 255), 2)
                                #
                                # # bottom
                                # elif y + h + pivot_img_size - int(pivot_img_size / 5) < resolution_y:
                                #
                                #     triangle_coordinates = np.array([
                                #         (x + int(w / 2), y + h)
                                #         , (x + int(w / 2) - int(w / 10), y + h + int(pivot_img_size / 3))
                                #         , (x + int(w / 2) + int(w / 10), y + h + int(pivot_img_size / 3))
                                #     ])
                                #
                                #     cv2.drawContours(freeze_img, [triangle_coordinates], 0, info_box_color, -1)
                                #
                                #     cv2.rectangle(freeze_img, (x + int(w / 5), y + h + int(pivot_img_size / 3)),
                                #                   (x + w - int(w / 5), y + h + pivot_img_size - int(pivot_img_size / 5)),
                                #                   info_box_color, cv2.FILLED)
                                #
                                #     cv2.putText(freeze_img, analysis_report,
                                #                 (x + int(w / 3.5), y + h + int(pivot_img_size / 1.5)),
                                #                 cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 111, 255), 2)

                                # ---------------
                                tic = time.time()  # in this way, freezed image can show 5 seconds

                                # ----------------

                                # transparency
                                overlay = freeze_img.copy()
                                opacity = 0.4

                                x_info = 10
                                y_info = 10

                                cv2.rectangle(freeze_img, (x_info, y_info), (x_info + 120, y_info + 80), (64, 64, 64), cv2.FILLED)
                                cv2.addWeighted(overlay, opacity, freeze_img, 1 - opacity, 0, freeze_img)

                                x_info += 2
                                y_info += 20
                                cv2.putText(freeze_img, f"Emotion: {self.current_user['emotion']}", (x_info, y_info),
                                            cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 255, 255), 1)
                                # x_info += 20
                                y_info += 20
                                cv2.putText(freeze_img, f"Gender: {gender}", (x_info, y_info),
                                            cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 255, 255), 1)
                                # x_info += 20
                                y_info += 20
                                cv2.putText(freeze_img, f"Age: {str(int(apparent_age))}", (x_info, y_info),
                                            cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 255, 255), 1)

                        time_left = int(self.time_threshold - (toc - tic) + 1)

                        cv2.rectangle(freeze_img, (10, resolution_y-50), (90, resolution_y-10), (67, 67, 67), -10)
                        cv2.putText(freeze_img, str(time_left), (40, resolution_y-20), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 1)


                        # Store the current image
                        self.currentFrame = freeze_img

                        freezed_frame = freezed_frame + 1

                    else:
                        face_detected = False
                        face_included_frames = 0
                        freeze = False
                        freezed_frame = 0

                else:
                    # Store the current image
                    self.currentFrame = frame
                    # toc = time.time()
                    # if (toc - tic) < self.time_threshold:
                    self.current_user = {}
                    pass

                if not self.pause:
                    self.updated.emit()

            else:
                # self.current_user = {}
                QTimer.singleShot(int(1000), self.unpause)

    def stop(self):
        """Sets run flag to False and waits for thread to finish"""
        print("STOP")
        self.pause = False
        self.wait()
