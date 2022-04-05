#include "coffee_machine_sensor.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <driver/adc.h>
#include <string.h>
#include <esp_system.h>


ESP_EVENT_DEFINE_BASE(COFFEE_MACHINE_SENSOR_EVENTS);


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




// ==================================
//      EMULATED COFFEE MACHINE
// ==================================
#if CONFIG_COFFEE_MACHINE_SENSOR == 0




const static char* TAG                      = "Emulated coffee machine";
static xQueueHandle gpio_buttons_evt_queue  = NULL;
int64_t last_short_coffee_isr_us            = 0;
int64_t last_long_coffee_isr_us             = 0;

typedef struct {
    uint32_t gpio_num;
    int64_t trigger_time_us;
} QueueItem;




static void IRAM_ATTR gpio_button_isr_handler (void* arg) {
    QueueItem item;
    memset (&item, '\0', sizeof(item));
    item.gpio_num           = (uint32_t) arg;
    item.trigger_time_us    = esp_timer_get_time ();
    xQueueSendFromISR (gpio_buttons_evt_queue, &item, NULL);
}


void queue_consumer_task (void* arg) {
    QueueItem item;
    memset (&item, '\0', sizeof(item));
    
    while (1) {
        if (xQueueReceive(gpio_buttons_evt_queue, &item, portMAX_DELAY)) {
            switch (item.gpio_num) {
            case GPIO_SINGLE_SHORT_COFFEE_BUTTON:
                if (item.trigger_time_us - last_short_coffee_isr_us > MIN_DELAY_ISR_TRIGGER_US) {
                    last_short_coffee_isr_us    = item.trigger_time_us;
                    esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, COFFEE_SHORT_EVENT, NULL, 0, 0);
                    //ESP_LOGI (TAG, "Short coffee delivered!");
                }
                else {
                    // Ignoring trigger
                    ESP_LOGW (TAG, "Ignoring trigger on gpio #%d", item.gpio_num);
                }
                break;
            case GPIO_SINGLE_LONG_COFFEE_BUTTON:
                if (item.trigger_time_us - last_long_coffee_isr_us > MIN_DELAY_ISR_TRIGGER_US) {
                    last_long_coffee_isr_us     = item.trigger_time_us;
                    esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, COFFEE_LONG_EVENT, NULL, 0, 0);
                    //ESP_LOGI (TAG, "Long coffee delivered!");
                }
                else {
                    // Ignoring trigger
                    ESP_LOGW (TAG, "Ignoring trigger on gpio #%d", item.gpio_num);
                }
                break;

            default:
                break;
            }
       }
       
       vTaskDelay(100 / portTICK_PERIOD_MS);
   }
}


void containers_buttons_monitor (void* arg) {
    // Setting up containers GPIOs
    gpio_pad_select_gpio (GPIO_WATER_CONTAINER_BUTTON);
    uint64_t water_gpio_up_start_us = 0;
    uint8_t water_alarm_triggered   = 0;
    int64_t curr_time_us            = esp_timer_get_time ();

    while (1) {
        curr_time_us    = esp_timer_get_time ();

        if (gpio_get_level(GPIO_WATER_CONTAINER_BUTTON)) {
            // Triggering event only if it remains up for at least 
            if (curr_time_us - water_gpio_up_start_us > MIN_DELAY_WATER_CONTAINER_US && !water_alarm_triggered) {
                esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, WATER_OPEN_ALARM_EVENT, NULL, 0, 0);
                ESP_LOGE (TAG, "Water container is opened!");
                water_alarm_triggered   = 1;
            }
        }
        else if (curr_time_us - water_gpio_up_start_us > MIN_DELAY_WATER_CONTAINER_US && water_alarm_triggered) {
            esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, WATER_OFF_ALARM_EVENT, NULL, 0, 0);
            ESP_LOGI (TAG, "Water container closed");
            water_gpio_up_start_us  = curr_time_us;
            water_alarm_triggered   = 0;
        }
        else {
            water_gpio_up_start_us  = curr_time_us;
        }

        vTaskDelay (pdMS_TO_TICKS(100));
    }
}


void led_blinker (void* _gpio_num) {
    while (1) {
        gpio_set_level (GPIO_YELLOW_LED, 0);

        gpio_set_level (GPIO_BLUE_LED, 1);
        vTaskDelay (pdMS_TO_TICKS(128));
        gpio_set_level (GPIO_BLUE_LED, 0);
        vTaskDelay (pdMS_TO_TICKS(128));
        gpio_set_level (GPIO_BLUE_LED, 1);
        vTaskDelay (pdMS_TO_TICKS(128));
        gpio_set_level (GPIO_BLUE_LED, 0);

        gpio_set_level (GPIO_YELLOW_LED, 1);
        vTaskDelay (pdMS_TO_TICKS(4096));
    }
}


void init_machine_coffee_sensor () {
    ESP_LOGI (TAG, "Initializing coffee machine sensor");
    
    gpio_config_t gpio_conf;


    // Enabling output power on GPIO
    gpio_conf.intr_type     = GPIO_PIN_INTR_DISABLE;
    gpio_conf.mode          = GPIO_MODE_OUTPUT;
    gpio_conf.pin_bit_mask  = (1ULL << GPIO_POWER_OUT_ENABLE);
    gpio_conf.pull_down_en  = GPIO_PULLDOWN_DISABLE;
    gpio_conf.pull_up_en    = GPIO_PULLUP_DISABLE;
    gpio_config(&gpio_conf);
    gpio_set_level(GPIO_POWER_OUT_ENABLE, 0);


    // Make blinking LEDs
    memset (&gpio_conf, '\0', sizeof(gpio_conf));
    gpio_conf.pin_bit_mask  = (1ULL << GPIO_YELLOW_LED) | (1ULL << GPIO_BLUE_LED);
    gpio_conf.mode          = GPIO_MODE_OUTPUT;
    gpio_conf.pull_up_en    = GPIO_PULLUP_DISABLE;
    gpio_conf.pull_down_en  = GPIO_PULLDOWN_DISABLE;
    gpio_conf.intr_type     = GPIO_PIN_INTR_DISABLE;
    gpio_config (&gpio_conf);
    xTaskCreate (led_blinker, "led_blinker_task", 2048, NULL, 10, NULL);


    // Creating queue for buttons events
    gpio_buttons_evt_queue  = xQueueCreate(1, sizeof(QueueItem));
    xTaskCreate (queue_consumer_task, "queue_consumer_task", 2048, NULL, 10, NULL);
    // Setting up buttons GPIOs
    memset (&gpio_conf, '\0', sizeof(gpio_conf));
    gpio_conf.pin_bit_mask  = (1ULL << GPIO_SINGLE_SHORT_COFFEE_BUTTON) | (1ULL << GPIO_SINGLE_LONG_COFFEE_BUTTON);
    gpio_conf.mode          = GPIO_MODE_INPUT;
    gpio_conf.pull_up_en    = GPIO_PULLUP_DISABLE;
    gpio_conf.pull_down_en  = GPIO_PULLDOWN_DISABLE;
    gpio_conf.intr_type     = GPIO_INTR_NEGEDGE;
    gpio_config(&gpio_conf);
    // Registering ISRs for buttons events
    last_short_coffee_isr_us    = esp_timer_get_time ();
    last_long_coffee_isr_us     = esp_timer_get_time ();
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_SINGLE_SHORT_COFFEE_BUTTON, gpio_button_isr_handler, (void*) GPIO_SINGLE_SHORT_COFFEE_BUTTON);
    gpio_isr_handler_add(GPIO_SINGLE_LONG_COFFEE_BUTTON, gpio_button_isr_handler, (void*) GPIO_SINGLE_LONG_COFFEE_BUTTON);


    // Creating monitor task for containers buttons
    xTaskCreate (containers_buttons_monitor, "containers_buttons_monitor", 2048, NULL, 10, NULL);

    esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, WATER_OFF_ALARM_EVENT, NULL, 0, 0);
    esp_event_post(COFFEE_MACHINE_SENSOR_EVENTS, CONTAINER_OFF_ALARM_EVENT, NULL, 0, 0);


    ESP_LOGI (TAG, "Waiting for beverages requests..\n\n");
}




// ==================================
//      DELONGHI COFFEE MACHINE
// ==================================
#elif CONFIG_COFFEE_MACHINE_SENSOR == 1




#define GPIO_LED_PIN_SEL    ((1ULL<<GPIO_WATER_LED) | (1ULL<<GPIO_CONTAINER_LED))
#define COFFEE_VALUE_TRIGGER 500

const static char *TAG = "DeLonghi coffee machine";
static xQueueHandle gpio_led_evt_queue = NULL;


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
    ESP_LOGI (TAG, "Setting up this coffee machine sensor: %d", CONFIG_COFFEE_MACHINE_SENSOR);

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




#endif
