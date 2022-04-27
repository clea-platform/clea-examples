
/**
 * @file main.c
 * @author Luca Di Mauro (luca.dimauro@seco.com)
 * @brief Entry point for Easy Edge project
 * @version 0.1
 * @date 2022-04-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <stdio.h>

// #include <eva-dts-engine.h>
// #include <astarte-handler.h>

#include <astarte_handler.h>

#include <driver/gpio.h>
#include <esp_log.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <esp_event_base.h>
#include <esp_wifi.h>


#define TASK_STACK_DEPTH 4096

// Storage
#define STORAGE_NAMESPACE   "nvs"

// Wifi
uint32_t wifi_retry_count   = 0;
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAILED_BIT     BIT1

ESP_EVENT_DEFINE_BASE(ASTARTE_HANDLER_EVENTS);


static void event_handler (void* handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    const char* TAG = "event_handler";
    ESP_LOGD (TAG, "Catched event  '%s'  with id  %d", event_base, event_id);
    

    // ================================     WIFI events     ================================
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            if (wifi_retry_count < CONFIG_WIFI_MAXIMUM_RETRY) {
                ++wifi_retry_count;
                ESP_LOGW (TAG, "Retrying to connect to the AP (#%d)", wifi_retry_count);
                esp_wifi_connect();
            } else {
                xEventGroupSetBits((EventGroupHandle_t) handler_arg, WIFI_FAILED_BIT);
                ESP_LOGE (TAG,"connect to the AP fail");
            }
        }
    }
    // ================================     IP events     ================================
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event    = (ip_event_got_ip_t*) event_data;
        wifi_retry_count            = 0;
        //ESP_LOGD (TAG, "Got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits ((EventGroupHandle_t) handler_arg, WIFI_CONNECTED_BIT);
    }
    // ================================     ASTARTE HANDLER events     ================================
    else if (event_base == ASTARTE_HANDLER_EVENTS) {
        if (event_id == ASTARTE_HANDLER_EVENT_CONNECT) {
            xEventGroupSetBits((EventGroupHandle_t) handler_arg, ASTARTE_HANDLER_INITIALIZED_BIT);
        }
        else if (event_id == ASTARTE_HANDLER_EVENT_DISCONNECT) {
            xEventGroupSetBits((EventGroupHandle_t) handler_arg, ASTARTE_HANDLER_FAILED_BIT);
        }
    }
    else {
        ESP_LOGE (TAG, "Unknown triggered event.\n\tEvent base:  %s\n\tEvent id:    %d", event_base, event_id);
    }
}




/*  =============================
            Init fuctions
    =============================  */
    
esp_err_t init_nvs () {
    const char* TAG = "init_nvs";
    esp_err_t res   = nvs_flash_init ();
    if (res == ESP_ERR_NVS_NO_FREE_PAGES || res == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI (TAG, "Erasing flash..");
        ESP_ERROR_CHECK (nvs_flash_erase());
        res = nvs_flash_init ();
    }
    return res;
}

// ##################################################

esp_err_t init_wifi_connection () {
    const char* TAG                     = "init_wifi_connection";
    esp_err_t res                       = ESP_OK;
    EventGroupHandle_t wifi_event_group = xEventGroupCreate();

    xEventGroupClearBits (wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAILED_BIT);

    ESP_ERROR_CHECK (esp_netif_init());
    ESP_ERROR_CHECK (esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta ();

    wifi_init_config_t cfg  = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK (esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK (esp_event_handler_instance_register (
        WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, (void*) wifi_event_group, &instance_any_id));

    ESP_ERROR_CHECK (esp_event_handler_instance_register (
        IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, (void*) wifi_event_group, &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid               = CONFIG_WIFI_SSID,
            .password           = CONFIG_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg    = {
                .capable    = true,
                .required   = false
            },
        },
    };

    ESP_ERROR_CHECK (esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK (esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK (esp_wifi_start());

    
    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed
     * for the maximum number of re-tries (WIFI_FAIL_BIT) */
    EventBits_t bits    = xEventGroupWaitBits (wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAILED_BIT,
                                                pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        // Do nothing
        //ESP_LOGD (TAG, "connected to ap SSID:%s", CONFIG_WIFI_SSID);
    } else if (bits & WIFI_FAILED_BIT) {
        ESP_LOGE (TAG, "Failed to connect to SSID:%s", CONFIG_WIFI_SSID);
    } else {
        ESP_LOGE (TAG, "UNEXPECTED EVENT");
        res = ESP_FAIL;
    }

    ESP_ERROR_CHECK(
        esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    
    vEventGroupDelete(wifi_event_group);

    return res;
}

// ##################################################

esp_err_t astarte_initializer (astarte_handler_t **target) {
    const char *TAG                         = "astarte_initializer";
    esp_err_t result                        = ESP_OK;
    EventGroupHandle_t astarte_event_group  = xEventGroupCreate ();
    *target                                 = astarte_handler_create();

    if (!(*target)) {
        ESP_LOGE (TAG, "Cannot create astarte_handler_t");
        return ESP_FAIL;
    }

    xEventGroupClearBits (astarte_event_group, ASTARTE_HANDLER_INITIALIZED_BIT | ASTARTE_HANDLER_FAILED_BIT);
    ESP_ERROR_CHECK (esp_event_handler_instance_register (ASTARTE_HANDLER_EVENTS, ESP_EVENT_ANY_ID,
                                                            &event_handler, astarte_event_group, NULL));

    while (!(*target)->start(*target)) {
        vTaskDelay(pdTICKS_TO_MS (250));

    }

    EventBits_t bits    = xEventGroupWaitBits (astarte_event_group,
                                                ASTARTE_HANDLER_INITIALIZED_BIT | ASTARTE_HANDLER_FAILED_BIT,
                                                pdFALSE, pdFALSE, portMAX_DELAY);
    if (bits & ASTARTE_HANDLER_INITIALIZED_BIT) {
        // Do nothing
    } else if (bits & ASTARTE_HANDLER_FAILED_BIT) {
        ESP_LOGE (TAG, "Astarte NOT corectly initialized");
        result  = ESP_FAIL;
    } else {
        ESP_LOGE (TAG, "UNEXPECTED EVENT");
        result  = ESP_FAIL;
    }

    return result;
}


void app_main(void) {
    const char *TAG                     = "app_main";
    astarte_handler_t *astarte_handler  = NULL;
    vTaskDelay(1);

    ESP_ERROR_CHECK (init_nvs());
    ESP_LOGI (TAG, "NVS initialized");

    ESP_ERROR_CHECK (init_wifi_connection());
    ESP_LOGI (TAG, "WiFi initialized");

    ESP_ERROR_CHECK (astarte_initializer(&astarte_handler));
    ESP_LOGI (TAG, "Astarte initlized");
}
