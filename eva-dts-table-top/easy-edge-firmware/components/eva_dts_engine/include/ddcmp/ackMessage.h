//
// Created by harlem88 on 07/10/20.
//

#ifndef DDCMP_ACKMESSAGE_H
#define DDCMP_ACKMESSAGE_H

#include "ddcmpMessage.h"
#define ACK_MESSAGE_SIZE 8
typedef DdcmpMessage AckMessage;

AckMessage *ackMessage_build(uint8_t *bytes, int len);
AckMessage *ackMessage_buildFromRxTx(int rxCount, int txCount);

void ackMessage_destroy(AckMessage *this);

#endif //DDCMP_ACKMESSAGE_H
