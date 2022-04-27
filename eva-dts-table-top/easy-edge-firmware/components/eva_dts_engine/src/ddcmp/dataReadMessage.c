//
// Created by harlem88 on 07/10/20.
//

#include "ddcmp/dateReadMessage.h"
#include <stdlib.h>

static uint8_t getListType(DataReadMessage *this);
static bool isDataFileLengthUnknown(DataReadMessage *this);
static uint16_t getDataFileLength(DataReadMessage *this);
static bool isAccepted(DataReadMessage *this, uint8_t ddcmpListNumber);

DataReadMessage *dataReadMessage_build(int msgBlockLength, uint8_t *bytes, int len) {
    if (len >= (msgBlockLength + HEADER_LEN) &&
        bytes[0] == DDCMP_CMD_RSP &&
        bytes[1] == DDCMP_READ_DATA &&
        ddcmpMessage_checksum(bytes, (msgBlockLength + HEADER_LEN))) {

        DataReadMessage *dataReadMessage = malloc(sizeof(*dataReadMessage));
        if (dataReadMessage != NULL) {
            dataReadMessage->data = ddcmpMessage_build(bytes, len);

            dataReadMessage->getListType = &getListType;
            dataReadMessage->isDataFileLengthUnknown = &isDataFileLengthUnknown;
            dataReadMessage->getDataFileLength = &getDataFileLength;
            dataReadMessage->isAccepted = &isAccepted;

            return dataReadMessage;
        }
    }
    return NULL;
}

static uint8_t getListType(DataReadMessage *this) {
    return this->data->bytes[3];
}

static bool isDataFileLengthUnknown(DataReadMessage *this) {
    return (this->data->bytes[7] == 0xFF) && (this->data->bytes[8] == 0xFF);
}

static uint16_t getDataFileLength(DataReadMessage *this) {
    return ((this->data->bytes[8] & 0xFFFF) << 8) | this->data->bytes[7];
}

static bool isAccepted(DataReadMessage *this, uint8_t ddcmpListNumber) {
    return this->data->bytes[3] == ddcmpListNumber;
}

void dataReadMessage_destroy(DataReadMessage *this){
    if(this != NULL){
        ddcmpMessage_destroy(this->data);
        free(this);
    }
}