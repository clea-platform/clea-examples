//
// Created by harlem88 on 09/10/20.
//
#include "evadtsHandler.h"
#include <string.h>
#include <sys/stat.h>
#include <regex.h>


// FIXME Check me!!!!
EvaDtsAudit* evadtsHandler_handleSensors(EvadtsDataBlockList* itemList){
    regex_t paRegex;
    regcomp(&paRegex, "(PA[1-6])", REG_EXTENDED);
    EvaDtsAudit *sensorEvaDts = evadtsAudit_build();

    sensorEvaDts->paSensorList = paSensorList_getInstance();
    sensorEvaDts->saSensorList = saSensorList_getInstance();

    while (evadtsDataBlockList_hasNext(itemList)) {
        EvadtsDataBlock *evadtsDataBlock = evadtsDataBlockList_next(itemList);

        if (evadtsDataBlock != NULL && evadtsDataBlock->size > 0) {
            char *first = evadtsDataBlock->elements[0];
            if (strcmp((char *) first, "ID1") == 0) {
                sensorEvaDts->idSensor = idSensor_build(evadtsDataBlock);
            } else if (strcmp((char *) first, "ID5") == 0) {
                idSensor_addId(sensorEvaDts->idSensor, evadtsDataBlock);
            } else if (regexec(&paRegex, first, 0, NULL, 0) == 0) {
                if (strcmp((char *) first, "PA1") == 0) {                           // FIXME Check me!
                    PASensor *paSensor = paSensor_build(evadtsDataBlock);

                    if (paSensor != NULL) {
                        paSensorList_addNode(sensorEvaDts->paSensorList, paSensor);
                    } else {
                        //TODO destroy all
                        sensorEvaDts = NULL;
                    }
                } else {
                    PASensor *paSensor = paSensorList_next(sensorEvaDts->paSensorList);
                    paSensor_addPA(paSensor, evadtsDataBlock);
                    paSensorList_addNode(sensorEvaDts->paSensorList, paSensor);
                }
            } else if (strcmp((char *) first, "SA2") == 0) {
                SASensor *saSensor = saSensor_build(evadtsDataBlock);
                if (saSensor != NULL) {
                    saSensorList_addNode(sensorEvaDts->saSensorList, saSensor);
                }
            }
        }

        evadtsDataBlock_destroy(evadtsDataBlock);
    }

    regfree(&paRegex);
    return sensorEvaDts;
}

