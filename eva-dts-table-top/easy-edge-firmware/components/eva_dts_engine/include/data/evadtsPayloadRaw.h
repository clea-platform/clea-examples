//
// Created by harlem88 on 23/12/20.
//

#ifndef EVA_DTS_ENGINE_MICRO_EVADTSPAYLOADRAW_H
#define EVA_DTS_ENGINE_MICRO_EVADTSPAYLOADRAW_H

typedef struct _evadtsPayloadRaw{
    char* data;
    int size;
}EvadtsPayloadRaw;

void evadtsPayloadRaw_destroy(EvadtsPayloadRaw *);

#endif //EVA_DTS_ENGINE_MICRO_EVADTSPAYLOADRAW_H
