//
// Created by harlem88 on 11/12/20.
//

#ifndef EVADTSITEMLIST_H
#define EVADTSITEMLIST_H

#include "link.h"
#include "evadtsDataBlock.h"

typedef struct _linkedList EvadtsDataBlockList;

EvadtsDataBlockList* evadtsDataBlockList_getInstance();
int evadtsDataBlockList_getSize(EvadtsDataBlockList*);
void evadtsDataBlockList_removeInstance(EvadtsDataBlockList * list);
void evadtsDataBlockList_addNode(EvadtsDataBlockList*, EvadtsDataBlock*);
void evadtsDataBlockList_addNodeToEnd(EvadtsDataBlockList*, EvadtsDataBlock*);
bool evadtsDataBlockList_hasNext(EvadtsDataBlockList *);
EvadtsDataBlock* evadtsDataBlockList_next(EvadtsDataBlockList *);

#endif //EVADTSITEMLIST_H
