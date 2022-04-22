
/**
 * @file astarte-handler.c
 * @author Luca Di Mauro (luca.dimauro@seco.com)
 * @brief 
 * @version 0.1
 * @date 2022-04-22
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <astarte-handler.h>

#include <astarte_hwid.h>
#include <astarte_credentials.h>

#include <esp_wifi.h>
#include <uuid.h>

#include <string.h>


static char* _astarte_handler_get_hardware_id_encoded ();
static esp_err_t _add_interface (astarte_device_handle_t device, astarte_interface_t* interface_descriptor);

static bool _start_handler (astarte_handler_t* this);
static bool _stop_handler (astarte_handler_t* this);

static void _astarte_data_cb (astarte_device_data_event_t* event);
static void _astarte_connection_cb ();
static void _astarte_disconnection_cb ();




astarte_handler_t* astarte_handler_create () {
    const char* TAG                 = "astarte_handler_create";
    astarte_handler_t* handler      = NULL;
    astarte_device_handle_t device  = NULL;
    char* encoded_hwid              = NULL;

    astarte_credentials_use_nvs_storage (NVS_PARTITION);
    astarte_credentials_init ();
    
#if CONFIG_USE_FIXED_ID == 0 
    ESP_LOGD (TAG, "Using hardware ID");
    encoded_hwid    = _astarte_handler_get_hardware_id_encoded();
#else
    ESP_LOGD (TAG, "Using fixed ID");
    encoded_hwid    = malloc (strlen(CONFIG_ASTARTE_ID)+1);
    memset (encoded_hwid, '\0', strlen(CONFIG_ASTARTE_ID)+1);
    memcpy (encoded_hwid, CONFIG_ASTARTE_ID, strlen(CONFIG_ASTARTE_ID));
#endif

    if (!encoded_hwid) {
        goto astarte_init_error;
    }

    ESP_LOGD (TAG, "Astarte Device ID -> %s", encoded_hwid);

    astarte_device_config_t cfg = {
        .data_event_callback            = _astarte_data_cb,
        .connection_event_callback      = _astarte_connection_cb,
        .disconnection_event_callback   = _astarte_disconnection_cb,
        .hwid                           = (char *) encoded_hwid
    };

    // Creating astarte device
    device  = astarte_device_init (&cfg);
    free(encoded_hwid);
    if (!device) {
        ESP_LOGE (TAG, "Cannot init astarte device");
        goto astarte_init_error;
    }

    // TODO Adding interfaces

    // Creating astarte handler
    handler = malloc (sizeof(astarte_handler_t));
    if (!handler) {
        ESP_LOGE(TAG, "Cannot allocate astarte handler");
        goto astarte_init_error;
    }
    memset (handler, '\0', sizeof(astarte_handler_t));

    handler->device_handle  = device;
    handler->start          = &_start_handler;
    handler->stop           = &_stop_handler;

    return handler;

astarte_init_error:
    ESP_LOGE (TAG, "An error happened during handler creation!");
    if (device) {
        astarte_device_destroy(device);
    }

    return NULL;
}




esp_err_t astarte_handler_destroy (astarte_handler_t* handler) {
    esp_err_t res   = ESP_OK;
    
    if (handler) {
        astarte_device_destroy(handler->device_handle);
        free(handler);
    }
    return res;
}




// =======================================
// =======================================



 
static char* _astarte_handler_get_hardware_id_encoded() {
    const char* TAG = "astarte_gandler_get_hardware_id_encoded";
    uint8_t mac[MAC_LENGTH];
    char mac_string[MAC_STRING_LENGTH];
    char *encoded_hwid = NULL;
    uuid_t namespace_uuid;
    uuid_t device_uuid;

    uuid_from_string (EXAMPLE_UUID, namespace_uuid);
    esp_err_t err   = esp_wifi_get_mac (WIFI_IF_STA, mac);
    if (err != ESP_OK) {
        ESP_LOGE (TAG, "Cannot get mac from wifi_STA (%s)", esp_err_to_name(err));
        return NULL;
    }
    snprintf (mac_string, MAC_STRING_LENGTH, "%02X%02X%02X%02X%02X%02X",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    uuid_generate_v5 (namespace_uuid, mac_string, strlen(mac_string), device_uuid);
    encoded_hwid = malloc (ENCODED_HWID_LENGTH);
    if (encoded_hwid == NULL) {
        ESP_LOGE (TAG, "Cannot allocate memory to init base64 buffer");
        return NULL;
    }
    astarte_hwid_encode (encoded_hwid, ENCODED_HWID_LENGTH, (const uint8_t *) device_uuid);

    return encoded_hwid;
}




static esp_err_t _add_interface (astarte_device_handle_t device, astarte_interface_t* interface_descriptor) {
    const char* TAG     = "_add_interface";
    astarte_err_t res   = astarte_device_add_interface (device, interface_descriptor);

    if (res != ASTARTE_OK) {
        ESP_LOGE (TAG, "Unable to add Astarte Interface (%s) error code: %d",
                        interface_descriptor->name, res);
        return ESP_FAIL;
    }

    return ESP_OK;
}




static bool _start_handler (astarte_handler_t* this) {
    const char* TAG     = "_start_handler";
    astarte_err_t err   = astarte_device_start (this->device_handle);
    
    if (err != ASTARTE_OK) {
        ESP_LOGE (TAG, "astarte device doesn't start, code err: %d", err);
    }

    return (err==ASTARTE_OK);
}




static bool _stop_handler (astarte_handler_t* this) {
    const char* TAG     = "_stop_handler";
    astarte_err_t err   = astarte_device_stop (this->device_handle);
    if (err != ASTARTE_OK) {
        ESP_LOGW (TAG, "astarte device doesn't stop, code err: %d", err);
        return false;
    }

    ESP_LOGI (TAG, "astarte device stopped");
    return true;
}





static void _astarte_data_cb (astarte_device_data_event_t* event) {
    const char* TAG = "_astarte_data_cb";
    ESP_LOGI (TAG, "Got Astarte data event, interface_name: %s, path: %s, bson_type: %d",
                    event->interface_name, event->path, event->bson_value_type);

    // TODO

    /*if (strcm p(event->interface_name, server_datastream_interface.name) == 0
        && strcmp(event->path, "/led") == 0 && event->bson_value_type == BSON_TYPE_BOOLEAN) {
        int led_state = astarte_bson_value_to_int8(event->bson_value);

        esp_event_post(ASTARTE_HANDLER_EVENTS, ASTARTE_EVENT_LED, (void*) &led_state, sizeof(int), 0);
    }*/
}




static void _astarte_connection_cb () {
    ESP_LOGV ("_astarte_connection_cb", "Connected to astarte!");
    // Triggering relative event
    esp_event_post (ASTARTE_HANDLER_EVENTS, ASTARTE_HANDLER_EVENT_CONNECT, NULL, 0, 0);
}




static void _astarte_disconnection_cb () {
    ESP_LOGV ("_astarte_disconnection_cb", "Disconnected from astarte!");
    // Triggering relative event
    esp_event_post(ASTARTE_HANDLER_EVENTS, ASTARTE_HANDLER_EVENT_DISCONNECT, NULL, 0, 0);
}