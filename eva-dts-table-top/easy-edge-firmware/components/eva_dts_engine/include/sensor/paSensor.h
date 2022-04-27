//
// Created by harlem88 on 09/10/20.
//

#ifndef EVA_DTS_PASENSOR_H
#define EVA_DTS_PASENSOR_H

#include "data/evadtsDataBlock.h"

typedef struct paSensor{
    char* productId;
    float* prize;
    int* valuePaidSinceInit;
    int* productsVendedSinceInit;
    int* numberFreeVendsSinceInit;
}PASensor;

PASensor* paSensor_build(EvadtsDataBlock*);
void paSensor_addPA(PASensor*, EvadtsDataBlock*);
void paSensor_destroy(PASensor*);

#endif //EVA_DTS_PASENSOR_H
