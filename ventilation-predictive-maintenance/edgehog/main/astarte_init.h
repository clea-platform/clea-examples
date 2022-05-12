#include <esp_log.h>

#include "astarte_bson.h"
#include "astarte_bson_serializer.h"
#include "astarte_bson_types.h"
#include "astarte_credentials.h"
#include "astarte_device.h"

#include <time.h>
#include <sys/time.h>

#include "utils.h"

#define ASTARTE_TAG "POC"

#define FLOW_EVENT 111
#define POLLUTION_EVENT 222

#define WARNING_NOTE 1
#define DANGER_NOTE 2

/**
 * @brief Function to get the actual time in ms from epoch.
 *
 * @return int64_t The time in ms from epoch.
 */
int64_t get_millis()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
}

/**
 * The astarte device pointer.
 */
astarte_device_handle_t device = NULL;

// Astarte interfaces ---------------------------

const static astarte_interface_t AirData = {
    .name = "com.seco.AirData",
    .major_version = 0,
    .minor_version = 1,
    .ownership = OWNERSHIP_DEVICE,
    .type = TYPE_DATASTREAM,
};

const static astarte_interface_t EventsHistory = {
    .name = "com.seco.EventsHistory",
    .major_version = 0,
    .minor_version = 1,
    .ownership = OWNERSHIP_DEVICE,
    .type = TYPE_DATASTREAM,
};

//-----------------------------------------------

// Astarte handlers -----------------------------

static void astarte_connection_events_handler(astarte_device_connection_event_t *event)
{
    ESP_LOGI(ASTARTE_TAG, "Astarte device connected, session_present: %d", event->session_present);
}

static void astarte_disconnection_events_handler(astarte_device_disconnection_event_t *event)
{
    ESP_LOGI(ASTARTE_TAG, "Astarte device disconnected");
}

static void astarte_data_events_handler(astarte_device_data_event_t *event)
{
    ESP_LOGI(ASTARTE_TAG, "Got Astarte data event, interface_name: %s, path: %s, bson_type: %d",
             event->interface_name, event->path, event->bson_value_type);
}

//-----------------------------------------------

/**
 * @brief Function to send the "Flow" to Astarte.
 *
 * @param flow A double value representing the flow between 0 an 1.
 */
void send_flow(double flow)
{
    astarte_device_stream_double(device, "com.seco.AirData",
                                 "/flow", flow, 0);
}

/**
 * @brief Function to send the "pollution" to Astarte.
 *
 * @param pollution A double value representing the pollution in μg/m3.
 */
void send_pollution(double pollution)
{
    astarte_device_stream_double(device, "com.seco.AirData",
                                 "/pollution", pollution, 0);
}

/**
 * @brief Function to send the "Air Velocity in m/s" to Astarte.
 *
 * @param velocity A double value representing the Air Velocity in m/s.
 */
void send_velocity(double velocity)
{
    astarte_device_stream_double(device, "com.seco.AirData",
                                 "/velocity", velocity, 0);
}

/**
 * @brief Function to send an event to Astarte
 *
 * @param event integer representing the kind of event (FLOW_EVENT or POLLUTION_EVENT)
 * @param value the double value that generated the event
 * @param note an integer that represents additional info on the event
 */
void send_event(int event, double value, int note)
{
    struct astarte_bson_serializer_t aggr;
    astarte_bson_serializer_init(&aggr);

    if (event == FLOW_EVENT)
    {
        astarte_bson_serializer_append_string(&aggr, "detection", "Flow");
    }
    else
    {
        astarte_bson_serializer_append_string(&aggr, "detection", "Pollution");
    }
    astarte_bson_serializer_append_double(&aggr, "measure", value);
    astarte_bson_serializer_append_int32(&aggr, "noteCode", note);
    astarte_bson_serializer_append_end_of_document(&aggr);
    printf("Sending Event with value: %f\n", value);
    int size;
    const void *document = astarte_bson_serializer_get_document(&aggr, &size);
    // astarte_device_stream_aggregate_with_timestamp(device, "com.seco.EventsHistory", "/event",
    //                                                document, get_millis(), 0);
    astarte_device_stream_aggregate(device, "com.seco.EventsHistory", "/event", document, 0);
    printf("Event sent!\n");
}