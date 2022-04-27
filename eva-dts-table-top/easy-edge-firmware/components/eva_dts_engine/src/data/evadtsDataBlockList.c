//
// Created by harlem88 on 11/12/20.
//
#include "data/evadtsDataBlockList.h"

EvadtsDataBlockList *evadtsDataBlockList_getInstance() {
    return link_getLinkedListInstance();
}

int evadtsDataBlockList_getSize(EvadtsDataBlockList* list){
    return link_getSize(list);
}

void evadtsDataBlockList_removeInstance(EvadtsDataBlockList *list) {
    link_removeLinkedListInstance(list);
}

void evadtsDataBlockList_addNode(EvadtsDataBlockList *list, EvadtsDataBlock* data) {
    link_addNode(list, data);
}

void evadtsDataBlockList_addNodeToEnd(EvadtsDataBlockList *list, EvadtsDataBlock* data) {
    link_addNodeToEnd(list, data);
}

EvadtsDataBlock* evadtsDataBlockList_next(EvadtsDataBlockList *list) {
    EvadtsDataBlock* data = link_next(list);
    return data;
}

bool evadtsDataBlockList_hasNext(EvadtsDataBlockList* list){
    return link_hasNext(list);
}