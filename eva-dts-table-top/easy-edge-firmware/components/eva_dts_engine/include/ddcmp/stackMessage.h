//
// Created by harlem88 on 07/10/20.
//

#ifndef DDCMP_STACKMESSAGE_H
#define DDCMP_STACKMESSAGE_H

#include "ddcmp/ddcmpMessage.h"

typedef DdcmpMessage StackMessage;

StackMessage *stackMessage_build(uint8_t *bytes, int len);
void stackMessage_destroy(StackMessage *this);

#endif //DDCMP_STACKMESSAGE_H
