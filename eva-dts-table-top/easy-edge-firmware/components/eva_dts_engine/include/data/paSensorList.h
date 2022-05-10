//
// Created by harlem88 on 14/12/20.
//

#ifndef EVA_DTS_ENGINE_MICRO_PASENSORLIST_H
#define EVA_DTS_ENGINE_MICRO_PASENSORLIST_H

#include <stdbool.h>
#include "sensor/paSensor.h"

typedef struct _linkedList PaSensorList;

PaSensorList* paSensorList_getInstance();
int paSensorList_getSize(PaSensorList*);
void paSensorList_removeInstance(PaSensorList*);
void paSensorList_addNode(PaSensorList*, PASensor*);
void paSensorList_addNodeToEnd(PaSensorList*, PASensor*);
bool paSensorList_hasNext(PaSensorList*);
PASensor* paSensorList_next(PaSensorList*);

#endif //EVA_DTS_ENGINE_MICRO_PASENSORLIST_H
