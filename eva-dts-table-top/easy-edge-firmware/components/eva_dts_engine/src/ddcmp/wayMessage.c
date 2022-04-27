//
// Created by harlem88 on 07/10/20.
//
#include "ddcmp/wayMessage.h"
#include <stdlib.h>

WayMessage *wayMessage_build(int msgBlockLength, uint8_t *bytes, int len) {

    if (len >= (msgBlockLength + HEADER_LEN) &&
        bytes[0] == DDCMP_CMD_RSP &&
        bytes[1] == DDCMP_WHO_ARE_YOU &&
        ddcmpMessage_checksum(bytes, (msgBlockLength + HEADER_LEN))) {

        return ddcmpMessage_build(bytes, (msgBlockLength + HEADER_LEN));
    }
    return NULL;
}

void wayMessage_destroy(WayMessage *this){
    ddcmpMessage_destroy(this);
}