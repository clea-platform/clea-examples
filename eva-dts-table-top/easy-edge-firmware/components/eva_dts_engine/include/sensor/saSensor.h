//
// Created by harlem88 on 17/12/20.
//

#ifndef EVA_DTS_ENGINE_MICRO_SASENSOR_H
#define EVA_DTS_ENGINE_MICRO_SASENSOR_H

#include "data/evadtsDataBlock.h"

typedef struct saSensor{
    char* ingredientNumber;
    int quantityVendedSinceInit;
}SASensor;

SASensor* saSensor_build(EvadtsDataBlock*);
void saSensor_addSa(SASensor*, EvadtsDataBlock*);
void saSensor_destroy(SASensor*);

#endif //EVA_DTS_ENGINE_MICRO_SASENSOR_H
