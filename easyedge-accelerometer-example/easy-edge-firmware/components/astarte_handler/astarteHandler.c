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

#define EXAMPLE_UUID "37119eb1-84fc-4e4b-97de-0b18ab1a49f1"
#define MAC_LENGTH 6
#define ENCODED_HWID_LENGTH 64
#define MAC_STRING_LENGTH 13

static const char *TAG = "ASTARTE_HANDLER";

bool start(AstarteHandler *this);

bool stop(AstarteHandler *this);

void pub_acc_data(astarte_device_handle_t astarte_device, float x, float y, float z);

const static astarte_interface_t acc_data_interface = { .name = "ai.clea.examples.Accelerometer",
    .major_version = 0,
    .minor_version = 1,
    .ownership = OWNERSHIP_DEVICE,
    .type = TYPE_DATASTREAM };

const static astarte_interface_t server_datastream_interface = {
    .name = "org.astarteplatform.esp32.ServerDatastream",
    .major_version = 0,
    .minor_version = 1,
    .ownership = OWNERSHIP_SERVER,
    .type = TYPE_DATASTREAM,
};

static void astarte_data_events_handler(astarte_device_data_event_t *event)
{
    ESP_LOGI(TAG, "Got Astarte data event, interface_name: %s, path: %s, bson_type: %d",
        event->interface_name, event->path, event->bson_value_type);

    if (strcmp(event->interface_name, server_datastream_interface.name) == 0
        && strcmp(event->path, "/led") == 0 && event->bson_value_type == BSON_TYPE_BOOLEAN) {
        int led_state = astarte_bson_value_to_int8(event->bson_value);

        esp_event_post(ASTARTE_EVENTS, ASTARTE_EVENT_LED, (void*) &led_state, sizeof(int), 0);
    }
}

static void astarte_connection_events_handler()
{
    esp_event_post(ASTARTE_EVENTS, ASTARTE_EVENT_CONNECT, NULL, 0, 0);
    ESP_LOGI(TAG, "on_connected");
}

static void astarte_disconnection_events_handler()
{
    esp_event_post(ASTARTE_EVENTS, ASTARTE_EVENT_CONNECT, NULL, 0, 0);
    ESP_LOGW(TAG, "on_disconnected");
}

esp_err_t add_interfaces(astarte_device_handle_t device)
{
    astarte_err_t ret;
    ret = astarte_device_add_interface(device, &acc_data_interface);

    if (ret != ASTARTE_OK) {
        ESP_LOGE(TAG, "Unable to add Astarte Interface ( %s ) error code: %d",
            acc_data_interface.name, ret);
        return ESP_FAIL;
    }

    ret = astarte_device_add_interface(device, &server_datastream_interface);
    if (ret != ASTARTE_OK) {
        ESP_LOGE(TAG, "Unable to add Astarte Interface ( %s ) error code: %d",
            server_datastream_interface.name, ret);
        return ESP_FAIL;
    }

    return ESP_OK;
}

AstarteHandler *astarteHandler_init()
{
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

    astarte_handler->astarteDeviceHandle = device;
    astarte_handler->start = &start;
    astarte_handler->stop = &stop;

    return astarte_handler;

astarte_init_error:
    if (device) {
        astarte_device_destroy(device);
    }
    return NULL;
}

bool start(AstarteHandler *this)
{
    astarte_err_t err = astarte_device_start(this->astarteDeviceHandle);
    if (err != ASTARTE_OK) {
        ESP_LOGW(TAG, "astarte device doesn't start, code err: %d", err);
        return false;
    }

    ESP_LOGI(TAG, "astarte device start");
    return true;
}

void pub_acc_data(astarte_device_handle_t astarte_device, float x, float y, float z)
{
    struct astarte_bson_serializer_t bs;
    astarte_bson_serializer_init(&bs);

    astarte_bson_serializer_append_double(&bs, "x", x);
    astarte_bson_serializer_append_double(&bs, "y", y);
    astarte_bson_serializer_append_double(&bs, "z", z);

    astarte_bson_serializer_append_end_of_document(&bs);

    int doc_len;
    const void *doc = astarte_bson_serializer_get_document(&bs, &doc_len);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t milliseconds = tv.tv_sec * 1000LL + tv.tv_usec / 1000LL;

    astarte_device_stream_aggregate_with_timestamp(
        astarte_device, acc_data_interface.name, "/accelerometer", doc, milliseconds, 0);
    astarte_bson_serializer_destroy(&bs);
}

bool stop(AstarteHandler *this)
{
    astarte_err_t err = astarte_device_stop(this->astarteDeviceHandle);
    if (err != ASTARTE_OK) {
        ESP_LOGW(TAG, "astarte device doesn't stop, code err: %d", err);
        return false;
    }

    ESP_LOGI(TAG, "astarte device stop");
    return true;
}

void astarteHandler_destroy(AstarteHandler *astarte_handler)
{
    if (astarte_handler) {
        astarte_device_destroy(astarte_handler->astarteDeviceHandle);
        free(astarte_handler);
    }
}

char *astarteHandler_get_hardware_id_encoded()
{
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
