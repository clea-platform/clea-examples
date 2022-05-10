//
// Created by harlem88 on 17/12/20.
//
#include "sensor/saSensor.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void str_tolower(char* str) {
    for ( ; *str; ++str) *str = tolower((unsigned char) *str);
}

void str_replace(char *str, char find, char replace) {
    for (; *str; ++str) {
        if (*str == find)*str = replace;
    }
}

SASensor* saSensor_build(EvadtsDataBlock*evadtsDataBlock){
    SASensor *saSensor= malloc(sizeof(SASensor));
    if( saSensor == NULL || evadtsDataBlock == NULL || evadtsDataBlock->elements == NULL) return NULL;

    saSensor->quantityVendedSinceInit= 0;

    saSensor_addSa(saSensor, evadtsDataBlock);
    return saSensor;
}

void saSensor_addSa(SASensor *saSensor, EvadtsDataBlock *evadtsDataBlock) {
    if (saSensor == NULL || evadtsDataBlock->size < 1) return;
    char *paIdentifier = evadtsDataBlock->elements[0];
    if (paIdentifier[2] == '2') {
        if (evadtsDataBlock->size > 1 && evadtsDataBlock->elements[1] != NULL) {
            char *ingredientNumber = evadtsDataBlock->elements[1];
            saSensor->ingredientNumber = malloc(strlen(ingredientNumber) + 1);
            if (saSensor->ingredientNumber != NULL) {
                strcpy(saSensor->ingredientNumber, ingredientNumber);
                str_tolower(saSensor->ingredientNumber);
                str_replace(saSensor->ingredientNumber, ' ', '_');
            } else {
                saSensor->ingredientNumber = NULL;
            }
        } else {
            saSensor->ingredientNumber = NULL;
        }

        if (evadtsDataBlock->size > 3 && evadtsDataBlock->elements[3] != NULL) {
            char *quantityVendedSinceInit = evadtsDataBlock->elements[3];
            saSensor->quantityVendedSinceInit = (int) strtol((char *) quantityVendedSinceInit, NULL, 10);
        } else {
            saSensor->quantityVendedSinceInit = 0;
        }
    }
}

void saSensor_destroy(SASensor* saSensor){
    if(saSensor != NULL){
        if(saSensor->ingredientNumber != NULL){
            free(saSensor->ingredientNumber);
        }
        free(saSensor);
    }
}
