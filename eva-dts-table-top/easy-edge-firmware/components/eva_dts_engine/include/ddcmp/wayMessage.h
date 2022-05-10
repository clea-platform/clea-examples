//
// Created by harlem88 on 07/10/20.
//

#ifndef DDCMP_WAYMESSAGE_H
#define DDCMP_WAYMESSAGE_H

#include "ddcmp/ddcmpMessage.h"

typedef DdcmpMessage WayMessage;

WayMessage *wayMessage_build(int, uint8_t *, int);
void wayMessage_destroy(WayMessage *this);

#endif //DDCMP_WAYMESSAGE_H
