//
// Created by harlem88 on 21/01/20.
//

#ifndef COFFEE_MACHINE_SENSOR_H
#define COFFEE_MACHINE_SENSOR_H
#include "esp_event.h"

#define GPIO_ON_OFF_LED 5
#define GPIO_WATER_LED 19
#define GPIO_CONTAINER_LED 2

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