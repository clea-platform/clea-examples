#ifndef ASTARTE_HANDLER_ASTARTEHANDLER_H
#define ASTARTE_HANDLER_ASTARTEHANDLER_H

#include <stdbool.h>
#include "astarte_device.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "coffee_machine_sensor.h"

ESP_EVENT_DECLARE_BASE(ASTARTE_EVENTS);
#define NVS_PARTITION "nvs"


enum {
    ASTARTE_EVENT_CONNECT,
    ASTARTE_EVENT_DISCONNECT,
    ASTARTE_EVENT_LED
};


typedef struct _privateData PrivateData;

typedef struct _astarteHandler {
    astarte_device_handle_t astarteDeviceHandle;

    PrivateData* private_data;

    bool (*start)(struct _astarteHandler *this);

    bool (*stop)(struct _astarteHandler *this);

    void (*publish_event)(struct _astarteHandler *this, coffee_machine_event event);
} AstarteHandler;


AstarteHandler *astarteHandler_init();

void astarteHandler_destroy(AstarteHandler *);

char *astarteHandler_get_hardware_id_encoded();

#endif //ASTARTE_HANDLER_ASTARTEHANDLER_H
