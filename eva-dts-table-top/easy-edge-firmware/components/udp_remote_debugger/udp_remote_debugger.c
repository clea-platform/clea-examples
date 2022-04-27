
/**
 * @file udp_remote_debugger.h
 * @author Luca Di Mauro (luca.dimauro@seco.com)
 * @brief Source file for UDP remote debugger component
 * @version 0.1
 * @date 2022-04-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <udp_remote_debugger.h>

#include <stddef.h>

#include <esp_log.h>

static esp_err_t send_cb (char *message, size_t length, udp_remote_debugger_t *this);
static void event_handler ();


udp_remote_debugger_t *setup_debugger () {
    const char *TAG                 = "setup_debugger";
    udp_remote_debugger_t *debugger = NULL;

    debugger    = (udp_remote_debugger_t*) malloc (sizeof(udp_remote_debugger_t));
    if (!debugger) {
        ESP_LOGE (TAG, "Unable to create the debugger!");
        goto setup_error;
    }
    memset (debugger, '\0', sizeof (udp_remote_debugger_t));

    debugger->send                          = send_cb;
    debugger->_dest_addr.sin_addr.s_addr    = inet_addr(CONFIG_SERVER_IP);
    debugger->_dest_addr.sin_family         = AF_INET;
    debugger->_dest_addr.sin_port           = htons(CONFIG_SERVER_PORT);
    debugger->_dest_socket                  = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (debugger->_dest_socket < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        goto setup_error;
    }
    debugger->_mutex    = xSemaphoreCreateMutex();
    if (!debugger->_mutex) {
        ESP_LOGE (TAG, "Unable to create semaphore!");
        goto setup_error;
    }

    return debugger;

setup_error:
    if (debugger) {
        if (debugger->_mutex)
            vSemaphoreDelete (debugger->_mutex);
        free (debugger);
    }
    
    return NULL;
}




static esp_err_t send_cb (char *message, size_t lenght, udp_remote_debugger_t *this) {
    int err = 0;

    if (xSemaphoreTake (this->_mutex, portMAX_DELAY) == pdTRUE) {
        err = sendto (this->_dest_socket, message, lenght, 0, (struct sockaddr *)&(this->_dest_addr), sizeof(this->_dest_addr));
        if (err < 0) {
            printf ("Error occurred during sending: errno %d\n\n", errno);
        }
        xSemaphoreGive (this->_mutex);
    }
    else {
        printf ("Cannot take mutex..\n");
    }
        
    return !(err == lenght);
}




static void event_handler () {
    // TODO
}