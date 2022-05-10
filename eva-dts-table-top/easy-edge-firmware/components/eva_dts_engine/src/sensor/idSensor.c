//
// Created by harlem88 on 09/10/20.
//
#include <malloc.h>
#include "sensor/idSensor.h"
#include <string.h>
#include <stdlib.h>

/**
 * ID5*200923*134708**OFF
 * @param idSensor
 * @param evadtsDataBlock
 */

void parseId5(IDSensor *idSensor, EvadtsDataBlock *evadtsDataBlock);

IDSensor *idSensor_build(EvadtsDataBlock* evadtsDataBlock) {
    IDSensor *idSensor = malloc(sizeof(IDSensor));
    if (idSensor == NULL || evadtsDataBlock == NULL || evadtsDataBlock->elements == NULL) return NULL;

    if (evadtsDataBlock->size > 1) {
        char *machine_sn = evadtsDataBlock->elements[1];
        idSensor->machine_sn = malloc(strlen(machine_sn) + 1);
        strcpy(idSensor->machine_sn, machine_sn);
    } else {
        idSensor->machine_sn = NULL;
    }

    idSensor->machine_time_sec = 0;
    return idSensor;
}

void idSensor_addId(IDSensor* idSensor, EvadtsDataBlock* evadtsDataBlock) {
    if(idSensor == NULL || evadtsDataBlock->size < 1) return;
    char* idIdentifier = evadtsDataBlock->elements[0];
    switch (idIdentifier[2]) {
        case '5':
            parseId5(idSensor, evadtsDataBlock);
            break;
    }
}

void parseId5(IDSensor *idSensor, EvadtsDataBlock *evadtsDataBlock) {
    if (evadtsDataBlock->size > 2) {
        char *time = evadtsDataBlock->elements[2];

        int machine_times = (int) strtol((char *) time, NULL, 10);
        int seconds = machine_times % 100;
        machine_times = machine_times / 100;
        int minutes = machine_times % 100;
        int hours = machine_times / 100;

        idSensor->machine_time_sec = hours * 3600L + minutes * 60L + seconds;
    } else {
        idSensor->machine_time_sec = 0;
    }
}

void idSensor_destroy(IDSensor* idSensor){
    if(idSensor != NULL){

        if(idSensor->machine_sn != NULL)
            free(idSensor->machine_sn);

        free(idSensor);
    }
}
