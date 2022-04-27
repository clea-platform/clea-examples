//
// Created by harlem88 on 14/12/20.
//

#ifndef EVA_DTS_ENGINE_MICRO_EVADTSDATABLOCK_H
#define EVA_DTS_ENGINE_MICRO_EVADTSDATABLOCK_H
#include <stdint.h>

typedef struct _evadtsDataBlock {
    char** elements;
    uint8_t size;
} EvadtsDataBlock;

EvadtsDataBlock* evadtsDataBlock_build(uint8_t);
void evadtsDataBlock_destroy(EvadtsDataBlock*);

#endif //EVA_DTS_ENGINE_MICRO_EVADTSDATABLOCK_H
