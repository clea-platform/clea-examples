
/**
 * @file main.c
 * @author Luca Di Mauro (luca.dimauro@seco.com)
 * @brief Entry point for Easy Edge project
 * @version 0.1
 * @date 2022-05-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>

#include <stdio.h>
#include <string.h>

#include <astarte_handler.h>

#include <driver/uart.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <esp_event_base.h>
#include <esp_wifi.h>
#include <time.h>
#include <sys/time.h>
#include <esp_sntp.h>


#define TASK_STACK_DEPTH 4096

// Storage
#define STORAGE_NAMESPACE       "nvs"

// Wifi
uint32_t wifi_retry_count   = 0;
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAILED_BIT     BIT1

// Astarte
ESP_EVENT_DEFINE_BASE(ASTARTE_HANDLER_EVENTS);

//TODO Start filer
#define LOOP_DELAY 1000    // ms
#define SEND_DELAY 100     // ms
#define SEND_INTERVAL 3000 // ms

#define DEBUG 1 // If 1 print debug info

#define ALPHA 0.15

#define MAX_VELOCITY 25.0
#define MAX_POLLUTION 100.0

#define FLOW_EVENT 111
#define POLLUTION_EVENT 222

#define WARNING_FLOW 0.7
#define DANGER_FLOW 0.5

#define WARNING_POLLUTION 20.0
#define DANGER_POLLUTION 35.0

#define WARNING_NOTE 1
#define DANGER_NOTE 2

#define UART_PORT_NUM (UART_NUM_2)
#define UART_BAUD_RATE 9600
#define UART_TXD (GPIO_NUM_18) // 18
#define UART_RXD (GPIO_NUM_19) // 19

#define BUF_SIZE 1024

#define DEFAULT_NEW_POLLUTION 0.0
#define DEFAULT_NEW_FLOW 0.0
// TODO End filter




static void event_handler (void* handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    const char* TAG = "event_handler";
    ESP_LOGV (TAG, "Catched event  '%s'  with id  %d", event_base, event_id);
    

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
    
esp_err_t nvs_initializer () {
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

esp_err_t wifi_initilizer () {
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

esp_err_t sntp_initializer () {
    sntp_setoperatingmode (SNTP_OPMODE_POLL);
    sntp_setservername (0, "pool.ntp.org");
    sntp_setservername (1, "europe.pool.ntp.org");
    sntp_setservername (2, "uk.pool.ntp.org ");
    sntp_setservername (3, "us.pool.ntp.org");
    sntp_setservername (4, "time1.google.com");
    sntp_init ();

    return ESP_OK;
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

// ##################################################

esp_err_t arduino_initializer () {
    esp_err_t success   = ESP_OK;

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate  = UART_BAUD_RATE,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };
    int intr_alloc_flags = 0;

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    return success;
}




/*  ==============================
            Utils fuctions
    ==============================  */
    
double flow2vel(double flow) {
    return flow * MAX_VELOCITY;
}

// ##################################################

int64_t get_millis() {
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
}

// ##################################################

void main_task (void *arg) {
    const char *TAG                     = "main_task";
    astarte_handler_t *astarte_handle   = (astarte_handler_t*) arg;

    // END INIT ASTARTE -------------------------------------------------------

    ESP_LOGI(TAG, "[APP] Encoded device ID: %s", astarte_device_get_encoded_id(astarte_handle->astarte_device_handle));
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());



    // INIT VARIABLES ---------------------------------------------------------
    double flow = 1.0;
    double air_pollution = 0.0;
    double velocity = 0.0;

    long long now = get_millis();
    long long last_sent = now;

    // double pos_flow = 0.0;      // random starting point
    // double pos_quality = 123.0; // random starting point
    // double speed = 0.005;

    bool flow_warning = false;
    bool flow_danger = false;

    bool pollution_warning = false;
    bool pollution_danger = false;

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
    int len = 0;
    int sent_bytes = 0;

    char *flow_key = "flow: ";
    char *pollution_key = "pollution: ";
    // END INIT VARIABLES -----------------------------------------------------

    // MAIN LOOP
    while (1)
    {
        // ------ GET FLUX ------------
        char *f_message = "f\r\n";
        // Write data to the UART
        sent_bytes = uart_write_bytes(UART_PORT_NUM, (const char *)f_message, strlen(f_message));
        ESP_LOGI(TAG, "Sent %d bytes for str: %s", sent_bytes, f_message);

        long long recv_start = get_millis();
        while (get_millis() < recv_start + 1000 && len == 0)
        {
            // Read data from the UART
            len = uart_read_bytes(UART_PORT_NUM, data, (BUF_SIZE - 1), 50 / portTICK_PERIOD_MS);
        }

        double new_flow;

        if (len > 0)
        {
            data[len] = '\0';
            ESP_LOGI(TAG, "Recv str: %s", (const char *)data);

            char *fres = strstr((const char *)data, flow_key);

            if (fres != NULL)
            {
                // Get new Flux value
                new_flow = atof(fres + strlen(flow_key));
            }
            else
            {
                new_flow = DEFAULT_NEW_FLOW;
            }
        }
        else
        {
            new_flow = DEFAULT_NEW_FLOW;
        }

        // Get new Flux value
        // double new_flow = 1.0;

        // ----------------------------

        // ------ GET Quality ---------
        char *q_message = "q\r\n";
        // Write data to the UART
        sent_bytes = uart_write_bytes(UART_PORT_NUM, (const char *)q_message, strlen(q_message));
        ESP_LOGI(TAG, "Sent %d bytes for str: %s", sent_bytes, q_message);

        recv_start = get_millis();
        len = 0;
        while (get_millis() < recv_start + 1000 && len == 0)
        {
            // Read data from the UART
            len = uart_read_bytes(UART_PORT_NUM, data, (BUF_SIZE - 1), 50 / portTICK_PERIOD_MS);
        }

        double new_pollution;

        if (len > 0)
        {
            data[len] = '\0';
            ESP_LOGI(TAG, "Recv str: %s", (const char *)data);

            char *pres = strstr((const char *)data, pollution_key);

            if (pres != NULL)
            {
                // Get new Quality value
                new_pollution = atof(pres + strlen(pollution_key));
            }
            else
            {
                new_pollution = DEFAULT_NEW_POLLUTION;
            }
        }
        else
        {
            new_pollution = DEFAULT_NEW_POLLUTION;
        }

        // Get new Quality value
        // double new_pollution = DEFAULT_NEW_POLLUTION;

        // ----------------------------

        // if (!len)
        // {
        //     continue;
        // }

        // double new_flow = to_range(perlin(pos_flow));
        // double new_pollution = to_range(perlin(pos_quality));

        // Maybe exponential average to use a mean of the last n readings
        flow = ALPHA * flow + new_flow * (1 - ALPHA);                        // Get value
        air_pollution = ALPHA * air_pollution + new_pollution * (1 - ALPHA); // Get value // * MAX_POLLUTION

        velocity = flow2vel(flow);

        if (DEBUG)
        {
            printf("Air Flow: %f\n", flow);
            printf("Air Pollution: %f\n", air_pollution);
            printf("Air Velocity: %f\n", velocity);
        }

        now = get_millis(); // get time now in milliseconds

        // SEND data according to their specified intervals
        if (now - last_sent > SEND_INTERVAL)
        {
            last_sent = now;
            ESP_LOGI(TAG, "Sending data: F=%f, P=%f, V=%f", flow, air_pollution, velocity);
            astarte_handle->publish_flow (astarte_handle, flow);
            astarte_handle->publish_pollution (astarte_handle, air_pollution);
            astarte_handle->publish_velocity (astarte_handle, velocity);

            vTaskDelay(pdMS_TO_TICKS(SEND_DELAY));
        }

        if (flow < DANGER_FLOW)
        {
            if (!flow_danger)
            {
                // Send danger alert
                flow_danger = true;
                printf("Flow Danger!\n");
                astarte_handle->publish_event (astarte_handle, FLOW_EVENT, flow, DANGER_NOTE);
                vTaskDelay(pdMS_TO_TICKS(SEND_DELAY));
            }
        }
        else
        {
            if (flow < WARNING_FLOW)
            {
                if (!flow_warning && !flow_danger)
                {
                    // Send warning alert
                    flow_warning = true;
                    printf("Flow Warning!\n");
                    astarte_handle->publish_event (astarte_handle, FLOW_EVENT, flow, WARNING_NOTE);
                    vTaskDelay(pdMS_TO_TICKS(SEND_DELAY));
                }
            }
            else
            {
                // No Danger
                flow_warning = false;
                flow_danger = false;
            }
        }

        if (air_pollution > DANGER_POLLUTION)
        {
            if (!pollution_danger)
            {
                // Send danger alert
                pollution_danger = true;
                printf("Pollution Danger!\n");
                astarte_handle->publish_event (astarte_handle, POLLUTION_EVENT, air_pollution, DANGER_NOTE);
                vTaskDelay(pdMS_TO_TICKS(SEND_DELAY));
            }
        }
        else
        {
            if (air_pollution > WARNING_POLLUTION)
            {
                if (!pollution_warning && !pollution_danger)
                {
                    // Send warning alert
                    pollution_warning = true;
                    printf("Pollution Warning!\n");
                    astarte_handle->publish_event (astarte_handle, POLLUTION_EVENT, air_pollution, WARNING_NOTE);
                    vTaskDelay(pdMS_TO_TICKS(SEND_DELAY));
                }
            }
            else
            {
                // No Danger
                pollution_danger = false;
                pollution_warning = false;
            }
        }

        // pos_flow = pos_flow + speed;
        // pos_quality = pos_quality + speed / 2;

        vTaskDelay(pdMS_TO_TICKS(LOOP_DELAY));
    }

    vTaskDelete(NULL);

    vTaskDelay (pdMS_TO_TICKS(1000));
}




/*  ====================
            Main        
    ====================  */

void app_main(void) {
    const char *TAG                     = "app_main";
    astarte_handler_t *astarte_handler  = NULL;
    vTaskDelay(100);

    printf ("\n\n\n\n");

    ESP_ERROR_CHECK (nvs_initializer());
    ESP_LOGI (TAG, "NVS initialized");

    ESP_ERROR_CHECK (wifi_initilizer());
    ESP_LOGI (TAG, "WiFi initialized");

    ESP_ERROR_CHECK (sntp_initializer());
    ESP_LOGI (TAG, "SNTP initialized");

    ESP_ERROR_CHECK (astarte_initializer(&astarte_handler));
    ESP_LOGI (TAG, "Astarte initlized");
    
    ESP_ERROR_CHECK (arduino_initializer());
    ESP_LOGI (TAG, "Arduino initlized");
    
    printf ("\n\n\n\n");
    
    // Executing main task
    main_task (astarte_handler);
}
