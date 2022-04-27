//
// Created by harlem88 on 07/10/20.
//

#include <stdlib.h>
#include "ddcmp/dataBlockMessage.h"

static int getBlockNumber(DataBlockMessage *this);
static uint8_t* getPayload(DataBlockMessage *this);
static int getPayloadLength(DataBlockMessage * this);

DataBlockMessage *dataBlockMessage_build(int msgBlockLength, int totalMsgLen, uint8_t *bytes) {
    if (totalMsgLen >= (msgBlockLength + CRC_LEN) &&
        bytes[0] == DDCMP_DATA_BLK &&
        ddcmpMessage_checksum(bytes, (msgBlockLength + CRC_LEN))) {

        DataBlockMessage *dataBlockMessage = malloc(sizeof(DataBlockMessage));
        if (dataBlockMessage != NULL) {
            dataBlockMessage->data = ddcmpMessage_build(bytes, msgBlockLength);

            dataBlockMessage->getBlockNumber = &getBlockNumber;
            dataBlockMessage->getPayload = &getPayload;
            dataBlockMessage->getPayloadLength = &getPayloadLength;

            return dataBlockMessage;
        }
    }
    return NULL;
}

static int getBlockNumber(DataBlockMessage *this){
    return this->data->bytes[1];
}

static uint8_t* getPayload(DataBlockMessage *this){
    return  &this->data->bytes[HEADER_LEN];
}

static int getPayloadLength(DataBlockMessage * this){
    return this->data->len - HEADER_LEN;
}

void dataBlockMessage_destroy(DataBlockMessage *this){
    if(this != NULL){
        ddcmpMessage_destroy(this->data);
        free(this);
    }
}