//
// Created by harlem88 on 09/10/20.
//
#include "sensor/paSensor.h"
#include <string.h>
#include <stdlib.h>
#include <esp_log.h>

PASensor* paSensor_build(EvadtsDataBlock* evadtsDataBlock){
    PASensor *paSensor= malloc(sizeof(PASensor));
    if( paSensor == NULL || evadtsDataBlock == NULL || evadtsDataBlock->elements == NULL) return NULL;

    paSensor->productId = NULL;
    paSensor->prize = NULL;
    paSensor->valuePaidSinceInit = NULL;
    paSensor->productsVendedSinceInit = NULL;
    paSensor->numberFreeVendsSinceInit = NULL;

    paSensor_addPA(paSensor, evadtsDataBlock);
    return paSensor;
}

void parsePa1(PASensor *paSensor, EvadtsDataBlock *evadtsDataBlock) {
    if (evadtsDataBlock->size > 1) {
        char *id = evadtsDataBlock->elements[1];

        paSensor->productId = malloc(strlen(id) + 1);

        if (paSensor->productId != NULL) {
            strcpy(paSensor->productId, id);
        }

        if (evadtsDataBlock->size > 2) {
            char *prize = evadtsDataBlock->elements[2];
            paSensor->prize = malloc(sizeof(float));
            if (paSensor->prize != NULL) *paSensor->prize = strtof((char *) prize, NULL);
        } else {
            paSensor->prize = NULL;
        }

    } else {
        paSensor->productId = NULL;
        paSensor->prize = NULL;
    }
}

void parsePa2(PASensor *paSensor, EvadtsDataBlock *evadtsDataBlock) {
    if (evadtsDataBlock->size > 1 && evadtsDataBlock->elements[1] != NULL) {
        char *productsVendedSinceInit = evadtsDataBlock->elements[1];
        paSensor->productsVendedSinceInit = malloc(sizeof(int));
        if (paSensor->productsVendedSinceInit != NULL)
            *paSensor->productsVendedSinceInit = (int) strtol((char *) productsVendedSinceInit, NULL, 10);
    } else {
        paSensor->productsVendedSinceInit = NULL;
    }

    if (evadtsDataBlock->size > 2 && evadtsDataBlock->elements[2] != NULL) {
        char *valuePaidSinceInit = evadtsDataBlock->elements[2];
        paSensor->valuePaidSinceInit = malloc(sizeof(int));
        if (paSensor->valuePaidSinceInit != NULL)
            *paSensor->valuePaidSinceInit = (int) strtol((char *) valuePaidSinceInit, NULL, 10);
    } else {
        paSensor->valuePaidSinceInit = NULL;
    }
}

void parsePa4(PASensor *paSensor, EvadtsDataBlock *evadtsDataBlock) {
    if (evadtsDataBlock->size > 1 && evadtsDataBlock->elements[1] != NULL) {
        char *numberFreeVendsSinceInit = evadtsDataBlock->elements[1];
        paSensor->numberFreeVendsSinceInit = malloc(sizeof(int));
        if (paSensor->numberFreeVendsSinceInit != NULL)
            *paSensor->numberFreeVendsSinceInit = (int) strtol((char *) numberFreeVendsSinceInit, NULL, 10);
    } else {
        paSensor->numberFreeVendsSinceInit = NULL;
    }
}

void paSensor_addPA(PASensor* paSensor, EvadtsDataBlock* evadtsDataBlock) {
    if(paSensor == NULL || evadtsDataBlock->size < 1) return;
    
    char* paIdentifier = evadtsDataBlock->elements[0];
    switch (paIdentifier[2]) {
        case '1':
            parsePa1(paSensor, evadtsDataBlock);
            break;
        case '2':
            parsePa2(paSensor, evadtsDataBlock);
            break;
        case '4':
            parsePa4(paSensor, evadtsDataBlock);
            break;
    }
}

void paSensor_destroy(PASensor* paSensor){
    if(paSensor!= NULL){
        if(paSensor->productId != NULL) free(paSensor->productId);
        if(paSensor->prize != NULL) free(paSensor->prize);
        if(paSensor->productsVendedSinceInit != NULL) free(paSensor->productsVendedSinceInit);
        if(paSensor->numberFreeVendsSinceInit != NULL) free(paSensor->numberFreeVendsSinceInit);
        if(paSensor->valuePaidSinceInit != NULL) free(paSensor->valuePaidSinceInit);
        free(paSensor);
    }
}