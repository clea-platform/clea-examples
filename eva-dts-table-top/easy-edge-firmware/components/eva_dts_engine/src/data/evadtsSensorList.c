//
// Created by harlem88 on 16/12/20.
//

#include "data/evadtsSensorList.h"
#include "data/link.h"

EvadtsSensorList* evadtsSensorList_getInstance(){
    return link_getLinkedListInstance();
}

int evadtsSensorList_getSize(EvadtsSensorList* list){
    return link_getSize(list);
}

void evadtsSensorList_removeInstance(EvadtsSensorList* list){
    link_removeLinkedListInstance(list);
}

void evadtsSensorList_addNode(EvadtsSensorList* list, EvaDtsSensor* sensor){
    link_addNode(list, sensor);
}

void evadtsSensorList_addNodeToEnd(EvadtsSensorList* list, EvaDtsSensor* sensor){
    link_addNodeToEnd(list, sensor);
}

bool evadtsSensorList_hasNext(EvadtsSensorList* list){
    return link_hasNext(list);
}

EvaDtsSensor* evadtsSensorList_next(EvadtsSensorList* list){
    return link_next(list);
}