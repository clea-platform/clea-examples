
/**
 * @file astarte-handler.h
 * @author Luca Di Mauro (luca.dimauro@seco.com)
 * @brief 
 * @version 0.1
 * @date 2022-05-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef ASTARTE_HANDLER_H
#define ASTARTE_HANDLER_H

#include <stdbool.h>
#include <astarte_device.h>
#include <esp_err.h>
#include <esp_event.h>
#include <esp_timer.h>


#define MAC_LENGTH 6
#define MAC_STRING_LENGTH 13
#define ENCODED_HWID_LENGTH 64
#define EXAMPLE_UUID "37119eb1-84fc-4e4b-97de-0b18ab1a49f1"

#define NVS_PARTITION "nvs"

#define FLOW_EVENT 111
#define POLLUTION_EVENT 222

ESP_EVENT_DECLARE_BASE(ASTARTE_HANDLER_EVENTS);
enum {
    ASTARTE_HANDLER_EVENT_CONNECT,
    ASTARTE_HANDLER_EVENT_DISCONNECT
};
#define ASTARTE_HANDLER_INITIALIZED_BIT BIT0
#define ASTARTE_HANDLER_FAILED_BIT      BIT1


/**
 * @brief Handle to actually use Astarte connection
 * 
 */
typedef struct _astarte_handler_s {
    astarte_device_handle_t astarte_device_handle;
    bool (*start)(struct _astarte_handler_s *this);
    bool (*stop)(struct _astarte_handler_s *this);

    esp_err_t (*publish_flow) (struct _astarte_handler_s *this, double flow_value);
    esp_err_t (*publish_velocity) (struct _astarte_handler_s *this, double velocity_value);
    esp_err_t (*publish_pollution) (struct _astarte_handler_s *this, double pollution_value);
    esp_err_t (*publish_event) (struct _astarte_handler_s *this, int event_id, double event_value, int note);
} astarte_handler_t;


// Defining astarte interfaces

const static astarte_interface_t air_data_interface = {
    .name           = "ai.clea.examples.AirData",
    .major_version  = 0,
    .minor_version  = 1,
    .ownership      = OWNERSHIP_DEVICE,
    .type           = TYPE_DATASTREAM,
};

const static astarte_interface_t event_history_interface    = {
    .name           = "ai.clea.examples.EventsHistory",
    .major_version  = 0,
    .minor_version  = 1,
    .ownership      = OWNERSHIP_DEVICE,
    .type           = TYPE_DATASTREAM,
};


astarte_handler_t *astarte_handler_create ();

void astarte_handler_destroy (astarte_handler_t *);

#endif //ASTARTE_HANDLER_H
