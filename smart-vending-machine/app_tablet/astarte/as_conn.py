from astarte.device import Device
import json
import os
import glob
import time


class Singleton(type):
    _instances = {}
    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            cls._instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
        return cls._instances[cls]


class Astarte(metaclass=Singleton):
    pass

###########################
# Configuration Variables #
###########################
persistency_dir = 'persistency'
device_id = '2-ASUSOCIwSERS-GWLA-Xg'
realm = 'devenv0'
credentials_secret = ''
pairing_base_url = "https://api.demo.clea.cloud/pairing"
interfaces_dir_path = "astarte/interfaces/face_age_emotions/*.json"


def load_interfaces(interfaces_dir):
    interfaces = []
    for interface_file in glob.iglob(interfaces_dir, recursive=True):
        with open(interface_file) as json_file:
            interfaces.append(json.load(json_file))
    return interfaces


def callback(device, iname, ipath, payload):
    """ Method called when Astarte sends data to the device """
    print(device, iname, ipath, payload)
    return True


def connect_callback(sel):
    """ Method called when Astarte connects successfully to the device. """
    print("Device has been connected!")

###########################
# Setup Device connection #
###########################


def set_device():
    os.makedirs(persistency_dir, exist_ok=True)
    device = Device(device_id=device_id, realm=realm, credentials_secret=credentials_secret,
                    pairing_base_url=pairing_base_url, persistency_dir=persistency_dir)

    interfaces = load_interfaces(interfaces_dir_path)
    for interface in interfaces:
        device.add_interface(interface)

    # device.on_connected = connect_callback
    device.on_data_received = callback

    device.connect()
    print("Wating connection with astarte: ")
    retry = 0
    while not device.is_connected() and retry<4:
        time.sleep(1)
        print(f"Device connection: {device.is_connected()}")
        retry += 1
    if not device.is_connected() and retry==4:
        device = False
    return device


def send_data(device, data):
    device.send_aggregate("devenv0.face.emotion.detection.Transaction", "/transaction", payload=data, timestamp=time.time())