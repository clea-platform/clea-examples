//
// Created by harlem88 on 17/12/20.
//
#include "data/saSensorList.h"
#include "data/link.h"

SaSensorList* saSensorList_getInstance(){
    return link_getLinkedListInstance();
}

int saSensorList_getSize(SaSensorList* list){
    return link_getSize(list);
}

void saSensorList_removeInstance(SaSensorList* list){
    link_removeLinkedListInstance(list);
}

void saSensorList_addNode(SaSensorList* list, SASensor* sensor){
    link_addNode(list, sensor);
}

void saSensorList_addNodeToEnd(SaSensorList* list, SASensor* sensor){
    link_addNodeToEnd(list, sensor);
}

bool saSensorList_hasNext(SaSensorList* list){
    return link_hasNext(list);
}

SASensor* saSensorList_next(SaSensorList* list){
    return link_next(list);
}