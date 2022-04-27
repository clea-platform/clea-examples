//
// Created by harlem88 on 11/12/20.
//

#include "data/stringList.h"
#include "data/link.h"

StringList *stringList_getInstance() {
    return link_getLinkedListInstance();
}

int stringList_getSize(StringList* list){
    return link_getSize(list);
}

void stringList_removeInstance(StringList *list) {
    link_removeLinkedListInstance(list);
}

void stringList_addNode(StringList *list, char* data) {
    link_addNode(list, data);
}

void stringList_addNodeToEnd(StringList *list, char* data) {
    link_addNodeToEnd(list, data);
}

char* stringList_next(StringList *list) {
    char* data = link_next(list);
    return data;
}

char* stringList_nextFromOffset(StringList *list, int offset) {
    char* data = link_nextFromOffset(list, offset);
    return data;
}

bool stringList_hasNext(StringList* list){
    return link_hasNext(list);
}

