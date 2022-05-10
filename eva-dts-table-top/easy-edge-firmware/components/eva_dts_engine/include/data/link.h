//
// Created by harlem88 on 06/11/20.
//

#ifndef EVADTS_LINK_H
#define EVADTS_LINK_H

#include <stdbool.h>

typedef void *Data;
typedef struct _linkedList LinkedList;

LinkedList* link_getLinkedListInstance();
int link_getSize(LinkedList*);
void link_removeLinkedListInstance(LinkedList* list);
void link_addNode(LinkedList*, Data);
void link_addNodeToEnd(LinkedList *list, Data data);
Data link_next(LinkedList*);
Data link_nextFromOffset(LinkedList* list, int offset);
bool link_hasNext(LinkedList*);

#endif //EVADTS_LINK_H
