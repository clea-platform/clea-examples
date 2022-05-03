//
// Created by harlem88 on 23/06/20.
//
#ifndef EVADTSENGINE_H
#define EVADTSENGINE_H

#include "sensor/evadtsAudit.h"
#include "data/evadtsSensorList.h"
#include "evadtsConfig.h"
#include <udp_remote_debugger.h>
#include <esp_err.h>

typedef struct _evadtsEngine {
    EvadtsConfig* data;
    void* timerArg;
    udp_remote_debugger_t *debugger;
    EvadtsSensorList* (*collectData)(struct _evadtsEngine *this);
    EvaDtsAudit* (*get_audit) (struct _evadtsEngine*);
} EvadtsEngine;

EvadtsEngine* evadtsEngine_init(char*, udp_remote_debugger_t *debugger);
EvaDtsAudit* evadtsEngine_read();
void evadtsEngine_destroy(EvadtsEngine*);

#endif