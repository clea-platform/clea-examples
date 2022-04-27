
/**
 * @file udp_remote_debugger.h
 * @author Luca Di Mauro (luca.dimauro@seco.com)
 * @brief Header file for UDP remote debugger component
 * @version 0.1
 * @date 2022-04-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#ifndef UDP_REMOTE_DEBUGGER_H
#define UDP_REMOTE_DEBUGGER_H


#include <esp_err.h>
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include <freertos/semphr.h>


typedef struct _udp_remote_debugger_s{
    struct sockaddr_in _dest_addr;
    int _dest_socket;
    SemaphoreHandle_t _mutex;
    esp_err_t (*send) (char *message, size_t length, struct _udp_remote_debugger_s *this);
} udp_remote_debugger_t;


udp_remote_debugger_t *setup_debugger ();


#endif // UDP_REMOTE_DEBUGGER_H