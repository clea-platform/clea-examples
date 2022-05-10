//
// Created by harlem88 on 17/12/20.
//
#include "repository/saRepository.h"
#include <esp_log.h>
#include <string.h>

static const char *TAG = "SA Repository";

esp_err_t saRepository_set(EngineRepository *repository, SASensor *saSensor) {
    if (repository == NULL || saSensor == NULL || saSensor->ingredientNumber == NULL) return ESP_FAIL;

    return repository->setI32(repository, saSensor->ingredientNumber, saSensor->quantityVendedSinceInit);
}

SASensor* saRepository_get(EngineRepository *repository, SASensor *saSensorCurrent) {
    if (repository == NULL || saSensorCurrent == NULL) return NULL;

    SASensor* saSensor = malloc(sizeof(SASensor));
    if(saSensor == NULL ) return NULL;

    saSensor->ingredientNumber= malloc(strlen(saSensorCurrent->ingredientNumber) + 1);
    if(saSensor->ingredientNumber == NULL ) return NULL;

    strcpy(saSensor->ingredientNumber, saSensorCurrent->ingredientNumber);

    saSensor->quantityVendedSinceInit= 0;

    if (repository->getI32(repository, saSensor->ingredientNumber, &saSensor->quantityVendedSinceInit) !=
        ESP_OK)
        saSensor->quantityVendedSinceInit = 0;

    return saSensor;
}