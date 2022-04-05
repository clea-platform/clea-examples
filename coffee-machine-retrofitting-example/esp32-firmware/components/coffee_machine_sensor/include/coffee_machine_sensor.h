//
// Created by harlem88 on 21/01/20.
//

#ifndef COFFEE_MACHINE_SENSOR_H
#define COFFEE_MACHINE_SENSOR_H
#include <esp_event.h>
#include <driver/gpio.h>


#if CONFIG_COFFEE_MACHINE_SENSOR == 0
    #define MIN_DELAY_ISR_TRIGGER_US        1000000         // 1 seconds delay
    #define MIN_DELAY_WATER_CONTAINER_US    3000000         // 3 seconds delay
    #define GPIO_POWER_OUT_ENABLE           GPIO_NUM_25
    #define GPIO_BLUE_LED                   GPIO_NUM_32
    #define GPIO_YELLOW_LED                 GPIO_NUM_33

    //#define GPIO_TRASH_CONTAINER_BUTTON     GPIO_NUM_21
    #define GPIO_WATER_CONTAINER_BUTTON     GPIO_NUM_5
    #define GPIO_SINGLE_SHORT_COFFEE_BUTTON GPIO_NUM_19
    #define GPIO_SINGLE_LONG_COFFEE_BUTTON  GPIO_NUM_18
#elif CONFIG_COFFEE_MACHINE_SENSOR == 1
    #define GPIO_ON_OFF_LED 5
    #define GPIO_WATER_LED 19
    #define GPIO_CONTAINER_LED 2
#else
    // Bad configuration: throwing an error!
    #error "Invalid CONFIG_COFFEE_MACHINE_SENSOR value: " CONFIG_COFFEE_MACHINE_SENSOR
#endif

ESP_EVENT_DECLARE_BASE(COFFEE_MACHINE_SENSOR_EVENTS);

typedef enum {
    MACHINE_ON_EVENT,
    MACHINE_OFF_EVENT,
    WATER_OFF_ALARM_EVENT,
    WATER_EMPTY_ALARM_EVENT,
    WATER_OPEN_ALARM_EVENT,
    CONTAINER_OFF_ALARM_EVENT,
    CONTAINER_OPEN_ALARM_EVENT,
    CONTAINER_FULL_ALARM_EVENT,
    COFFEE_SHORT_EVENT,
    COFFEE_LONG_EVENT,
} coffee_machine_event;

const char *get_event_name(coffee_machine_event event);

void init_machine_coffee_sensor();


#endif // COFFEE_MACHINE_SENSOR_H