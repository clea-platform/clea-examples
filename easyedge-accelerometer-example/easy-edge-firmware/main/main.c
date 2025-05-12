/**
 * Simple example with one sensor connected to I2C.
 */

#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "lis3dh.h"
#include <astarteHandler.h>
#include <freertos/event_groups.h>
#include <nvs_flash.h>
#include <stdio.h>

#define TASK_STACK_DEPTH 4096

// I2C interface definitions
#define I2C_BUS 0
#define I2C_SCL_PIN 19
#define I2C_SDA_PIN 18
#define I2C_FREQ I2C_FREQ_400K

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define LED_GPIO 33

static lis3dh_sensor_t *sensor;
static const char *TAG = "i2c-simple-example";
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
ESP_EVENT_DEFINE_BASE(ASTARTE_EVENTS);

static void event_handler(
    void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 10) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init(void)
{
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

static void led_init()
{
    gpio_pad_select_gpio(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
}

static void astarte_event_handler(
    void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    ESP_LOGI(TAG, "Astarte EVENT: %d", event_id);
    switch (event_id) {
        case ASTARTE_EVENT_CONNECT: {
            ESP_LOGI(TAG, " ASTARTE CONNECTED");
        } break;
        case ASTARTE_EVENT_LED: {
            int *led_value = (int *) event_data;
            if (led_value != NULL && *led_value == 1) {
                ESP_LOGI(TAG, "Turning led on");
                gpio_set_level(LED_GPIO, 1);
            } else {
                ESP_LOGI(TAG, "Turning led off");
                gpio_set_level(LED_GPIO, 0);
            }
        }
    }
}

AstarteHandler *astarte_init()
{
    AstarteHandler *astarteHandler = astarteHandler_init();

    while (!astarteHandler->start(astarteHandler)) {
        vTaskDelay(60 * 100 / portTICK_PERIOD_MS);
    }

    esp_event_handler_instance_register(
        ASTARTE_EVENTS, ESP_EVENT_ANY_ID, &astarte_event_handler, astarteHandler, NULL);

    return astarteHandler;
}

void read_data(AstarteHandler *astarteHandler)
{
    lis3dh_float_data_t data;

    if (lis3dh_new_data(sensor) && lis3dh_get_float_data(sensor, &data)) {
        ESP_LOGI(TAG, "LIS3DH (xyz)[g] ax=%+7.3f ay=%+7.3f az=%+7.3f", data.ax, data.ay, data.az);

        if (astarteHandler) {
            pub_acc_data(astarteHandler->astarteDeviceHandle, data.ax, data.ay, data.az);
        }
    }
}

void user_task_periodic(void *pvParameters)
{
    vTaskDelay(100 / portTICK_PERIOD_MS);

    while (1) {
        read_data((AstarteHandler *) pvParameters);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    AstarteHandler *astarte_handler;
    uart_set_baud(0, 115200);
    vTaskDelay(1);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    wifi_init();
    led_init();
    astarte_handler = astarte_init();

    i2c_init(I2C_BUS, I2C_SCL_PIN, I2C_SDA_PIN, I2C_FREQ);
    sensor = lis3dh_init_sensor(I2C_BUS, LIS3DH_I2C_ADDRESS_2, 0);

    if (sensor) {
        lis3dh_set_scale(sensor, lis3dh_scale_2_g);
        lis3dh_set_mode(sensor, lis3dh_odr_10, lis3dh_high_res, true, true, true);

        xTaskCreate(
            user_task_periodic, "user_task_periodic", TASK_STACK_DEPTH, astarte_handler, 2, NULL);
    } else
        printf("Could not initialize LIS3DH sensor\n");
}
