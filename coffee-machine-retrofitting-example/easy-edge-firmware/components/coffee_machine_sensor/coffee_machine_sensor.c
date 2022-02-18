#include "coffee_machine_sensor.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <driver/adc.h>
#include <string.h>

#define GPIO_LED_PIN_SEL    ((1ULL<<GPIO_WATER_LED) | (1ULL<<GPIO_CONTAINER_LED))
#define COFFEE_VALUE_TRIGGER 500

ESP_EVENT_DEFINE_BASE(COFFEE_MACHINE_SENSOR_EVENTS);
const static char *TAG = "Coffee_Sensor";
static xQueueHandle gpio_led_evt_queue = NULL;


const char *get_event_name(coffee_machine_event event) {
    switch (event) {
        case MACHINE_ON_EVENT:
            return "MACHINE_ON";
        case MACHINE_OFF_EVENT:
            return "MACHINE_OFF";
        case WATER_OFF_ALARM_EVENT:
            return "WATER_OFF_ALARM_EVENT";
        case WATER_EMPTY_ALARM_EVENT:
            return "WATER_EMPTY_ALARM_EVENT";
        case WATER_OPEN_ALARM_EVENT:
            return "WATER_OPEN_ALARM_EVENT";
        case CONTAINER_OFF_ALARM_EVENT:
            return "CONTAINER_OFF_ALARM_EVENT";
        case CONTAINER_OPEN_ALARM_EVENT:
            return "CONTAINER_OPEN_ALARM_EVENT";
        case CONTAINER_FULL_ALARM_EVENT:
            return "CONTAINER_FULL_ALARM_EVENT";
        case COFFEE_SHORT_EVENT:
            return "COFFEE_SHORT_EVENT";
        case COFFEE_LONG_EVENT:
            return "COFFEE_LONG_EVENT";
    }
   
    return "";
}


static void IRAM_ATTR gpio_led_isr_handler (void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_led_evt_queue, &gpio_num, NULL);
}


static void on_off_machine (int gpio_level) {
    if (gpio_level) {
        esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, MACHINE_ON_EVENT, NULL, 0, 0);
    }
    else {
        esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, MACHINE_OFF_EVENT, NULL, 0, 0);
    }
}


enum LED_STATE {
    ON,
    BLINK,
    OFF
};


static enum LED_STATE check_led_state (uint32_t io_num, int start_level) {
    int sum = start_level;
    int samples = 15;
    for(int i= 0; i < samples; i++) {
        sum += gpio_get_level(io_num);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    if(sum >= 12){
        return ON;
    }
    else if (sum > 3){
        return BLINK;
    }
    else{
        return OFF;
    }
}


static void check_water (int gpio_level) {
    switch (check_led_state(GPIO_WATER_LED, gpio_level)) {
        case ON:
            esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, WATER_OPEN_ALARM_EVENT, NULL, 0, 0);
            break;
        case BLINK:
            esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, WATER_EMPTY_ALARM_EVENT, NULL, 0, 0);
            break;
        case OFF:
            esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, WATER_OFF_ALARM_EVENT, NULL, 0, 0);
            break;
    }
}


static void check_container (int gpio_level) {
    switch (check_led_state(GPIO_CONTAINER_LED, gpio_level)) {
        case ON:
            esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, CONTAINER_OPEN_ALARM_EVENT, NULL, 0, 0);
            break;
        case BLINK:
            esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, CONTAINER_FULL_ALARM_EVENT, NULL, 0, 0);
            break;
        case OFF:
            esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, CONTAINER_OFF_ALARM_EVENT, NULL, 0, 0);
            break;
    }
}


static void gpio_led_reader_task (void* arg) {
    uint32_t io_num;

    for(;;) {
        if(xQueueReceive(gpio_led_evt_queue, &io_num, portMAX_DELAY)) {
            int gpio_level = gpio_get_level(io_num);
            ESP_LOGD(TAG, "GPIO[%d] led intr, val: %d", io_num, gpio_level);

            switch (io_num) {
                case GPIO_ON_OFF_LED:
                    on_off_machine(gpio_level);
                    break;
                case GPIO_WATER_LED:
                    check_water(gpio_level);
                    break;
                case GPIO_CONTAINER_LED:
                    check_container(gpio_level);
                    break;
            }

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}


void adc_bnt_short_task()
{
    for (;;) {

        int adc1_6_value = adc1_get_raw(ADC1_CHANNEL_6);
        int adc1_7_value = adc1_get_raw(ADC1_CHANNEL_7);
        ESP_LOGD(TAG, "ADC[%d] val: %d", ADC1_CHANNEL_6, adc1_6_value);
        ESP_LOGD(TAG, "ADC[%d] val: %d", ADC1_CHANNEL_7, adc1_7_value);
        if (adc1_6_value <= COFFEE_VALUE_TRIGGER) {

          esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, COFFEE_SHORT_EVENT, NULL, 0, 0);
          vTaskDelay(2000 / portTICK_PERIOD_MS);

        } else if (adc1_7_value <= COFFEE_VALUE_TRIGGER) {

          //DOUBLE_SHORT
          esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, COFFEE_SHORT_EVENT, NULL, 0, 0);
          vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}


void adc_bnt_long_task()
{
    for (;;) {

        int adc1_4_value = adc1_get_raw(ADC1_CHANNEL_4);
        int adc1_5_value = adc1_get_raw(ADC1_CHANNEL_5);
        ESP_LOGD(TAG, "ADC[%d] val: %d", ADC1_CHANNEL_4, adc1_4_value);
        ESP_LOGD(TAG, "ADC[%d] val: %d", ADC1_CHANNEL_5, adc1_5_value);

        if (adc1_4_value <= COFFEE_VALUE_TRIGGER || adc1_5_value <= COFFEE_VALUE_TRIGGER) {
            esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, COFFEE_LONG_EVENT, NULL, 0, 0);
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}


void init_machine_coffee_sensor(){
    gpio_config_t gpio_led_conf;

    gpio_led_conf.pin_bit_mask = GPIO_LED_PIN_SEL;
    gpio_led_conf.mode         = GPIO_MODE_INPUT;
    gpio_led_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
    gpio_led_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_led_conf.intr_type    = GPIO_INTR_POSEDGE;
    gpio_config(&gpio_led_conf);

    gpio_install_isr_service(0);

    gpio_led_evt_queue = xQueueCreate(1, sizeof(uint32_t));
    gpio_isr_handler_add(GPIO_WATER_LED, gpio_led_isr_handler, (void*) GPIO_WATER_LED);
    gpio_isr_handler_add(GPIO_CONTAINER_LED, gpio_led_isr_handler, (void*) GPIO_CONTAINER_LED);

    xTaskCreate(gpio_led_reader_task, "gpio_led_evt_queue", 2048, NULL, 10, NULL);

    adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_DB_0);
    adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_DB_0);
    adc1_config_channel_atten(ADC1_CHANNEL_3,ADC_ATTEN_DB_0);
    adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_0);

    xTaskCreate(adc_bnt_short_task, "adc_bnt_short_task", 2048, NULL, 10, NULL);
    xTaskCreate(adc_bnt_long_task, "adc_bnt_long_task", 2048, NULL, 10, NULL);

    ESP_LOGI(TAG, " adc_reader_task started");
}

