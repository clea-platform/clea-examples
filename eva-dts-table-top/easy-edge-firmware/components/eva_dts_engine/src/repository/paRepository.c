//
// Created by harlem88 on 17/12/20.
//
#include "repository/paRepository.h"
#include <esp_log.h>
#include <string.h>

static const char* TAG = "PA Repository";

esp_err_t paRepository_set(EngineRepository *repository, PASensor *paSensor) {
    if (repository == NULL || paSensor == NULL || paSensor->productId == NULL) return ESP_FAIL;

    esp_err_t error = ESP_OK;

    char freeInitKey[15];
    char vendedInitKey[15];
    char cashedInitKey[15];

    sprintf(freeInitKey, "%s_%s", paSensor->productId, PA_FREE_KEY);
    sprintf(vendedInitKey, "%s_%s", paSensor->productId, PA_PAID_KEY);
    sprintf(cashedInitKey, "%s_%s", paSensor->productId, PA_CASHED_KEY);

    if (paSensor->numberFreeVendsSinceInit != NULL) {
        error = repository->setI32(repository, freeInitKey, *paSensor->numberFreeVendsSinceInit);
    }

    if (error == ESP_OK && paSensor->productsVendedSinceInit != NULL) {
        error = repository->setI32(repository, vendedInitKey, *paSensor->productsVendedSinceInit);
    }

    if (error == ESP_OK && paSensor->valuePaidSinceInit != NULL) {
        error = repository->setI32(repository, cashedInitKey, *paSensor->valuePaidSinceInit);
    }
    return error;
}

PASensor* paRepository_get(EngineRepository* repository, const char* paSensorId){
    if(repository == NULL || paSensorId == NULL) return NULL;

    PASensor* paSensor = malloc(sizeof(PASensor));
    if(paSensor == NULL ) return NULL;

    paSensor->productId= malloc(strlen(paSensorId) + 1);
    strcpy(paSensor->productId, paSensorId);

    paSensor->prize = NULL;
    paSensor->valuePaidSinceInit = malloc(sizeof(int));
    paSensor->productsVendedSinceInit = malloc(sizeof(int ));
    paSensor->numberFreeVendsSinceInit = malloc(sizeof(int ));

    char freeInitKey[15];
    char vendedInitKey[15];
    char cashedInitKey[15];

    sprintf(freeInitKey, "%s_%s", paSensor->productId, PA_FREE_KEY);
    sprintf(vendedInitKey, "%s_%s", paSensor->productId, PA_PAID_KEY);
    sprintf(cashedInitKey, "%s_%s", paSensor->productId, PA_CASHED_KEY);

    if(repository->getI32(repository, freeInitKey, paSensor->numberFreeVendsSinceInit) != ESP_OK) {
        free(paSensor->numberFreeVendsSinceInit);
        paSensor->numberFreeVendsSinceInit = NULL;
    }

    if(repository->getI32(repository, vendedInitKey, paSensor->productsVendedSinceInit) != ESP_OK){
        free(paSensor->productsVendedSinceInit);
        paSensor->productsVendedSinceInit = NULL;
    }

    if(repository->getI32(repository, cashedInitKey, paSensor->valuePaidSinceInit) != ESP_OK){
        free(paSensor->valuePaidSinceInit);
        paSensor->valuePaidSinceInit = NULL;
    }

    return paSensor;
}