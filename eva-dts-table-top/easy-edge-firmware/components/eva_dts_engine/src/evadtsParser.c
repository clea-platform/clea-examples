//
// Created by harlem88 on 09/10/20.
//

#include "evadtsParser.h"
#include "data/link.h"
#include <string.h>
#include <malloc.h>
#include "data/stringList.h"
#include <esp_log.h>
#define EVA_DTS_END "DXE"

typedef struct _linkedList Node;

EvadtsDataBlockList *evadtsParser_parse(EvadtsPayloadRaw *payloadRaw) {
    if (payloadRaw == NULL) return NULL;

    Node *lineNode = link_getLinkedListInstance();
    if(lineNode == NULL) return NULL;

    int i = 0;
    int offset = i;

    while (i < payloadRaw->size - 1) {
        if (payloadRaw->data[i] == '\r' && payloadRaw->data[i + 1] == '\n') {
            int tokenLen = i - offset;
            char *data = (char *) malloc(tokenLen + 1);
            if (data != NULL) {
                memcpy(data, &(payloadRaw->data[offset]), tokenLen);
                data[tokenLen] = '\0';
                ESP_LOGW("ParserLine", "%s", data);
                link_addNodeToEnd(lineNode, data);

                if (strstr(data, EVA_DTS_END)) {
                    i = payloadRaw->size;
                }
            }
            i = offset = i + 2;
        } else {
            i++;
        }
    }

    EvadtsDataBlockList* items = evadtsDataBlockList_getInstance();
    if (items != NULL) {
        while (link_hasNext(lineNode)) {

            StringList* evadtsLineRaw= stringList_getInstance();
            if (evadtsLineRaw != NULL) {

                char *line = link_next(lineNode);
                char* token = strtok(line, "*");

                uint8_t elementSize = 0;

                while (token != NULL) {
                    char *evaDtsElement = (char *) malloc(strlen(token) + 1);
                    if (evaDtsElement != NULL) {
                        strcpy(evaDtsElement, token);
                        stringList_addNodeToEnd(evadtsLineRaw, evaDtsElement);
                        elementSize++;
                    }
                    token = strtok(NULL, "*");
                }
                free(line);

                if (elementSize > 0) {
                    EvadtsDataBlock *dataElement = evadtsDataBlock_build(elementSize);
                    char** pDataElements = dataElement->elements;

                    int j = 0;
                    while (stringList_hasNext(evadtsLineRaw) && j < elementSize) {
                        char *element = stringList_next(evadtsLineRaw);
                        pDataElements[j] = malloc(strlen(element) + 1);
                        strcpy(pDataElements[j], element);
                        free(element);
                        j++;
                    }

                    evadtsDataBlockList_addNodeToEnd(items, dataElement);
                }
                stringList_removeInstance(evadtsLineRaw);

            }
        }
    }

    link_removeLinkedListInstance(lineNode);
    return items;
}
