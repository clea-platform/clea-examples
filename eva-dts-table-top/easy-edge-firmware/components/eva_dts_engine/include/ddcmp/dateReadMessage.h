//
// Created by harlem88 on 07/10/20.
//

#ifndef DDCMP_DATEREADMESSAGE_H
#define DDCMP_DATEREADMESSAGE_H

#include "ddcmp/ddcmpMessage.h"

typedef struct dataReadMessage {
    DdcmpMessage *data;

    uint8_t (*getListType)(struct dataReadMessage *this);
    bool (*isDataFileLengthUnknown)(struct dataReadMessage * this);
    uint16_t (*getDataFileLength)(struct dataReadMessage * this);
    bool (*isAccepted)(struct dataReadMessage *this, uint8_t);

} DataReadMessage;

DataReadMessage *dataReadMessage_build(int, uint8_t*, int);
void dataReadMessage_destroy(DataReadMessage *this);

#endif //DDCMP_DATEREADMESSAGE_H
