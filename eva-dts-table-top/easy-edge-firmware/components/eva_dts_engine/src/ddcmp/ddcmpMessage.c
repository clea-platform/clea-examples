//
// Created by harlem88 on 07/10/20.
//

#include "ddcmp/ddcmpMessage.h"
#include <stdlib.h>
#include <string.h>

DdcmpMessage *ddcmpMessage_build(uint8_t *bytes, int len) {
    DdcmpMessage *ddcmpMessage = malloc(sizeof(DdcmpMessage));

    if (ddcmpMessage != NULL) {
        ddcmpMessage->bytes = malloc(len * sizeof(uint8_t));
        if (ddcmpMessage->bytes != NULL) {
            memcpy(ddcmpMessage->bytes, bytes, len * sizeof(uint8_t));
            ddcmpMessage->len = len;
        } else {
            ddcmpMessage->bytes = NULL;
            ddcmpMessage->len = 0;
            free(ddcmpMessage);
            ddcmpMessage = NULL;
        }
        return ddcmpMessage;
    }
    return NULL;
}

bool ddcmpMessage_checksum(const uint8_t* bytes, int len) {
    uint16_t crc = ddcmpMessage_calcCRC(bytes, len - CRC_LEN);
    return ((crc & 0xFF) == bytes[len - 2]) && ((crc >> 8) == bytes[len - 1]);
}

uint16_t ddcmpMessage_calcCRC(const uint8_t* bytes, int len){
    uint16_t crc = 0x0000;
    for (int pos = 0; pos < len; pos++) {
        crc ^= bytes[pos];
        for (int i = 8; i > 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

bool ddcmpMessage_compareMsg(const uint8_t *left, int leftSize,  const uint8_t *right, int rightSize, int size) {
        bool result = ( leftSize >= size && rightSize >= size);

        for (int i = 0; (result && i < size); i++) {
            result = left[i] == right[i];
        }

        return result;
}

void ddcmpMessage_destroy(DdcmpMessage* message){
    if (message != NULL) {
        if (message->bytes != NULL)
            free(message->bytes);

        free(message);
    }
}

DdcmpMessage* ddcmpMessage_buildReadDataMessage(uint8_t ddcmpListNumber){
    int8_t size = 27;
    uint8_t readDataBuffer[size];
    memcpy(readDataBuffer , DDCMP_READ_DATA_FRAME_MSG, size * sizeof(uint8_t));
    readDataBuffer[19] = ddcmpListNumber;
    uint16_t crc = ddcmpMessage_calcCRC(&readDataBuffer[16], 9);
    readDataBuffer[25] = crc & 0xff;
    readDataBuffer[26] = crc >> 8;

    return ddcmpMessage_build(readDataBuffer, size);
}

DdcmpMessage* ddcmpMessage_buildFinishMessage(const uint8_t* ack_message, int rxCount, int txCount){
    uint8_t finishBuffer[20];
    memcpy(finishBuffer , ack_message, 8 * sizeof(uint8_t));
    finishBuffer[8] = DDCMP_DATA_MSG;
    finishBuffer[9] = 0x02;
    finishBuffer[10] = DDCMP_CM_FLAG_DEFAULT;
    finishBuffer[11] = rxCount;
    finishBuffer[12] = txCount;
    finishBuffer[13] = 0x01;
    uint16_t crc = ddcmpMessage_calcCRC(&finishBuffer[8], 6);
    finishBuffer[14] = crc & 0xff;
    finishBuffer[15] = crc >> 8;
    memcpy(finishBuffer + 16, DDCMP_FINISH_FRAME_MSG, 4 * sizeof(uint8_t));

    return ddcmpMessage_build(finishBuffer, 20);
}
