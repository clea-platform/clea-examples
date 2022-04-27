//
// Created by harlem88 on 14/12/20.
//
#include "data/paSensorList.h"
#include "data/link.h"

PaSensorList* paSensorList_getInstance(){
    return link_getLinkedListInstance();
}

int paSensorList_getSize(PaSensorList* list){
        return link_getSize(list);
}

void paSensorList_removeInstance(PaSensorList* list){
    link_removeLinkedListInstance(list);
}

void paSensorList_addNode(PaSensorList* list, PASensor* sensor){
    link_addNode(list, sensor);
}

void paSensorList_addNodeToEnd(PaSensorList* list, PASensor* sensor){
    link_addNodeToEnd(list, sensor);
}

bool paSensorList_hasNext(PaSensorList* list){
    return link_hasNext(list);
}

PASensor* paSensorList_next(PaSensorList* list){
    return link_next(list);
}