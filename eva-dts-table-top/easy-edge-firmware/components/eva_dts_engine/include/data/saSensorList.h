//
// Created by harlem88 on 17/12/20.
//

#ifndef EVA_DTS_ENGINE_MICRO_SASENSORLIST_H
#define EVA_DTS_ENGINE_MICRO_SASENSORLIST_H

#include <stdbool.h>
#include "sensor/saSensor.h"

typedef struct _linkedList SaSensorList;

SaSensorList* saSensorList_getInstance();
int saSensorList_getSize(SaSensorList*);
void saSensorList_removeInstance(SaSensorList*);
void saSensorList_addNode(SaSensorList*, SASensor*);
void saSensorList_addNodeToEnd(SaSensorList*, SASensor*);
bool saSensorList_hasNext(SaSensorList*);
SASensor* saSensorList_next(SaSensorList*);


#endif //EVA_DTS_ENGINE_MICRO_SASENSORLIST_H
