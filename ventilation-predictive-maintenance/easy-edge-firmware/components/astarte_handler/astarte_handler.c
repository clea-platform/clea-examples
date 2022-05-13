
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

#include <astarte_handler.h>
#include <astarte_bson.h>
#include <astarte_bson_types.h>
#include <astarte_credentials.h>
#include <astarte_hwid.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <mbedtls/base64.h>
#include <astarte_bson_serializer.h>
#include <uuid.h>


// Static private functions
static char *_get_hardware_id_encoded();
static esp_err_t _add_interface (astarte_device_handle_t device, const astarte_interface_t* interface_descriptor);

static bool _start_handler (astarte_handler_t* this);
static bool _stop_handler (astarte_handler_t* this);
static esp_err_t _publish_flow (struct _astarte_handler_s *this, double flow_value);
static esp_err_t _publish_velocity (struct _astarte_handler_s *this, double velocity_value);
static esp_err_t _publish_pollution (struct _astarte_handler_s *this, double pollution_value);
static esp_err_t _publish_event (struct _astarte_handler_s *this, int event_id, double event_value, int note);

static void _astarte_data_cb (astarte_device_data_event_t* event);
static void _astarte_connection_cb ();
static void _astarte_disconnection_cb ();




// =======================================
// =======================================



 
astarte_handler_t *astarte_handler_create() {
    const char *TAG                 = "astarte_handler_create";
    astarte_device_handle_t device  = NULL;

    astarte_credentials_use_nvs_storage(NVS_PARTITION);
    astarte_credentials_init();

    char *encoded_hwid = _get_hardware_id_encoded();

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

    device = astarte_device_init(&cfg);
    free(encoded_hwid);
    if (!device) {
        ESP_LOGE(TAG, "Cannot to init astarte device");
        goto astarte_init_error;
    }

    // Add BeverageData
    _add_interface (device, &air_data_interface);
    _add_interface (device, &event_history_interface);

    astarte_handler_t *astarte_handler = malloc(sizeof(astarte_handler_t));

    if (!astarte_handler) {
        ESP_LOGE (TAG, "Cannot allocate astarte handler");
        goto astarte_init_error;
    }

    astarte_handler->astarte_device_handle  = device;
    astarte_handler->start                  = &_start_handler;
    astarte_handler->stop                   = &_stop_handler;
    astarte_handler->publish_flow           = &_publish_flow;
    astarte_handler->publish_velocity       = &_publish_velocity;
    astarte_handler->publish_pollution      = &_publish_pollution;
    astarte_handler->publish_event          = &_publish_event;

    return astarte_handler;

astarte_init_error:
    if (device) {
        astarte_device_destroy(device);
    }
    return NULL;
}




void astarte_handler_destroy (astarte_handler_t *astarte_handler) {
    if (astarte_handler) {
        astarte_device_destroy(astarte_handler->astarte_device_handle);
        free(astarte_handler);
    }
}




// =======================================
// =======================================



 
static bool _start_handler (astarte_handler_t *this) {
    astarte_err_t err   = astarte_device_start (this->astarte_device_handle);
    if (err != ASTARTE_OK) {
        ESP_LOGW("start", "astarte device doesn't start, code err: %d", err);
        return false;
    }
    return true;
}

static bool _stop_handler (astarte_handler_t *this) {
    astarte_err_t err = astarte_device_stop(this->astarte_device_handle);
    if (err != ASTARTE_OK) {
        ESP_LOGW("stop", "astarte device doesn't stop, code err: %d", err);
        return false;
    }
    return true;
}

static char *_build_path (char *prefix, char *suffix) {
    size_t path_len = 1 + strlen(prefix) + 1 + strlen(suffix);
    char *path      = malloc (path_len+1);
    memset (path, '\0', path_len+1);
    strcpy (path, "/");
    strcat (path, prefix);
    strcat (path, "/");
    strcat (path, suffix);

    return path;
}

#if 0
static esp_err_t _publish_data (struct _astarte_handler_s *this, char *beverage_id, double revenue, int units) {

    const char *TAG     = "_publish_data";
    esp_err_t success   = ESP_OK;
    int doc_len         = 0;
    char *revenues_path = _build_path (beverage_id, "revenues");
    char *units_path    = _build_path (beverage_id, "units");
    ESP_LOGD (TAG, "Using paths:\n\t%s\n\t%s", revenues_path, units_path);//FIXME REMOVE ME!
    ESP_LOGW (TAG, "Values for %s are (u) %d\t(r) %f", beverage_id, units, revenue);//FIXME REMOVE ME!
    struct astarte_bson_serializer_t bs;
    
    memset (&bs, '\0', sizeof(struct astarte_bson_serializer_t));
    astarte_bson_serializer_init(&bs);
    astarte_bson_serializer_append_int32 (&bs, "units", units);
    astarte_bson_serializer_append_double (&bs, "revenues", revenue);

    astarte_bson_serializer_append_end_of_document(&bs);

    const void *doc = astarte_bson_serializer_get_document (&bs, &doc_len);
    if (!doc) {
        ESP_LOGE (TAG, "Cannot serialize bson document!\n\tunits: %d\n\trevenues: %f", units, revenue);
        success = ESP_FAIL;
    }
    else {
        astarte_err_t res   = ASTARTE_OK;

        res = astarte_device_stream_double (this->astarte_device_handle, beverage_data_interface.name,
                                            revenues_path, revenue, 0);
        if (res != ASTARTE_OK) {
            success = ESP_FAIL;
            ESP_LOGE (TAG, "Cannot publish data to astarte mapping %s", revenues_path);
            goto error_handling;
        }

        res = astarte_device_stream_integer (this->astarte_device_handle, beverage_data_interface.name,
                                                units_path, units, 0);
        if (res != ASTARTE_OK) {
            success = ESP_FAIL;
            ESP_LOGE (TAG, "Cannot publish data to astarte mapping %s", units_path);
            goto error_handling;
        }
    }
    astarte_bson_serializer_destroy (&bs);
    
    return success;

error_handling:
    ESP_LOGE (TAG, "Handling error!");
    astarte_bson_serializer_destroy (&bs);
    free (revenues_path);
    free (units_path);

    return success;
}
#endif

static esp_err_t _publish_flow (struct _astarte_handler_s *this, double flow_value) {
    astarte_err_t success   = astarte_device_stream_double (this->astarte_device_handle, "com.seco.AirData",
                                                            "/flow", flow_value, 0);
    return (success == ASTARTE_OK);
}

static esp_err_t _publish_velocity (struct _astarte_handler_s *this, double velocity_value) {
    astarte_err_t success   = astarte_device_stream_double (this->astarte_device_handle, "com.seco.AirData",
                                                            "/velocity", velocity_value, 0);
    return (success == ASTARTE_OK);
}

static esp_err_t _publish_pollution (struct _astarte_handler_s *this, double pollution_value) {
    astarte_err_t success   = astarte_device_stream_double (this->astarte_device_handle, "com.seco.AirData",
                                                            "/pollution", pollution_value, 0);
    return (success == ASTARTE_OK);
}

static esp_err_t _publish_event (struct _astarte_handler_s *this, int event_id, double event_value,
                                    int event_note) {
    esp_err_t success   = ESP_FAIL;

    struct astarte_bson_serializer_t aggr;
    astarte_bson_serializer_init(&aggr);

    if (event_value == FLOW_EVENT) {
        astarte_bson_serializer_append_string (&aggr, "detection", "Flow");
    }
    else {
        astarte_bson_serializer_append_string(&aggr, "detection", "Pollution");
    }
    astarte_bson_serializer_append_double(&aggr, "measure", event_value);
    astarte_bson_serializer_append_int32(&aggr, "noteCode", event_note);
    astarte_bson_serializer_append_end_of_document(&aggr);
    printf("Sending Event with value: %f\n", event_value);
    int size;
    const void *document    = astarte_bson_serializer_get_document(&aggr, &size);
    astarte_err_t res       = astarte_device_stream_aggregate (this->astarte_device_handle, "com.seco.EventsHistory",
                                                                "/event", document, 0);
    printf("Event sent!\n");

    return (res == ASTARTE_OK);
}

static char *_get_hardware_id_encoded () {
    const char *TAG = "_get_hardware_id_encoded";

    uint8_t mac[MAC_LENGTH];
    char mac_string[MAC_STRING_LENGTH];
    char *encoded_hwid = NULL;
    uuid_t namespace_uuid;
    uuid_t device_uuid;

#if CONFIG_USE_FIXED_ID == 0
    ESP_LOGD (TAG, "Using hardware ID");

    uuid_from_string(EXAMPLE_UUID, namespace_uuid);
    esp_err_t err = esp_wifi_get_mac(WIFI_IF_STA, mac);
    if (err != ESP_OK) {
        ESP_LOGE("hw", "Cannot get mac from wifi_STA (%s)", esp_err_to_name(err));
        return NULL;
    }
    snprintf(mac_string, MAC_STRING_LENGTH, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2],
        mac[3], mac[4], mac[5]);
    uuid_generate_v5(namespace_uuid, mac_string, strlen(mac_string), device_uuid);
    encoded_hwid = malloc(ENCODED_HWID_LENGTH);
    if (encoded_hwid == NULL) {
        ESP_LOGE("hw", "Cannot allocate memory to init base64 buffer");
        return NULL;
    }
    astarte_hwid_encode(encoded_hwid, ENCODED_HWID_LENGTH, (const uint8_t *) device_uuid);
#else
    ESP_LOGD (TAG, "Using fixed ID");
    encoded_hwid    = malloc (strlen(CONFIG_ASTARTE_ID)+1);
    memset (encoded_hwid, '\0', strlen(CONFIG_ASTARTE_ID)+1);
    memcpy (encoded_hwid, CONFIG_ASTARTE_ID, strlen(CONFIG_ASTARTE_ID));
#endif

    return encoded_hwid;
}

static esp_err_t _add_interface (astarte_device_handle_t device, const astarte_interface_t* interface_descriptor) {
    const char* TAG     = "_add_interface";
    astarte_err_t res   = astarte_device_add_interface (device, interface_descriptor);

    if (res != ASTARTE_OK) {
        ESP_LOGE (TAG, "Unable to add Astarte Interface (%s) error code: %d",
                        interface_descriptor->name, res);
        return ESP_FAIL;
    }

    return ESP_OK;
}

static void _astarte_data_cb (astarte_device_data_event_t *event) {
    /*ESP_LOGI(TAG, "Got Astarte data event, interface_name: %s, path: %s, bson_type: %d",
        event->interface_name, event->path, event->bson_value_type);*/
}

static void _astarte_connection_cb () {
    esp_event_post(ASTARTE_HANDLER_EVENTS, ASTARTE_HANDLER_EVENT_CONNECT, NULL, 0, 0);
}

static void _astarte_disconnection_cb () {
    esp_event_post(ASTARTE_HANDLER_EVENTS, ASTARTE_HANDLER_EVENT_DISCONNECT, NULL, 0, 0);
}
