//
// Created by harlem88 on 11/12/20.
//

#ifndef EVA_DTS_ENGINE_MICRO_STRINGLIST_H
#define EVA_DTS_ENGINE_MICRO_STRINGLIST_H

#include <stdbool.h>

typedef struct _linkedList StringList;

StringList* stringList_getInstance();
int stringList_getSize(StringList*);
void stringList_removeInstance(StringList*);
void stringList_addNode(StringList*, char*);
void stringList_addNodeToEnd(StringList*, char*);
bool stringList_hasNext(StringList*);
char* stringList_next(StringList*);
char*  stringList_nextFromOffset(StringList*, int);

#endif //EVA_DTS_ENGINE_MICRO_STRINGLIST_H
