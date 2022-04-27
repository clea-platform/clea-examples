//
// Created by harlem88 on 11/12/20.
//

#include "evadtsConfig.h"
#include <cJSON.h>
#include <esp_log.h>
#include <stdio.h>
#include <string.h>

#define EVADTS_SENSORS_KEY "sensors"
#define EVADTS_SENSORS_ID_KEY "id"
#define EVADTS_SENSORS_MAP_KEY "map"
#define EVADTS_SENSORS_TYPE_KEY "data_type"

#define EVADTS_TRANSMISSION_KEY "transmission"
#define EVADTS_TRANSMISSION_H_KEY "hour"
#define EVADTS_TRANSMISSION_M_KEY "minutes"
#define EVADTS_TRANSMISSION_S_KEY "seconds"

static const char *TAG = "evadts_config";

int parse_sensor(cJSON *json_sensor, EvaDtsSensor *sensor);
int parse_transmission(const cJSON *json_transmission, EvadtsConfig *config);

void print_parsed_config(EvadtsConfig *cfg);

EvadtsConfig *evadtsConfig_read(char *config_raw) {
    EvadtsConfig *config = NULL;

    if (config_raw == NULL)
        goto parsing_error;

    cJSON *config_json = cJSON_Parse(config_raw);

    if (config_json == NULL) {

        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGW(TAG, "Error before: %s\n", error_ptr);
        }

        goto parsing_error;
    }

    const cJSON *json_sensors = cJSON_GetObjectItem(config_json, EVADTS_SENSORS_KEY);

    if (json_sensors == NULL) {
        const char *error = cJSON_GetErrorPtr();
        if (error != NULL) {
            ESP_LOGW(TAG, "Error before: %s\n", error);
        }

        goto parsing_error;
    }

    const cJSON *json_transmission = cJSON_GetObjectItem(config_json, EVADTS_TRANSMISSION_KEY);

    if (json_transmission == NULL) {
        const char *error = cJSON_GetErrorPtr();
        if (error != NULL) {
            ESP_LOGW(TAG, "Error before: %s\n", error);
        }

        goto parsing_error;
    }

    int size = cJSON_GetArraySize(json_sensors);

    config = malloc(sizeof(EvadtsConfig));
    if (config != NULL) {
        parse_transmission(json_transmission, config);
        config->sensors = malloc(size * sizeof(EvaDtsSensor));

        if (config->sensors != NULL) {
            config->sensorsSize = size;

            int sensors_index = 0;
            cJSON *json_sensor = NULL;

            cJSON_ArrayForEach(json_sensor, json_sensors) {
                if (parse_sensor(json_sensor, &(config->sensors[sensors_index])))
                    goto parsing_error;

                sensors_index++;
            }

            cJSON_Delete(config_json);
            print_parsed_config(config);

            return config;
        }else{
            cJSON_Delete(config_json);
            goto parsing_error;
        }
    } else{
        cJSON_Delete(config_json);
    }

    parsing_error:
    if(config != NULL){
        if(config->sensors != NULL) free(config->sensors);
        free(config);
    }
    ESP_LOGE(TAG, "ERROR PARSING!");
    return NULL; //TODO
}

int parse_sensor(cJSON *json_sensor, EvaDtsSensor *sensor) {
    cJSON *id = cJSON_GetObjectItem(json_sensor, EVADTS_SENSORS_ID_KEY);
    cJSON *map = cJSON_GetObjectItem(json_sensor, EVADTS_SENSORS_MAP_KEY);
    cJSON *type = cJSON_GetObjectItem(json_sensor, EVADTS_SENSORS_TYPE_KEY);

    if (id != NULL) {
        sprintf(sensor->id, "%s", id->valuestring);
    } else {
        goto parse_sensor_error;
    }

    if (map != NULL) {
        sprintf(sensor->map, "%s", map->valuestring);
    } else {
        sprintf(sensor->map, "%s", id->valuestring);
    }

    if (type != NULL) {
        char *tmp_type = type->valuestring;
        if (strstr(tmp_type, "TYPE_U8") != NULL) {
            sensor->valueType = VALUE_TYPE_U8;
        } else if (strstr(tmp_type, "TYPE_U16") != NULL) {
            sensor->valueType = VALUE_TYPE_U16;
        } else if (strstr(tmp_type, "TYPE_U32") != NULL) {
            sensor->valueType = VALUE_TYPE_U32;
        } else if (strstr(tmp_type, "TYPE_FLOAT") != NULL) {
            sensor->valueType = VALUE_TYPE_FLOAT;
        } else if (strstr(tmp_type, "TYPE_ASCII") != NULL) {
            sensor->valueType = VALUE_TYPE_ASCII;
        } else {
            goto parse_sensor_error;
        }
    } else {
        goto parse_sensor_error;
    }

    return 0;

    parse_sensor_error:
    ESP_LOGE(TAG, "ERROR PARSING SENSOR\n%s", cJSON_Print(json_sensor));
    return 1;
}

int parse_transmission(const cJSON *json_transmission, EvadtsConfig *config){
    cJSON *hours_json = cJSON_GetObjectItem(json_transmission, EVADTS_TRANSMISSION_H_KEY);
    cJSON *minutes_json = cJSON_GetObjectItem(json_transmission, EVADTS_TRANSMISSION_M_KEY);
    cJSON *seconds_json = cJSON_GetObjectItem(json_transmission, EVADTS_TRANSMISSION_S_KEY);

    int hours = hours_json != NULL ? hours_json->valueint : 23;
    int minutes = minutes_json != NULL ? minutes_json->valueint : 50;
    int seconds = seconds_json != NULL ? seconds_json->valueint : 0;

    config->transmissionHour = hours * 3600L + minutes * 60L  + seconds;

    return 0;
}

void print_parsed_config(EvadtsConfig *cfg) {
    ESP_LOGD(TAG, "********************************************");
    ESP_LOGD(TAG, "*          EVADTS SENSOR CONFIG            *");
    ESP_LOGD(TAG, "********************************************");
    for (int i = 0; i < cfg->sensorsSize; i++) {
        ESP_LOGD(TAG, "* NAME: %s", cfg->sensors[i].id);
        ESP_LOGD(TAG, "* MAP: %s", cfg->sensors[i].map);

        switch (cfg->sensors[i].valueType) {
            case VALUE_TYPE_U8:
                ESP_LOGD(TAG, "* TYPE: TYPE_U8");
                break;
            case VALUE_TYPE_U16:
                ESP_LOGD(TAG, "* TYPE: TYPE_U16");
                break;
            case VALUE_TYPE_U32:
                ESP_LOGD(TAG, "* TYPE: TYPE_U32");
                break;
            case VALUE_TYPE_FLOAT:
                ESP_LOGD(TAG, "* TYPE: TYPE_FLOAT");
                break;
            case VALUE_TYPE_ASCII:
                ESP_LOGD(TAG, "* TYPE: TYPE_ASCII");
                break;
            default:
                break;
        }
    }
}

void evadtsConfig_destroy(EvadtsConfig *evadtsConfig) {
    if (evadtsConfig != NULL) {
        if (evadtsConfig->sensors != NULL) {
            for (int i = 0; i < evadtsConfig->sensorsSize; i++) {
                if (evadtsConfig->sensors[i].valueType == VALUE_TYPE_ASCII &&
                    evadtsConfig->sensors[i].value.sValue != NULL) {
                    free(evadtsConfig->sensors[i].value.sValue);
                }
            }
            free(evadtsConfig->sensors);
        }
        free(evadtsConfig);
    }
}
