//
// Created by harlem88 on 09/10/20.
//

#include "sensor/evadtsAudit.h"
#include <malloc.h>
#include <string.h>
#include <sdkconfig.h>

EvaDtsAudit* evadtsAudit_build(){
    EvaDtsAudit* sensor= malloc(sizeof(*sensor));
    if( sensor == NULL) return NULL;

    sensor->idSensor = NULL;
    sensor->paSensorList = NULL;
    sensor->saSensorList = NULL;

    return sensor;
}

void evadtsAudit_destroy(EvaDtsAudit* evaDtsAudit){
    if(evaDtsAudit != NULL){
        idSensor_destroy(evaDtsAudit->idSensor);
        paSensorList_removeInstance(evaDtsAudit->paSensorList);
        saSensorList_removeInstance(evaDtsAudit->saSensorList);
        free(evaDtsAudit);
    }
}

enum EVADTS_SENSOR_ENUM {
    FOREACH_EVADTS_SENSOR(GENERATE_ENUM)
};

static const char *EVADTS_SENSOR_NAME[] = {
        FOREACH_EVADTS_SENSOR(GENERATE_STRING)
};

const char *evadtsSensor_mapNameFromId(const char *choice) {
    if (strcmp(choice, CONFIG_EVADTS_MACHINE_ID) == 0) {
        return EVADTS_SENSOR_NAME[machine_id];
    } else if (strcmp(choice, CONFIG_EVADTS_SENSOR_ESPRESSO) == 0) {
        return EVADTS_SENSOR_NAME[espresso];
    } else if (strcmp(choice, CONFIG_EVADTS_SENSOR_COFFEE) == 0) {
        return EVADTS_SENSOR_NAME[coffee];
    } else if (strcmp(choice, CONFIG_EVADTS_SENSOR_AMERICANO) == 0) {
        return EVADTS_SENSOR_NAME[americano];
    }
    return NULL;
}

