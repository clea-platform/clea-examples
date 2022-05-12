import os

messages_dicts = {0X01: "PROVISIONED",
                  0X02: "NOT_PROVISIONED",
                  0X03: "USER_APP_LAUNCHED",
                  0X04: "ERROR_LAUNCHING_USER_APP",
                  0X05: "CONNECTED",
                  0X06: "CONNECTION_LOST",
                  0x07: "CONNECTED",
                  0x08: "DISCONNECTED",
                  0X09: "ACTIVE",
                  0X0A: "NOT_ACTIVE",
                  0X0B: "API_RECEIVED",
                  0X0C: "WIFI_RECEIVED",
                  0X0D: "FOUND_DATA_NVS",
                  0X0E: "NO_DATA_NVS",
                  0X0F: "NEW_API_RECEIVED",
                  0X10: "API_VERIFICATION_FAILED",
                  0X11: "DEINIT",
                  0X12: "ON",
                  0X13: "START_PPP",
                  0X14: "STOP_PPP",
                  0x15: "REQUESTED",
                  0X16: "STARTED",
                  0X17: "SUCCESS",
                  0X18: "ATTEMPT_FAILED",
                  0X19: "FAILED",
                  0x1A: "CONFIGURED",
                  0X1B: "NOT_CONFIGURED",
                  0X1C: "STARTED",
                  0X1D: "CONNECTED",
                  0X1E: "DISCONNECTED",
                  0X1F: "NEW_CONFIGURATION",
                  0x20: "PPP_CONNECTED",
                  0x21: "PPP_DISCONNECTED",
                  0x22: "LOCATION_REQUEST",
                  0x23: "BATTERY_POWERED",
                  0x24: "AC_POWERED",
                  0x25: "OTA_COMPLETED",
                  0x26: "ERROR_LAUNCHING_OTA"
                  0XFF: "INIT_COMPLETED"}

modules_dict = {0x10: "CORE:__ ",
                0x20: "BLE:___ ",
                0x30: "WIFI:__ ",
                0x40: "OTA:___ ",
                0x50: "MODEM:_ ",
                0x60: "BATT:__ ",
                0x70: "MQTT:__ ",
                0x80: "API:___ "}

severyty_dict = {0x01: "(EE)",
                 0x02: "(WW)",
                 0x03: "(II)",
                 0x04: "(DD)",
                 0x05: "(VV)"}


def parse_message_code(message_byte):
    return messages_dicts.get(message_byte, hex(message_byte))


def parse_severyty_and_module(byte):
    if byte & 0x0F in severyty_dict.keys() and byte & 0xF0 in modules_dict.keys():
        return severyty_dict[byte & 0x0F] + "-" + modules_dict[byte & 0xF0]

    return hex(byte)


def read_binary(path):
    file = open(path, "rb")
    try:
        entry = file.read(2)
        while entry != b'':
            if entry[0] == 0xFF and entry[1] == 0xFF:
                print("========= BOOT =========")
            else:
                print(parse_severyty_and_module(
                    entry[1]) + parse_message_code(entry[0]))
            entry = file.read(2)

    finally:
        file.close()
    return


def load_log_files(path):
    abs_path = os.curdir+"/"+path
    log_file_list = os.listdir(path)

    for element in log_file_list:
        if "bin" in element:
            print("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$")
            print("$  reading file: "+element)
            print("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$")
            read_binary(abs_path+"/"+element)
    return


def main():
    folder_content_list = os.listdir()

    # TODO ADD LOADING DICTIONARIES

    for elem in folder_content_list:
        if "log" in elem.lower():
            if os.path.isdir(elem):
                load_log_files(elem)
    return


if __name__ == "__main__":
    main()
