#ifndef ASTARTE_HANDLER_ASTARTEHANDLER_H
#define ASTARTE_HANDLER_ASTARTEHANDLER_H

#include <stdbool.h>
#include "astarte_device.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_timer.h"

typedef void (*func_bool_ptr)(bool);

void set_led_callback(func_bool_ptr led_cb);

void pub_acc_data(astarte_device_handle_t astarte_device, float x, float y, float z);

ESP_EVENT_DECLARE_BASE(ASTARTE_EVENTS);
#define NVS_PARTITION "nvs"

enum {
    ASTARTE_EVENT_CONNECT,
    ASTARTE_EVENT_DISCONNECT,
    ASTARTE_EVENT_LED
};

typedef struct _astarteHandler {
    astarte_device_handle_t astarteDeviceHandle;

    bool (*start)(struct _astarteHandler *this);

    bool (*stop)(struct _astarteHandler *this);
} AstarteHandler;

AstarteHandler *astarteHandler_init();

void astarteHandler_destroy(AstarteHandler *);

char *astarteHandler_get_hardware_id_encoded();

#endif //ASTARTE_HANDLER_ASTARTEHANDLER_H
