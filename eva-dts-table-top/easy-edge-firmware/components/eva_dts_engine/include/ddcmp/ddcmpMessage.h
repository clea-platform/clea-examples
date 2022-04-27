//
// Created by harlem88 on 07/10/20.
//

#ifndef DDCMPMESSAGE_H
#define DDCMPMESSAGE_H

#include <stdint.h>
#include <stdbool.h>

#define CRC_LEN 2
#define DDCMP_DEFAULT_MSG_SIZE 8
#define HEADER_LEN 2

static const uint8_t DDCMP_CM = 0x05;

static const uint8_t DDCMP_ACK = 0x01;
//static const uint8_t DDCMP_NACK = 0x02;
static const uint8_t DDCMP_START = 0x06;
static const uint8_t DDCMP_STACK = 0x07;
static const uint8_t DDCMP_DATA_MSG = 0x81;

/* MESSAGE HEADER */
static const uint8_t DDCMP_CMD = 0x77;
static const uint8_t DDCMP_CMD_RSP = 0x88;
static const uint8_t DDCMP_DATA_BLK = 0x99;

/* MESSAGE TYPE */
static const uint8_t DDCMP_WHO_ARE_YOU = 0xE0;
static const uint8_t DDCMP_READ_DATA = 0xE2;
static const uint8_t DDCMP_FINISH = 0xFF;

/* DDCMP List numbers */
static const uint8_t AUDIT_COLLECTION_LIST = 0x01;
static const uint8_t SECURITY_READ_LIST = 0x02;


static const uint8_t DDCMP_CM_FLAG_DEFAULT = 0x40;

static const uint8_t DDCMP_START_FRAME_MSG[8] = { DDCMP_CM, DDCMP_START, DDCMP_CM_FLAG_DEFAULT, 0x00, 0x00, 0x01, 0x5C,0x55 };

static const uint8_t DDCMP_WH0_ARE_YOU_FRAME_MSG[26] = { DDCMP_DATA_MSG, 0x10, DDCMP_CM_FLAG_DEFAULT, 0x00, 0x01, 0x01,0x0A,0x42,
                                                         DDCMP_CMD, DDCMP_WHO_ARE_YOU, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                         0x13,0x02, 0x17, 0x10, 0x32, 0x02, 0x00, 0x00, 0x0C, 0x82, 0x9F };

static const uint8_t DDCMP_READ_DATA_FRAME_MSG[27] = { DDCMP_CM, DDCMP_ACK, DDCMP_CM_FLAG_DEFAULT, 0x01, 0x00, 0x01, 0xB8,
                                                       0x55,DDCMP_DATA_MSG, 0x09, 0x40, 0x01, 0x02, 0x01, 0x46, 0xB0,
                                                       DDCMP_CMD, DDCMP_READ_DATA, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00,
                                                       0x00, 0xF0, 0x72 };

static const uint8_t DDCMP_FINISH_FRAME_MSG[4] = { DDCMP_CMD, DDCMP_FINISH, 0x67, 0xB0 };

typedef struct ddcmpMessage {
    uint8_t *bytes;
    int len;
} DdcmpMessage;

DdcmpMessage* ddcmpMessage_build(uint8_t*, int);
bool ddcmpMessage_checksum(const uint8_t*, int);
uint16_t ddcmpMessage_calcCRC(const uint8_t*, int);
bool ddcmpMessage_compareMsg(const uint8_t *left, int leftSize,  const uint8_t *right, int rightSize, int size);
void ddcmpMessage_destroy(DdcmpMessage* message);
DdcmpMessage* ddcmpMessage_buildReadDataMessage(uint8_t ddcmpListNumber);
DdcmpMessage* ddcmpMessage_buildFinishMessage(const uint8_t* ack_message, int rxCount, int txCount);

#endif //DDCMPMESSAGE_H