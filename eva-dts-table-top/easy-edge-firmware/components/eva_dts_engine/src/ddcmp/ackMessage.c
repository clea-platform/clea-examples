//
// Created by harlem88 on 07/10/20.
//
#include "ddcmp/ackMessage.h"
#include <stdlib.h>

AckMessage *ackMessage_build(uint8_t *bytes, int len) {

    if (len >= DDCMP_DEFAULT_MSG_SIZE &&
        bytes[0] == DDCMP_CM &&
        bytes[1] == DDCMP_ACK &&
        ddcmpMessage_checksum(bytes, DDCMP_DEFAULT_MSG_SIZE)){

        return ddcmpMessage_build(bytes, len);
    }
    return NULL;
}

AckMessage* ackMessage_buildFromRxTx(int rxCount, int txCount) {
    uint8_t ackBuffer[8];
    ackBuffer[0] = DDCMP_CM;
    ackBuffer[1] = DDCMP_ACK;
    ackBuffer[2] = DDCMP_CM_FLAG_DEFAULT;
    ackBuffer[3] = rxCount;
    ackBuffer[4] = txCount;
    ackBuffer[5] = 0x01;
    uint16_t crc = ddcmpMessage_calcCRC(ackBuffer, 6);
    ackBuffer[6] = crc & 0xff;
    ackBuffer[7] = crc >> 8;

    return ddcmpMessage_build(ackBuffer, 8);
}

void ackMessage_destroy(AckMessage *this){
    ddcmpMessage_destroy(this);
}
