//
// Created by harlem88 on 16/12/20.
//

#ifndef EVADTSSENSOTLIST_H
#define EVADTSSENSOTLIST_H

#include <stdbool.h>
#include "sensor/evadtsSensor.h"

typedef struct _linkedList EvadtsSensorList;

EvadtsSensorList* evadtsSensorList_getInstance();
int evadtsSensorList_getSize(EvadtsSensorList*);
void evadtsSensorList_removeInstance(EvadtsSensorList*);
void evadtsSensorList_addNode(EvadtsSensorList*, EvaDtsSensor*);
void evadtsSensorList_addNodeToEnd(EvadtsSensorList*, EvaDtsSensor*);
bool evadtsSensorList_hasNext(EvadtsSensorList*);
EvaDtsSensor* evadtsSensorList_next(EvadtsSensorList*);


#endif //EVADTSSENSOTLIST_H
