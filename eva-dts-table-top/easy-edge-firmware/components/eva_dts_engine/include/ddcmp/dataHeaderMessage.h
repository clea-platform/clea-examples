//
// Created by harlem88 on 07/10/20.
//

#ifndef DDCMP_DATAHEADERMESSAGE_H
#define DDCMP_DATAHEADERMESSAGE_H

#include "ddcmp/ddcmpMessage.h"

typedef struct dataHeaderMessage {
    DdcmpMessage *data;

    bool (*isLastBlock)(struct dataHeaderMessage *this);
    int (*getBlockLength)(struct dataHeaderMessage *this);
    int (*getBlockLengthWithoutCrc)(struct dataHeaderMessage *this);

} DataHeaderMessage;

DataHeaderMessage *dataHeaderMessage_build(uint8_t *bytes, int len);
void dataHeaderMessage_destroy(DataHeaderMessage *this);

#endif //DDCMP_DATAHEADERMESSAGE_H
