//
// Created by harlem88 on 07/10/20.
//

#include "ddcmp/dataHeaderMessage.h"
#include <stdlib.h>

static bool isLastBlock(DataHeaderMessage* this);
static int getBlockLength(DataHeaderMessage* this);
static int getBlockLengthWithoutCrc(DataHeaderMessage* this);

DataHeaderMessage *dataHeaderMessage_build(uint8_t *bytes, int len) {

    if (len >= DDCMP_DEFAULT_MSG_SIZE &&
        bytes[0] == DDCMP_DATA_MSG &&
        ddcmpMessage_checksum(bytes, DDCMP_DEFAULT_MSG_SIZE)) {

        DataHeaderMessage *dataHeaderMessage = malloc(sizeof(DataHeaderMessage));
        if (dataHeaderMessage != NULL) {
            dataHeaderMessage->data = ddcmpMessage_build(bytes, len);

            dataHeaderMessage->isLastBlock= &isLastBlock;
            dataHeaderMessage->getBlockLength= &getBlockLength;
            dataHeaderMessage->getBlockLengthWithoutCrc= &getBlockLengthWithoutCrc;

            return dataHeaderMessage;
        }
    }
    return NULL;
}

static bool isLastBlock(DataHeaderMessage* this) {
    return this->data->bytes[2] == 0xC0;
}

static int getBlockLength(DataHeaderMessage* this) {
    return this->data->bytes[1];
}

static int getBlockLengthWithoutCrc(DataHeaderMessage* this) {
    return this->data->bytes[1] - CRC_LEN;
}

void dataHeaderMessage_destroy(DataHeaderMessage *this){
    if(this != NULL){
        ddcmpMessage_destroy(this->data);
        free(this);
    }
}