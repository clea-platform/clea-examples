//
// Created by harlem88 on 07/10/20.
//
#include "ddcmp/stackMessage.h"
#include <stdlib.h>

StackMessage *stackMessage_build(uint8_t *bytes, int len) {

    if (len >= DDCMP_DEFAULT_MSG_SIZE &&
        bytes[0] == DDCMP_CM &&
        bytes[1] == DDCMP_STACK &&
        bytes[2] == DDCMP_CM_FLAG_DEFAULT &&
        ddcmpMessage_checksum(bytes, DDCMP_DEFAULT_MSG_SIZE)){

        return ddcmpMessage_build(bytes, len);
    }
    return NULL;
}

void stackMessage_destroy(StackMessage *this){
    ddcmpMessage_destroy(this);
}