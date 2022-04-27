//
// Created by harlem88 on 23/06/20.
//

#include "evadtsEngine.h"
#include <esp_log.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include "evadtsRetriever.h"
#include "evadtsParser.h"
#include "evadtsHandler.h"
#include "data/evadtsSensorList.h"
#include "repository/paRepository.h"
#include "repository/saRepository.h"
#include "evadtsReport.h"

static const char* TAG = "EvadtsEngine";

static EvadtsSensorList* collectData();
void saveInitData(EvaDtsAudit *evaDtsAudit);

EvadtsEngine *evadtsEngine_init(char *config_raw) {
    ESP_LOGI(TAG, "===== INIT =====");
    EvadtsEngine *engine = NULL;
    EvadtsConfig *evadtsConfig = evadtsConfig_read(config_raw);

    if (evadtsConfig == NULL) return NULL;

    engine = malloc(sizeof(EvadtsEngine));

    if (engine != NULL) {
        engine->data = evadtsConfig;
        engine->collectData = &collectData;

        int retry = 0;
        int retryMax = 3;
        EvaDtsAudit *evaDtsAudit = NULL;

        while (retry < retryMax) {
            ESP_LOGI(TAG, "read init Free memory: %d bytes", esp_get_free_heap_size());
            EvadtsPayloadRaw *payloadRaw = evadtsRetriever_readDataCollection(false);
            if (payloadRaw != NULL) {
                EvadtsDataBlockList *evadtsDataBlockList = evadtsParser_parse(payloadRaw);
                evadtsPayloadRaw_destroy(payloadRaw);
                ESP_LOGW(TAG, "parse Free memory: %d bytes", esp_get_free_heap_size());
                if (evadtsDataBlockList != NULL) {
                    evaDtsAudit = evadtsHandler_handleSensors(evadtsDataBlockList);
                    ESP_LOGW(TAG, "handle end Free memory: %d bytes", esp_get_free_heap_size());
                    if (evaDtsAudit != NULL) {
                        if (evaDtsAudit->idSensor) {
                            if (evadtsConfig->transmissionHour >= evaDtsAudit->idSensor->machine_time_sec) {
                                evadtsConfig->initTransmissionDelayMS =
                                        (evadtsConfig->transmissionHour - evaDtsAudit->idSensor->machine_time_sec) *
                                        1000L;
                            } else {
                                evadtsConfig->initTransmissionDelayMS =
                                        (86400L -
                                         (evaDtsAudit->idSensor->machine_time_sec - evadtsConfig->transmissionHour)) *
                                        1000L; //86400 = 24h -> I wrap to the following day
                            }
                        } else {
                            ESP_LOGW(TAG, "evadts AUDIT empty ID1");
                            evadtsConfig->initTransmissionDelayMS = evadtsConfig->transmissionHour * 1000L;
                        }

                        saveInitData(evaDtsAudit);
                        evadtsAudit_destroy(evaDtsAudit);
                        retry = retryMax;

                    } else {
                        ESP_LOGW(TAG, "evadts Audit empty");
                        retry++;
                    }
                    evadtsDataBlockList_removeInstance(evadtsDataBlockList);
                } else {
                    ESP_LOGW(TAG, "evadts Block data empty");
                    retry++;
                }
            } else {
                ESP_LOGW(TAG, "payload_raw empty");
                retry++;
            }

            vTaskDelay(5 * 1000 / portTICK_PERIOD_MS);
        }
    } else {
        evadtsConfig_destroy(evadtsConfig);
    }

    return engine;
}

void saveInitData(EvaDtsAudit *evaDtsAudit) {
    EngineRepository* engineRepository = engineRepository_init();

    if (engineRepository != NULL) {

        while (paSensorList_hasNext(evaDtsAudit->paSensorList)) {
            paRepository_set(engineRepository, paSensorList_next(evaDtsAudit->paSensorList));
        }

        while (saSensorList_hasNext(evaDtsAudit->saSensorList)) {
            saRepository_set(engineRepository, saSensorList_next(evaDtsAudit->saSensorList));
        }

        engineRepository_destroy(engineRepository);
    }
}

EvadtsSensorList *collectData(EvadtsEngine* this) {
    int retry = 0;
    int retryMax = 3;
    EvadtsSensorList *sensors = NULL;

    while (retry < retryMax) {
        ESP_LOGI(TAG, "read init Free memory: %d bytes", esp_get_free_heap_size());
        EvadtsPayloadRaw *payloadRaw = evadtsRetriever_readDataCollection(false);

        if (payloadRaw != NULL) {
            EvadtsDataBlockList *evadtsDataBlockList = evadtsParser_parse(payloadRaw);
            evadtsPayloadRaw_destroy(payloadRaw);
            EvaDtsAudit* evaDtsAudit = evadtsHandler_handleSensors(evadtsDataBlockList);
            ESP_LOGW(TAG, "handle end Free memory: %d bytes", esp_get_free_heap_size());
            evadtsDataBlockList_removeInstance(evadtsDataBlockList);
            sensors = evadtsReport_getSensors(this->data, evaDtsAudit);
            ESP_LOGW(TAG, "getSensors end Free memory: %d bytes", esp_get_free_heap_size());
            evadtsAudit_destroy(evaDtsAudit);
            ESP_LOGI(TAG, "read end Free memory: %d bytes", esp_get_free_heap_size());
            retry = retryMax;
        } else {
            ESP_LOGW(TAG, "payload_raw empty");
            retry++;
            vTaskDelay( 60 * 1000 / portTICK_PERIOD_MS);
        }
    }

    return sensors;
}

void evadtsEngine_destroy(EvadtsEngine *engine){
    if( engine != NULL){
        if(engine->data != NULL){
            evadtsConfig_destroy(engine->data);
        }

        free(engine);
        engine=NULL;
    }
}

