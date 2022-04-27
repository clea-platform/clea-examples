//
// Created by harlem88 on 09/10/20.
//

#ifndef EVA_DTS_IDSENSOR_H
#define EVA_DTS_IDSENSOR_H

#include "data/evadtsDataBlock.h"

typedef struct idSensor{
    char* machine_sn;
    int64_t machine_time_sec;
}IDSensor;

IDSensor* idSensor_build(EvadtsDataBlock*);
void idSensor_addId(IDSensor* idSensor, EvadtsDataBlock* evadtsDataBlock);
void idSensor_destroy(IDSensor*);

#endif //EVA_DTS_IDSENSOR_H
