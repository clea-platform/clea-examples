//
// Created by harlem88 on 23/06/20.
//
#ifndef EVADTSENGINE_H
#define EVADTSENGINE_H

#include "sensor/evadtsAudit.h"
#include "data/evadtsSensorList.h"
#include "evadtsConfig.h"
#include <esp_err.h>

typedef struct _evadtsEngine {
    EvadtsConfig* data;
    void* timerArg;
    EvadtsSensorList* (*collectData)(struct _evadtsEngine *this);
} EvadtsEngine;

EvadtsEngine* evadtsEngine_init(char*);
EvaDtsAudit* evadtsEngine_read();
void evadtsEngine_destroy(EvadtsEngine*);

#endif