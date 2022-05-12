#include <esp_log.h>
#include <esp_system.h>
#include "edgehog_micro.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_timer.h>
#include <nvs_flash.h>
#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"

#include <driver/uart.h>
#include <driver/gpio.h>

// Use Pin 18 and 19

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "astarte_init.h"
// #include "perlin.h"

// #include "mbedtls/base64.h"

#define TAG "POC"

#define NVS_PARTITION "nvs"

#define LOOP_DELAY 1000    // ms
#define SEND_DELAY 100     // ms
#define SEND_INTERVAL 3000 // ms

#define DEBUG 1 // If 1 print debug info

#define ALPHA 0.15

#define MAX_VELOCITY 25.0
#define MAX_POLLUTION 100.0

#define WARNING_FLOW 0.7
#define DANGER_FLOW 0.5

#define WARNING_POLLUTION 20.0
#define DANGER_POLLUTION 35.0

#define UART_PORT_NUM (UART_NUM_2)
#define UART_BAUD_RATE 9600
#define UART_TXD (GPIO_NUM_18) // 18
#define UART_RXD (GPIO_NUM_19) // 19

#define BUF_SIZE 1024

#define DEFAULT_NEW_POLLUTION 0.0
#define DEFAULT_NEW_FLOW 0.0

void initialize_sntp(void)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_setservername(1, "europe.pool.ntp.org");
    sntp_setservername(2, "uk.pool.ntp.org ");
    sntp_setservername(3, "us.pool.ntp.org");
    sntp_setservername(4, "time1.google.com");
    sntp_init();
}

double flow2vel(double flow)
{
    return flow * MAX_VELOCITY;
}

/**
 * @brief FreeRTOS task started at boot. It handles the initialization of the required resources and loops waiting for maessages.
 *
 */
static void on_start_app(void)
{
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
    
#if 0
    // INIT ASTARTE -----------------------------------------------------------
    esp_log_level_set("ASTARTE_PAIRING", ESP_LOG_VERBOSE);

    astarte_credentials_use_nvs_storage(NVS_PARTITION);

    astarte_credentials_init();

    // Get MAC addr for hwid

    // uint8_t mac_addr[6] = {0, 0, 0, 0, 0, 0};
    // esp_efuse_mac_get_default(&mac_addr);
    // char *mac_str = mac_addr2str(&mac_addr);
    // ESP_LOGI(TAG, "[APP] MAC addr: %s", mac_str);

    // unsigned char mac_base64[64];
    // size_t outlen;

    // // #include "mbedtls/base64.h"
    // mbedtls_base64_encode(&mac_base64, 64, &outlen, (unsigned char *)mac_str, strlen(mac_str));

    // ESP_LOGI(TAG, "[APP] ASTARTE DeviceID: %s", (char *)&mac_base64);

    astarte_device_config_t cfg = {
        .data_event_callback = astarte_data_events_handler,
        .connection_event_callback = astarte_connection_events_handler,
        .disconnection_event_callback = astarte_disconnection_events_handler,
        .hwid = "ypFF4lmVTJmtGjkd_Xwp2g"}; //(char *)&mac_base64}; //TODO: use MAC addr as hwid

    // Credentials secret: lrRlbeUVMizCqKzeRGhRzU2Uux2s4gmocPqyS69Gifw=

    device = astarte_device_init(&cfg);
    if (!device)
    {
        ESP_LOGE(TAG, "Failed to init astarte device");
        return;
    }

    astarte_device_add_interface(device, &AirData);
    astarte_device_add_interface(device, &EventsHistory);

    if (astarte_device_start(device) != ASTARTE_OK)
    {
        ESP_LOGE(TAG, "Failed to start astarte device");
        return;
    }

    // END INIT ASTARTE -------------------------------------------------------

    ESP_LOGI(TAG, "[APP] Encoded device ID: %s", astarte_device_get_encoded_id(device));
#endif
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());

    ESP_LOGI(TAG, "[ASTARTE] Waiting for ASTARTE.");
    vTaskDelay(pdMS_TO_TICKS(1000));

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
#if 0
            send_flow(flow);
            send_pollution(air_pollution);
            send_velocity(velocity);
#endif
            vTaskDelay(pdMS_TO_TICKS(SEND_DELAY));
        }

        if (flow < DANGER_FLOW)
        {
            if (!flow_danger)
            {
                // Send danger alert
                flow_danger = true;
                printf("Flow Danger!\n");
#if 0
                send_event(FLOW_EVENT, flow, DANGER_NOTE);
#endif 
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
#if 0
                    send_event(FLOW_EVENT, flow, WARNING_NOTE);
#endif
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
#if 0
                send_event(POLLUTION_EVENT, air_pollution, DANGER_NOTE);
#endif
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
#if 0
                    send_event(POLLUTION_EVENT, air_pollution, WARNING_NOTE);
#endif
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
}

/**
 * @brief Main function.
 *
 */
void app_main()
{
    // Start SNTP protocol for time
    initialize_sntp();

    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    int intr_alloc_flags = 0;

    ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TXD, UART_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Start task
    // init_edgehog(on_start_app, NULL);
    on_start_app ();
}
