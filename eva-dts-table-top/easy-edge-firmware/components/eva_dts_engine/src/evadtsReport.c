//
// Created by harlem88 on 17/12/20.
//
#include "evadtsReport.h"
#include "repository/paRepository.h"
#include "repository/saRepository.h"
#include <string.h>
#include <esp_log.h>

static void addPaSensors(EvadtsConfig* evadtsConfig, EvaDtsAudit* evaDtsAudit, EngineRepository* engineRepository, EvadtsSensorList* evadtsSensorList);
static void addSaSensors(EvadtsConfig* evadtsConfig, EvaDtsAudit* evaDtsAudit, EngineRepository* engineRepository, EvadtsSensorList* evadtsSensorList);

EvadtsSensorList* evadtsReport_getSensors(EvadtsConfig* evadtsConfig, EvaDtsAudit* evaDtsAudit){
    EngineRepository* engineRepository = engineRepository_init();
    EvadtsSensorList* evadtsSensorList= evadtsSensorList_getInstance();

    addPaSensors(evadtsConfig, evaDtsAudit, engineRepository, evadtsSensorList);
    addSaSensors(evadtsConfig, evaDtsAudit, engineRepository, evadtsSensorList);

    engineRepository_destroy(engineRepository);
    return evadtsSensorList;
}

esp_err_t getTotalProductsValue(PASensor *paSensorInit, PASensor *paSensorCurrent, int* total) {
    if (paSensorCurrent != NULL && paSensorCurrent->numberFreeVendsSinceInit != NULL &&
        paSensorCurrent->productsVendedSinceInit != NULL) {

        int vendedTotalValue = *paSensorCurrent->productsVendedSinceInit;
        int freeTotalValue = *paSensorCurrent->numberFreeVendsSinceInit;

        int vendedInit = 0;
        int freeInit = 0;

        if (paSensorInit != NULL) {
            if (paSensorInit->numberFreeVendsSinceInit != NULL) freeInit = *paSensorInit->numberFreeVendsSinceInit;
            if (paSensorInit->productsVendedSinceInit != NULL) vendedInit = *paSensorInit->productsVendedSinceInit;
        }

        *total= vendedTotalValue + freeTotalValue - vendedInit - freeInit;
        return ESP_OK;
    }else{
        return ESP_FAIL;
    }
}

esp_err_t getFreeVends(PASensor *paSensorInit, PASensor *paSensorCurrent, int *total) {
    if (paSensorCurrent != NULL && paSensorCurrent->numberFreeVendsSinceInit != NULL) {

        int freeTotalValue = *paSensorCurrent->numberFreeVendsSinceInit;

        int freeInit = 0;

        if (paSensorInit != NULL && paSensorInit->numberFreeVendsSinceInit != NULL) freeInit = *paSensorInit->numberFreeVendsSinceInit;

        *total = freeTotalValue - freeInit;
        return ESP_OK;
    } else {
        return ESP_FAIL;
    }
}

esp_err_t getPaidVended(PASensor *paSensorInit, PASensor *paSensorCurrent, int* total) {
    if (paSensorCurrent != NULL && paSensorCurrent->productsVendedSinceInit != NULL) {

        int vendedTotalValue = *paSensorCurrent->productsVendedSinceInit;

        int vendedInit = 0;

        if (paSensorInit != NULL && paSensorInit->productsVendedSinceInit != NULL) vendedInit = *paSensorInit->productsVendedSinceInit;

        *total= vendedTotalValue - vendedInit;
        return ESP_OK;
    }else{
        return ESP_FAIL;
    }
}

esp_err_t getCashedTotal(PASensor *paSensorInit, PASensor *paSensorCurrent, int *total) {
    if (paSensorCurrent != NULL && paSensorCurrent->valuePaidSinceInit != NULL) {

        int cashedTotal = *paSensorCurrent->valuePaidSinceInit;

        int cashedInit = 0;

        if (paSensorInit != NULL && paSensorInit->valuePaidSinceInit != NULL)
            cashedInit = *paSensorInit->valuePaidSinceInit;

        *total = cashedTotal - cashedInit;
        return ESP_OK;
    } else {
        return ESP_FAIL;
    }
}


static void addPaSensors(EvadtsConfig* evadtsConfig, EvaDtsAudit* evaDtsAudit, EngineRepository* engineRepository, EvadtsSensorList* evadtsSensorList){
    while (paSensorList_hasNext(evaDtsAudit->paSensorList)) {
        PASensor *paSensor = paSensorList_next(evaDtsAudit->paSensorList);
        if (paSensor != NULL && paSensor->productId != NULL) {

            PASensor* paSensorInit= paRepository_get(engineRepository, paSensor->productId);

            for (int i = 0; i < evadtsConfig->sensorsSize; i++) {
                EvaDtsSensor *sensor = NULL;
                int total;

                if (strcmp(evadtsConfig->sensors[i].id, paSensor->productId) == 0) {
                    if(getTotalProductsValue(paSensorInit, paSensor, &total) == ESP_OK){
                        sensor = &evadtsConfig->sensors[i];
                        sensor->value.fValue = (float) total;
                    }
                } else {
                    char strTemp[51];
                    strcpy(strTemp, evadtsConfig->sensors[i].id);

                    char *sensorId = strtok(strTemp, "_");
                    if (sensorId != NULL && strcmp(sensorId, paSensor->productId) == 0) {
                        if (strstr(evadtsConfig->sensors[i].id, PA_FREE_KEY)) {
                            if (getFreeVends(paSensorInit, paSensor, &total) == ESP_OK) {
                                sensor = &evadtsConfig->sensors[i];
                                sensor->value.fValue = (float) total;
                            }
                        } else if (strstr(evadtsConfig->sensors[i].id, PA_PAID_KEY)) {
                            if (getPaidVended(paSensorInit, paSensor, &total) == ESP_OK) {
                                sensor = &evadtsConfig->sensors[i];
                                sensor->value.fValue = (float) total;
                            }
                        } else if (strstr(evadtsConfig->sensors[i].id, PA_CASHED_KEY)) {
                            if (getCashedTotal(paSensorInit, paSensor, &total) == ESP_OK) {
                                sensor = &evadtsConfig->sensors[i];
                                sensor->value.fValue = (float) total;
                            }
                        }
                    }
                }

                if (sensor != NULL) {
                    evadtsSensorList_addNode(evadtsSensorList, sensor);
                    /*ESP_LOGW("PA sensors", "config id-%s  map-%s product id-%s %d", sensor->id, sensor->map,
                             paSensor->productId, (int) sensor->value.fValue);*/
                }
            }

            paRepository_set(engineRepository, paSensor);
            paSensor_destroy(paSensor);
            paSensor_destroy(paSensorInit);
        }
    }
}

static void addSaSensors(EvadtsConfig* evadtsConfig, EvaDtsAudit* evaDtsAudit, EngineRepository* engineRepository, EvadtsSensorList* evadtsSensorList){
    while (saSensorList_hasNext(evaDtsAudit->saSensorList)) {
        SASensor *saSensor = saSensorList_next(evaDtsAudit->saSensorList);
        if (saSensor != NULL && saSensor->ingredientNumber != NULL) {
            SASensor* saSensorInit = saRepository_get(engineRepository, saSensor);

            int quantityVendedSinceInit = saSensor->quantityVendedSinceInit -
                                          (saSensorInit != NULL ? saSensorInit->quantityVendedSinceInit : 0);

            for (int i = 0; i < evadtsConfig->sensorsSize; i++) {
                if (strcmp(evadtsConfig->sensors[i].id, saSensor->ingredientNumber) == 0) {
                    EvaDtsSensor* sensor = &evadtsConfig->sensors[i];

                    sensor->value.fValue = (float) quantityVendedSinceInit;
                    evadtsSensorList_addNode(evadtsSensorList, sensor);
                    /*ESP_LOGW("SA sensors", "config id-%s  map-%s product id-%s %d", sensor->id, sensor->map,
                             saSensor->ingredientNumber, (int) sensor->value.fValue);*/
                }
            }

            saRepository_set(engineRepository, saSensor);
            saSensor_destroy(saSensor);
            saSensor_destroy(saSensorInit);
        }
    }
}

