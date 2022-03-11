
#include "coffee_machine_sensor.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include <astarteHandler.h>
#include <freertos/event_groups.h>
#include <nvs_flash.h>
#include <stdio.h>

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

const static char *TAG = "coffee_machine_demo";
static EventGroupHandle_t s_wifi_event_group;

static int s_retry_num = 0;


static void event_handler (void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 10) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}


static void coffee_machine_event_handler (void *arg, esp_event_base_t event_base,
                                            int32_t event_id, void *event_data) {
    ESP_LOGI(TAG, "COFFEE_MACHINE SENSOR EVENT: %d %s", event_id, get_event_name(event_id));
    AstarteHandler *astarte_handler = (AstarteHandler *) arg;

    if (astarte_handler) {
        astarte_handler->publish_event(astarte_handler, (coffee_machine_event) event_id);
    }
}


void wifi_init (void) {
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config = {
            .sta = {
                    .ssid = CONFIG_WIFI_SSID,
                    .password = CONFIG_WIFI_PASSWORD,
                    .threshold.authmode = WIFI_AUTH_WPA2_PSK,

                    .pmf_cfg = {
                            .capable = true,
                            .required = false
                    },
            },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed
     * for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which
     * event actually happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s", CONFIG_WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s", CONFIG_WIFI_SSID);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(
        esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}


static void astarte_event_handler (void *arg, esp_event_base_t event_base,
                                    int32_t event_id, void *event_data) {
    ESP_LOGI(TAG, "Astarte EVENT: %d", event_id);
    switch (event_id) {
        case ASTARTE_EVENT_CONNECT: {
            ESP_LOGI(TAG, " ASTARTE CONNECTED");
        } break;
    }
}


AstarteHandler *astarte_init (void) {
    AstarteHandler *astarteHandler = astarteHandler_init();

    while (!astarteHandler->start(astarteHandler)) {
        vTaskDelay(60 * 100 / portTICK_PERIOD_MS);
    }

    esp_event_handler_instance_register(
        ASTARTE_EVENTS, ESP_EVENT_ANY_ID, &astarte_event_handler, astarteHandler, NULL);

    esp_event_handler_instance_register(COFFEE_MACHINE_SENSOR_EVENTS, ESP_EVENT_ANY_ID,
        &coffee_machine_event_handler, astarteHandler, NULL);

    return astarteHandler;
}


void app_main (void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    wifi_init();
    AstarteHandler *astarte_handler = astarte_init();

    init_machine_coffee_sensor();
}