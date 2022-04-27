//
// Created by harlem88 on 11/12/20.
//

#ifndef EVA_DTS_ENGINE_MICRO_EVADTSCONFIG_H
#define EVA_DTS_ENGINE_MICRO_EVADTSCONFIG_H

#include <stdint.h>
#include "sensor/evadtsSensor.h"

typedef struct _evadtsConfig {
    EvaDtsSensor* sensors;
    int sensorsSize;
    int64_t transmissionHour;
    int64_t initTransmissionDelayMS;
} EvadtsConfig;

EvadtsConfig* evadtsConfig_read(char*);
void evadtsConfig_destroy(EvadtsConfig*);

#endif //EVA_DTS_ENGINE_MICRO_EVADTSCONFIG_H
