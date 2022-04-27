//
// Created by harlem88 on 14/12/20.
//

#include "data/evadtsDataBlock.h"
#include <stdlib.h>

EvadtsDataBlock *evadtsDataBlock_build(uint8_t elementSize) {
    EvadtsDataBlock *dataBlock = malloc(sizeof(EvadtsDataBlock));

    if (dataBlock == NULL) return NULL;

    dataBlock->elements = (char **) malloc(elementSize * sizeof(char *));
    if (dataBlock->elements != NULL) {
        dataBlock->size = elementSize;
    } else{
        free(dataBlock);
        dataBlock = NULL;
    }

    return dataBlock;
}

void evadtsDataBlock_destroy(EvadtsDataBlock* dataBlock){
    if(dataBlock != NULL){

        if (dataBlock->elements != NULL) {
            for (uint8_t i = 0; i < dataBlock->size; i++) {
                if (dataBlock->elements[i] != NULL)
                    free(dataBlock->elements[i]);
            }
            free(dataBlock->elements);
        }

        free(dataBlock);
    }
}
