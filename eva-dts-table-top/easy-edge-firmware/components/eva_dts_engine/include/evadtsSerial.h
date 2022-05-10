//
// Created by harlem88 on 06/10/20.
//

#ifndef EVADTSSERIAL_H
#define EVADTSSERIAL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct serialDataMsg{
    const uint8_t *bytes;
    int length;
}SerialDataMsg;

bool evadtsSerial_setup();
SerialDataMsg* evadtsSerial_read();
int evadtsSerial_write(SerialDataMsg);
bool evadtsSerial_close();
void evadtsSerial_freeSerialData(SerialDataMsg*);

#endif //EVADTSSERIAL_H
