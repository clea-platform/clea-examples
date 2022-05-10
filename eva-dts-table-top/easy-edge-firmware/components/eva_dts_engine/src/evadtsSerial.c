//
// Created by harlem88 on 06/10/20.
//

#include "driver/uart.h"
#include "evadtsSerial.h"
#include "driver/gpio.h"
#include <esp_log.h>

#define BUF_SIZE (128)
#define uart_num UART_NUM_2
static const char* TAG = "EVADTS_SERIAL";

bool evadtsSerial_setup(){
    uart_config_t uart_config = {
            .baud_rate = 9600,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    if(uart_param_config(uart_num, &uart_config) != ESP_OK) return false;

    if(uart_driver_install(uart_num, BUF_SIZE *2, 0, 0, NULL, 0)!= ESP_OK) return false;

    return uart_set_pin(uart_num,   GPIO_NUM_1, GPIO_NUM_3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) == ESP_OK;
}

SerialDataMsg *evadtsSerial_read() {
    SerialDataMsg *serialDataMsg = (SerialDataMsg *) malloc(sizeof(SerialDataMsg));

    if (serialDataMsg != NULL) {
        serialDataMsg->bytes = (uint8_t *) malloc(BUF_SIZE);
        int len = uart_read_bytes(uart_num, (uint8_t *) serialDataMsg->bytes, BUF_SIZE, 10 / portTICK_RATE_MS);
        serialDataMsg->length = len;
        /*ESP_LOGI("Serial", "read len %d 0x%x", serialDataMsg->length, serialDataMsg->bytes[0]);
        for(int i= 0; i<len; i++){
            printf("0x%x ", serialDataMsg->bytes[i]);
        }
        printf("\n");*/
    }

    return serialDataMsg;
}

int evadtsSerial_write(SerialDataMsg dataMsg){
    int ret= uart_write_bytes(uart_num, (const char *) dataMsg.bytes, dataMsg.length);
    //ESP_LOGI("Serial", "write len %d 0x%x",dataMsg.length, dataMsg.bytes[0]);
    return ret;
}

bool evadtsSerial_close(){
    return uart_driver_delete(uart_num) == ESP_OK;
}

void evadtsSerial_freeSerialData(SerialDataMsg *data) {
    if (data != NULL) {
        if (data->bytes != NULL) free((uint8_t *) data->bytes);
        free(data);
    }
}
