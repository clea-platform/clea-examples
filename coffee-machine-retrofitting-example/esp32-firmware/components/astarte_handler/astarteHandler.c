#include "astarteHandler.h"
#include "astarte_bson.h"
#include "astarte_bson_types.h"
#include "astarte_credentials.h"
#include "astarte_hwid.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "mbedtls/base64.h"
#include <astarte_bson_serializer.h>
#include <uuid.h>
#include <nvs_flash.h>
#include <nvs.h>

#define EXAMPLE_UUID "37119eb1-84fc-4e4b-97de-0b18ab1a49f1"
#define MAC_LENGTH 6
#define ENCODED_HWID_LENGTH 64
#define MAC_STRING_LENGTH 13

static const char *TAG = "ASTARTE_HANDLER";
ESP_EVENT_DEFINE_BASE(ASTARTE_EVENTS);

bool start(AstarteHandler *this);

bool stop(AstarteHandler *this);

void publish_event(AstarteHandler *this, coffee_machine_event event);

struct _privateData{
    nvs_handle_t nvs_handle;
    int32_t short_coffee_cnt;
    int32_t long_coffee_cnt;
};


const static astarte_interface_t machine_counters_interface = {
    .name = "ai.clea.examples.machine.Counters",
    .major_version = 0,
    .minor_version = 1,
    .ownership = OWNERSHIP_DEVICE,
    .type = TYPE_DATASTREAM
};


const static astarte_interface_t machine_status_interface = {
    .name = "ai.clea.examples.machine.Status",
    .major_version = 0,
    .minor_version = 1,
    .ownership = OWNERSHIP_DEVICE,
    .type = TYPE_DATASTREAM,
};


static void astarte_data_events_handler (astarte_device_data_event_t *event) {
    ESP_LOGI(TAG, "Got Astarte data event, interface_name: %s, path: %s, bson_type: %d",
        event->interface_name, event->path, event->bson_value_type);
}


static void astarte_connection_events_handler() {
    esp_event_post(ASTARTE_EVENTS, ASTARTE_EVENT_CONNECT, NULL, 0, 0);
}


static void astarte_disconnection_events_handler() {
    esp_event_post(ASTARTE_EVENTS, ASTARTE_EVENT_CONNECT, NULL, 0, 0);
    ESP_LOGW(TAG, "ASTARTE DISCONNECTED!");
    // FIXME Try to perform a connection
}


esp_err_t add_interfaces (astarte_device_handle_t device) {
    astarte_err_t ret;
    ret = astarte_device_add_interface(device, &machine_counters_interface);

    if (ret != ASTARTE_OK) {
        ESP_LOGE(TAG, "Unable to add Astarte Interface ( %s ) error code: %d",
            machine_counters_interface.name, ret);
        return ESP_FAIL;
    }

    ret = astarte_device_add_interface(device, &machine_status_interface);
    if (ret != ASTARTE_OK) {
        ESP_LOGE(TAG, "Unable to add Astarte Interface ( %s ) error code: %d",
            machine_status_interface.name, ret);
        return ESP_FAIL;
    }

    return ESP_OK;
}


esp_err_t update_counter (nvs_handle_t* nvs_handle, int32_t* counter, int32_t new_value, char* nvs_key) {
    esp_err_t esp_err   = ESP_OK;

    if ((esp_err=nvs_set_i32 (*nvs_handle, nvs_key, new_value)) != ESP_OK) {
        return esp_err;
    }
    if ((esp_err=nvs_commit (*nvs_handle)) != ESP_OK) {
        return esp_err;
    }

    *counter    = new_value;

    return ESP_OK;
}


AstarteHandler *astarteHandler_init() {
    astarte_device_handle_t device = NULL;

    astarte_credentials_use_nvs_storage(NVS_PARTITION);
    astarte_credentials_init();

    char *encoded_hwid = astarteHandler_get_hardware_id_encoded();

    if (!encoded_hwid) {
        goto astarte_init_error;
    }

    ESP_LOGI(TAG, "Astarte Device ID -> %s", encoded_hwid);
    astarte_device_config_t cfg = {
        .data_event_callback = astarte_data_events_handler,
        .connection_event_callback = astarte_connection_events_handler,
        .disconnection_event_callback = astarte_disconnection_events_handler,
        .hwid = (char *) encoded_hwid
    };

    device = astarte_device_init(&cfg);
    free(encoded_hwid);

    if (!device) {
        ESP_LOGE(TAG, "Cannot to init astarte device");
        goto astarte_init_error;
    }

    ESP_ERROR_CHECK(add_interfaces(device));
    AstarteHandler *astarte_handler = malloc(sizeof(AstarteHandler));

    if (!astarte_handler) {
        ESP_LOGE(TAG, "Cannot allocate astarte handler");
        goto astarte_init_error;
    }

    astarte_handler->private_data = malloc(sizeof(struct _privateData));
    if (!astarte_handler->private_data) {
        ESP_LOGE(TAG, "Cannot allocate astarte private data");
        goto astarte_init_error;
    }

    // Opening NVS
    esp_err_t esp_err       = nvs_open ("counters_part", NVS_READWRITE, &(astarte_handler->private_data->nvs_handle));
    ESP_ERROR_CHECK (esp_err);
    // Loading counters from NVS
    esp_err = nvs_get_i32 (astarte_handler->private_data->nvs_handle, "ss_coffee",
                            &(astarte_handler->private_data->short_coffee_cnt));
    if (esp_err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW (TAG, "ss_coffee key not present in NVS. Setting its value to 0");
        ESP_ERROR_CHECK (update_counter (&(astarte_handler->private_data->nvs_handle),
                                            &(astarte_handler->private_data->short_coffee_cnt), 0, "ss_coffee"));
    }
    esp_err = nvs_get_i32 (astarte_handler->private_data->nvs_handle, "sl_coffee",
                            &(astarte_handler->private_data->long_coffee_cnt));
    if (esp_err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW (TAG, "sl_coffee key not present in NVS. Setting its value to 0");
        ESP_ERROR_CHECK (update_counter (&(astarte_handler->private_data->nvs_handle),
                                            &(astarte_handler->private_data->long_coffee_cnt), 0, "sl_coffee"));
    }

    ESP_LOGI (TAG, "Counters are\n\tssc: %d\n\tslc: %d", astarte_handler->private_data->short_coffee_cnt,
                                                            astarte_handler->private_data->long_coffee_cnt);

    astarte_handler->astarteDeviceHandle = device;
    astarte_handler->start = &start;
    astarte_handler->stop = &stop;
    astarte_handler->publish_event = &publish_event;

    return astarte_handler;

astarte_init_error:
    if (device) {
        astarte_device_destroy(device);
    }
    return NULL;
}


bool start (AstarteHandler *this) {
    astarte_err_t err = astarte_device_start(this->astarteDeviceHandle);
    if (err != ASTARTE_OK) {
        ESP_LOGW(TAG, "astarte device doesn't start, code err: %d", err);
        return false;
    }

    ESP_LOGI(TAG, "astarte device start");
    return true;
}


void publish_event (AstarteHandler *this, coffee_machine_event event) {
    
    switch (event) {
        case CONTAINER_OFF_ALARM_EVENT:
            astarte_device_stream_string(this->astarteDeviceHandle, machine_status_interface.name, "/containerStatus", "CONTAINER_OFF_ALARM_EVENT", 0);
            break;
        case CONTAINER_OPEN_ALARM_EVENT:
            astarte_device_stream_string(this->astarteDeviceHandle, machine_status_interface.name, "/containerStatus", "CONTAINER_OPEN_ALARM_EVENT", 0);
            break;
        case CONTAINER_FULL_ALARM_EVENT:
            astarte_device_stream_string(this->astarteDeviceHandle, machine_status_interface.name, "/containerStatus", "CONTAINER_FULL_ALARM_EVENT", 0);
            break;
        case WATER_OFF_ALARM_EVENT:
            astarte_device_stream_string(this->astarteDeviceHandle, machine_status_interface.name, "/waterStatus", "WATER_OFF_ALARM_EVENT", 0);
            break;
        case WATER_EMPTY_ALARM_EVENT:
            astarte_device_stream_string(this->astarteDeviceHandle, machine_status_interface.name, "/waterStatus", "WATER_EMPTY_ALARM_EVENT", 0);
            break;
        case WATER_OPEN_ALARM_EVENT:
            astarte_device_stream_string(this->astarteDeviceHandle, machine_status_interface.name, "/waterStatus", "WATER_OPEN_ALARM_EVENT", 0);
            break;
        case COFFEE_SHORT_EVENT:
            ESP_ERROR_CHECK (update_counter (&(this->private_data->nvs_handle), &(this->private_data->short_coffee_cnt),
                                                this->private_data->short_coffee_cnt+1, "ss_coffee"));
            astarte_device_set_integer_property(this->astarteDeviceHandle, machine_counters_interface.name, "/shortCoffee", this->private_data->short_coffee_cnt);
            break;
        case COFFEE_LONG_EVENT:
            ESP_ERROR_CHECK (update_counter (&(this->private_data->nvs_handle), &(this->private_data->long_coffee_cnt),
                                                this->private_data->long_coffee_cnt+1, "ss_coffee"));
            astarte_device_set_integer_property(this->astarteDeviceHandle, machine_counters_interface.name, "/longCoffee", this->private_data->long_coffee_cnt);
            break;
        default:
            ESP_LOGW(TAG, "event not supported");
    }
}


bool stop (AstarteHandler *this) {
    astarte_err_t err = astarte_device_stop(this->astarteDeviceHandle);
    if (err != ASTARTE_OK) {
        ESP_LOGW(TAG, "astarte device doesn't stop, code err: %d", err);
        return false;
    }

    ESP_LOGI(TAG, "astarte device stop");
    return true;
}


void astarteHandler_destroy (AstarteHandler *astarte_handler) {
    if (astarte_handler) {
        astarte_device_destroy(astarte_handler->astarteDeviceHandle);
        free(astarte_handler->private_data);
        free(astarte_handler);
    }
}


char *astarteHandler_get_hardware_id_encoded() {
    uint8_t mac[MAC_LENGTH];
    char mac_string[MAC_STRING_LENGTH];
    char *encoded_hwid = NULL;
    uuid_t namespace_uuid;
    uuid_t device_uuid;

    uuid_from_string(EXAMPLE_UUID, namespace_uuid);
    esp_err_t err = esp_wifi_get_mac(WIFI_IF_STA, mac);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Cannot get mac from wifi_STA (%s)", esp_err_to_name(err));
        return NULL;
    }
    snprintf(mac_string, MAC_STRING_LENGTH, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2],
        mac[3], mac[4], mac[5]);
    uuid_generate_v5(namespace_uuid, mac_string, strlen(mac_string), device_uuid);
    encoded_hwid = malloc(ENCODED_HWID_LENGTH);
    if (encoded_hwid == NULL) {
        ESP_LOGE(TAG, "Cannot allocate memory to init base64 buffer");
        return NULL;
    }
    astarte_hwid_encode(encoded_hwid, ENCODED_HWID_LENGTH, (const uint8_t *) device_uuid);
    return encoded_hwid;
}
