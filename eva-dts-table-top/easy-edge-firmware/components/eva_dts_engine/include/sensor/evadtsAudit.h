//
// Created by harlem88 on 09/10/20.
//

#ifndef EVADTS_AUDIT_H
#define EVADTS_AUDIT_H

#include "sensor/idSensor.h"
#include "sensor/paSensor.h"
#include "data/paSensorList.h"
#include "data/saSensorList.h"

#define EVADTS_SENSORS_LEN 4
#define FOREACH_EVADTS_SENSOR(EVADTS_SENSOR) \
        EVADTS_SENSOR(machine_id)     \
        EVADTS_SENSOR(espresso)     \
        EVADTS_SENSOR(coffee)       \
        EVADTS_SENSOR(americano)    \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef struct _evaDtsAudit{
    IDSensor* idSensor;
    PaSensorList* paSensorList;
    SaSensorList* saSensorList;
}EvaDtsAudit;

EvaDtsAudit* evadtsAudit_build();
void evadtsAudit_destroy(EvaDtsAudit*);
const char* evadtsSensor_mapNameFromId(const char*);

#endif //EVADTS_AUDIT_H
