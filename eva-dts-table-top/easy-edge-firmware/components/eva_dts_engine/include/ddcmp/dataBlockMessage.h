//
// Created by harlem88 on 07/10/20.
//

#ifndef DDCMP_DATABLOCKMESSAGE_H
#define DDCMP_DATABLOCKMESSAGE_H

#include "ddcmp/ddcmpMessage.h"

typedef struct dataBlockMessage {
    DdcmpMessage *data;

    int (*getBlockNumber)(struct dataBlockMessage *this);
    uint8_t* (*getPayload)(struct dataBlockMessage *this);
    int (*getPayloadLength)(struct dataBlockMessage * this);

} DataBlockMessage;

DataBlockMessage *dataBlockMessage_build(int, int, uint8_t*);
void dataBlockMessage_destroy(DataBlockMessage *this);
#endif //DDCMP_DATABLOCKMESSAGE_H
