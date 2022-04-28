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

EvadtsEngine *evadtsEngine_init(char *config_raw, udp_remote_debugger_t *debugger) {
    EvadtsEngine *engine            = NULL;
    EvadtsConfig *evadtsConfig      = evadtsConfig_read(config_raw);
    EvadtsPayloadRaw *payloadRaw    = NULL;

    if (evadtsConfig == NULL)
        return NULL;

    engine = malloc(sizeof(EvadtsEngine));
    memset (engine, '\0', sizeof(EvadtsEngine));


    if (engine != NULL) {
        engine->debugger    = debugger;
        engine->data        = evadtsConfig;
        engine->collectData = &collectData;

        int retry = 0;
        int retryMax = 3;
        EvaDtsAudit *evaDtsAudit = NULL;

        while (retry < retryMax) {
            ESP_LOGI(TAG, "read init Free memory: %d bytes", esp_get_free_heap_size());

#ifdef CONFIG_USE_RECORDED_DATA
            extern uint8_t report_example_start[] asm("_binary_evoca_kalea_report_txt_start");
            extern uint8_t report_example_end[] asm("_binary_evoca_kalea_report_txt_end");
            //ESP_LOGI (TAG, "Loading already loaded data from %ld to %ld..\n%s", (long int) report_example_start, (long int) report_example_end, report_example_start);

            // Copying report data to payloadRaw variable
            payloadRaw  = malloc (sizeof(EvadtsPayloadRaw));
            memset (payloadRaw, '\0', sizeof(EvadtsPayloadRaw));
            payloadRaw->size    = report_example_end - report_example_start;
            payloadRaw->data    = malloc (payloadRaw->size);
            memset (payloadRaw->data, '\0', payloadRaw->size);
            memcpy (payloadRaw->data, report_example_start, payloadRaw->size-1);

            //ESP_LOGI (TAG, "Copied data:\n%s", payloadRaw->data);
#else
            vTaskDelay (pdMS_TO_TICKS(50));
            payloadRaw  = evadtsRetriever_readDataCollection(false);
#endif

            if (payloadRaw != NULL) {
                EvadtsDataBlockList *evadtsDataBlockList = evadtsParser_parse(payloadRaw);
                evadtsPayloadRaw_destroy(payloadRaw);
                ESP_LOGW(TAG, "parse Free memory: %d bytes", esp_get_free_heap_size());
                vTaskDelay (pdMS_TO_TICKS(50));
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
        ESP_LOGI (TAG, "Destroying everything!");
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

