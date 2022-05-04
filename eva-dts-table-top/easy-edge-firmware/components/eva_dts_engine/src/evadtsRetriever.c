//
// Created by harlem88 on 06/10/20.
//

#include "evadtsRetriever.h"
#include <esp_log.h>
#include "ddcmp/ddcmpMessage.h"
#include "ddcmp/stackMessage.h"
#include "ddcmp/ackMessage.h"
#include "ddcmp/dataHeaderMessage.h"
#include "ddcmp/wayMessage.h"
#include "ddcmp/dateReadMessage.h"
#include "ddcmp/dataBlockMessage.h"
#include "evadtsSerial.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define DDCMP_START_ATTEMPTS_MAX 10
#define DDCMP_ATTEMPTS_MAX 3
static const char* TAG = "EVADTS_RETRIEVER";

static EvadtsPayloadRaw* buildPayload(int* rx, bool(*endCondition)(DataHeaderMessage *));
static bool linkInit();
static bool startMsg();
static bool wayMsg();
static EvadtsPayloadRaw* dataAuditCollection(uint8_t ddcmpListNumber);
static DataReadMessage *readInitData(uint8_t ddcmpListNumber);

static int sPayloadRemainLength=0;

static bool endConditionIsLastBlock(DataHeaderMessage *it){
    return it->isLastBlock(it);
}

static bool endConditionRemainLength(DataHeaderMessage *it){
    sPayloadRemainLength -= it->getBlockLengthWithoutCrc(it);
    return sPayloadRemainLength <= 0;
}

static bool connectionSetup(){
    return evadtsSerial_setup();
}

static bool connectionClose(){
    return evadtsSerial_close();
}

EvadtsPayloadRaw* evadtsRetriever_readDataCollection(bool init_connection, bool resetValue){
    EvadtsPayloadRaw* payloadRaw= NULL;
    bool connection_success     = true;

#ifdef CONFIG_USE_RECORDED_DATA
            extern uint8_t report_example_start[] asm("_binary_evoca_kalea_report_txt_start");
            extern uint8_t report_example_end[] asm("_binary_evoca_kalea_report_txt_end");
            //ESP_LOGI (TAG, "Loading already loaded data from %ld to %ld..\n%s", (long int) report_example_start, (long int) report_example_end, report_example_start);

            // Copying report data to payloadRaw variable
            payloadRaw  = malloc (sizeof(EvadtsPayloadRaw));
            memset (payloadRaw, '\0', sizeof(EvadtsPayloadRaw));
            payloadRaw->size    = report_example_end - report_example_start;
            payloadRaw->data    = malloc (payloadRaw->size);
            memset (payloadRaw->data, '\0', payloadRaw->size);
            memcpy (payloadRaw->data, report_example_start, payloadRaw->size-1);

            //ESP_LOGI (TAG, "Copied data:\n%s", payloadRaw->data);
#else

    if (init_connection) {
        ESP_LOGI (TAG, "Setting up connection..");
        connection_success  = connectionSetup();
        ESP_LOGI (TAG, "Connection done..? %d", connection_success);
    }

    if (connection_success) {
        if (linkInit()) {
            uint8_t ddcmpListNumber = resetValue ? AUDIT_COLLECTION_LIST:SECURITY_READ_LIST;
            payloadRaw = dataAuditCollection(ddcmpListNumber);
        } else {
            ESP_LOGE(TAG, "Unable to establish link Init");
        }

        // Never close connection
        //bool closed = connectionClose();
    } else {
        ESP_LOGE(TAG, "Unable to complete Connection Setup");
    }

#endif

    return payloadRaw;
}

static bool linkInit(){
    ESP_LOGI(TAG, "LINK INIT");

    if (!startMsg()) return false;

    return wayMsg();
}

static bool startMsg(){
    bool result;
    ESP_LOGI(TAG, "startMsg");

    SerialDataMsg ddcmpStartData = {
            .bytes = DDCMP_START_FRAME_MSG,
            .length = 8
    };

    evadtsSerial_write(ddcmpStartData);
    vTaskDelay(250 / portTICK_PERIOD_MS);

    SerialDataMsg* msgFrame= evadtsSerial_read();

    if (msgFrame != NULL) {
        StackMessage *stackMessage = stackMessage_build((uint8_t*)msgFrame->bytes, msgFrame->length);

        int start_attempts = 0;
        while (stackMessage == NULL && start_attempts < DDCMP_START_ATTEMPTS_MAX) {
            evadtsSerial_freeSerialData(msgFrame);
            vTaskDelay(250 / portTICK_PERIOD_MS);
            msgFrame= evadtsSerial_read();
            if (msgFrame != NULL) stackMessage= stackMessage_build((uint8_t*)msgFrame->bytes, msgFrame->length);
            start_attempts++;
        }

        result = stackMessage != NULL;
        stackMessage_destroy(stackMessage);

        evadtsSerial_freeSerialData(msgFrame);
    } else {
        ESP_LOGI(TAG, "msgFrame null");
        result = false;
    }

    return result;
}

static bool wayMsg() {
    bool result;

    SerialDataMsg ddcmpStartData = {
            .bytes = DDCMP_WH0_ARE_YOU_FRAME_MSG,
            .length = 26
    };

    evadtsSerial_write(ddcmpStartData);
    vTaskDelay(250 / portTICK_PERIOD_MS);

    SerialDataMsg* msgFrame= evadtsSerial_read();

    if (msgFrame != NULL) {
        AckMessage* expectedAckMsg= ackMessage_buildFromRxTx(1, 0);
        AckMessage* ackRespMsg = ackMessage_build((uint8_t*)msgFrame->bytes, msgFrame->length);

        if (ackRespMsg != NULL && ddcmpMessage_compareMsg(ackRespMsg->bytes, ackRespMsg->len, expectedAckMsg->bytes, expectedAckMsg->len, ACK_MESSAGE_SIZE) && msgFrame->length >= 16) {
            ackMessage_destroy(ackRespMsg);

            DataHeaderMessage *dataHeaderMessage = dataHeaderMessage_build((uint8_t*)&(msgFrame->bytes[8]), msgFrame->length - 8);

            if (dataHeaderMessage != NULL) {
                WayMessage *wayMessage = wayMessage_build(dataHeaderMessage->getBlockLength(dataHeaderMessage),
                        (uint8_t*)&(msgFrame->bytes[16]),
                        msgFrame->length - 16);

                result = wayMessage != NULL;

                dataHeaderMessage_destroy(dataHeaderMessage);
                wayMessage_destroy(wayMessage);
                evadtsSerial_freeSerialData(msgFrame);

            } else {
                result = false;
            }
        } else {
            evadtsSerial_freeSerialData(msgFrame);
            result = false;
        }

        ackMessage_destroy(expectedAckMsg);
    } else {
        result = false;
    }


    return result;
}

int indexOf(const uint8_t array[], int len, uint8_t value) {
    int indexFound = -1;
    for (int i = 0; i < len; i++) {
        if (array[i] == value) {
            indexFound = i;
            i = len;
        }
    }

    return indexFound;
}

static EvadtsPayloadRaw* dataAuditCollection(uint8_t ddcmpListNumber){
    int rxCount = 1;

    DataReadMessage *dataReadMessageResponse = readInitData(ddcmpListNumber);

    if (dataReadMessageResponse == NULL || !dataReadMessageResponse->isAccepted(dataReadMessageResponse, ddcmpListNumber)) {
        dataReadMessage_destroy(dataReadMessageResponse);
        return NULL;
    }

    EvadtsPayloadRaw* payload= NULL;

    if (dataReadMessageResponse->isDataFileLengthUnknown(dataReadMessageResponse)) {
        payload = buildPayload(&rxCount, endConditionIsLastBlock);
    } else {
        sPayloadRemainLength = dataReadMessageResponse->getDataFileLength(dataReadMessageResponse);
        payload = buildPayload(&rxCount, endConditionRemainLength);
    }

    dataReadMessage_destroy(dataReadMessageResponse);
    AckMessage* nextAckMsg= ackMessage_buildFromRxTx(++rxCount, 0);
    DdcmpMessage* finishMessage  = ddcmpMessage_buildFinishMessage(nextAckMsg->bytes, rxCount, 3);

    SerialDataMsg finishMessageSerialData = {
            .bytes = finishMessage->bytes,
            .length = finishMessage->len
    };

    evadtsSerial_write(finishMessageSerialData);

    rxCount= 3;
    vTaskDelay(250 / portTICK_PERIOD_MS);

    ackMessage_destroy(nextAckMsg);
    ddcmpMessage_destroy(finishMessage);

    SerialDataMsg* msgFrame = evadtsSerial_read();

    if (msgFrame != NULL) {

        int indexDDCMP_CM = indexOf(msgFrame->bytes, msgFrame->length, DDCMP_CM);

        if (indexDDCMP_CM >= 0) {
            AckMessage *expectedAckMessage = ackMessage_buildFromRxTx(rxCount, 0);

            if (expectedAckMessage == NULL ||
                !ddcmpMessage_compareMsg(&msgFrame->bytes[indexDDCMP_CM], msgFrame->length - indexDDCMP_CM, (uint8_t *) expectedAckMessage->bytes,
                                         expectedAckMessage->len, ACK_MESSAGE_SIZE)) {
                payload = NULL;
            }

            ackMessage_destroy(expectedAckMessage);
        } else {
            payload = NULL;
        }
    } else {
        payload = NULL;
    }

    evadtsSerial_freeSerialData(msgFrame);

    return payload;
}


static DataReadMessage *readInitData(uint8_t ddcmpListNumber) {
    ESP_LOGI(TAG, "readInitData");

    DdcmpMessage* ddcmpStartMessage = ddcmpMessage_buildReadDataMessage(ddcmpListNumber);
    if( ddcmpStartMessage == NULL) return NULL;

    DataReadMessage *dataReadMessageResponse = NULL;

    SerialDataMsg ddcmpStartData = {
            .bytes = ddcmpStartMessage->bytes,
            .length =ddcmpStartMessage->len
    };

    evadtsSerial_write(ddcmpStartData);
    ddcmpMessage_destroy(ddcmpStartMessage);

    vTaskDelay(2500 / portTICK_PERIOD_MS);

    int rxCount = 1;
    int readInitAttempts = 0;

    SerialDataMsg *msgFrame = evadtsSerial_read();
    while (msgFrame == NULL || (msgFrame->length < 8 && readInitAttempts < DDCMP_ATTEMPTS_MAX)) {
        vTaskDelay(250 / portTICK_PERIOD_MS);
        evadtsSerial_freeSerialData(msgFrame);
        msgFrame = evadtsSerial_read();
        readInitAttempts++;
    }

    if (msgFrame != NULL && msgFrame->length >= 8) {

        AckMessage *expectedAckMsg = ackMessage_buildFromRxTx(++rxCount, 0);
        AckMessage *ackRespMsg = ackMessage_build((uint8_t *) msgFrame->bytes, msgFrame->length);

        if (ackRespMsg != NULL && ddcmpMessage_compareMsg(ackRespMsg->bytes, ackRespMsg->len, expectedAckMsg->bytes, expectedAckMsg->len, ACK_MESSAGE_SIZE) &&
            msgFrame->length >= 16) {
            ackMessage_destroy(ackRespMsg);

            DataHeaderMessage *dataHeaderMessage = dataHeaderMessage_build((uint8_t *) &(msgFrame->bytes[8]),
                                                                           msgFrame->length - 8);

            if (dataHeaderMessage != NULL) {
                dataReadMessageResponse = dataReadMessage_build(
                        dataHeaderMessage->getBlockLength(dataHeaderMessage),
                        (uint8_t *) &(msgFrame->bytes[16]),
                        msgFrame->length - 16);

                dataHeaderMessage_destroy(dataHeaderMessage);
            }
        } else {
            dataReadMessageResponse = NULL;
        }

        ackMessage_destroy(expectedAckMsg);
    } else {
        dataReadMessageResponse= NULL;
    }

    evadtsSerial_freeSerialData(msgFrame);

    return dataReadMessageResponse;
}

static EvadtsPayloadRaw* buildPayload(int* rx, bool(*endCondition)(DataHeaderMessage *)) {
    bool isEnd = false;

    EvadtsPayloadRaw *payloadRaw = NULL;
    char* payload= NULL;
    int payloadLen= 0;
    int dataBlockNumber= -1;
    SerialDataMsg *prevAuditPartPayload =NULL;

    while(!isEnd) {
        SerialDataMsg *auditPartPayload = NULL;

        if (prevAuditPartPayload == NULL) {
            AckMessage *nextAckMessage = ackMessage_buildFromRxTx(++(*rx), 0);

            SerialDataMsg nextAckSerialData = {
                    .bytes = nextAckMessage->bytes,
                    .length = ACK_MESSAGE_SIZE
            };

            evadtsSerial_write(nextAckSerialData);
            vTaskDelay(250 / portTICK_PERIOD_MS);
            ackMessage_destroy(nextAckMessage);

            int readInitAttempts = 0;
            auditPartPayload = evadtsSerial_read();

            while (auditPartPayload->length < 8 && readInitAttempts < DDCMP_ATTEMPTS_MAX) {
                vTaskDelay(250 / portTICK_PERIOD_MS);
                evadtsSerial_freeSerialData(auditPartPayload);
                auditPartPayload = evadtsSerial_read();
                readInitAttempts++;
            }
        } else {
            auditPartPayload = prevAuditPartPayload;
        }

        if (auditPartPayload->length < DDCMP_DEFAULT_MSG_SIZE) {
            isEnd = true;
        } else {
            DataHeaderMessage *dataHeaderMessage = dataHeaderMessage_build((uint8_t *) auditPartPayload->bytes,
                                                                           auditPartPayload->length);

            if (dataHeaderMessage != NULL) {
                DataBlockMessage *dataBlockMessage = dataBlockMessage_build(
                        dataHeaderMessage->getBlockLength(dataHeaderMessage),
                        auditPartPayload->length - DDCMP_DEFAULT_MSG_SIZE,
                        (uint8_t *) &(auditPartPayload->bytes[DDCMP_DEFAULT_MSG_SIZE]));

                if (dataBlockMessage != NULL) {
                    if (dataBlockNumber < dataBlockMessage->getBlockNumber(dataBlockMessage)) {
                        dataBlockNumber = dataBlockMessage->getBlockNumber(dataBlockMessage);

                        int currentPayloadLen = dataBlockMessage->getPayloadLength(dataBlockMessage);
                        int currentPayloadOffset = payloadLen;
                        char *newPayload = (char *) realloc(payload, (payloadLen + currentPayloadLen));
                        if (newPayload != NULL) {
                            payloadLen += currentPayloadLen;
                            payload = newPayload;
                            memcpy(&payload[currentPayloadOffset], dataBlockMessage->getPayload(dataBlockMessage),
                                   currentPayloadLen * sizeof(uint8_t));
                            isEnd = endCondition(dataHeaderMessage);
                        } else {
                            free(payload);
                            payload = NULL;
                            isEnd = true;
                        }
                    }

                    int firstBlockSize = dataHeaderMessage->data->len + dataBlockMessage->data->len;

                    if ((auditPartPayload->length - firstBlockSize) > DDCMP_DEFAULT_MSG_SIZE) {
                        evadtsSerial_freeSerialData(prevAuditPartPayload);
                        int payloadRemainLength = auditPartPayload->length - firstBlockSize;
                        ESP_LOGW("evaRetriever", "line %d ", payloadRemainLength);

                        prevAuditPartPayload = (SerialDataMsg *) malloc(sizeof(SerialDataMsg));
                        prevAuditPartPayload->bytes = malloc(payloadRemainLength * sizeof(uint8_t));
                        prevAuditPartPayload->length = payloadRemainLength;
                        memcpy((void *) prevAuditPartPayload->bytes, &(auditPartPayload->bytes[firstBlockSize]),
                               payloadRemainLength * sizeof(uint8_t));

                    } else {
                        prevAuditPartPayload = NULL;
                    }

                    dataBlockMessage_destroy(dataBlockMessage);
                }
                dataHeaderMessage_destroy(dataHeaderMessage);
            } else {
                isEnd = true;
            }

            evadtsSerial_freeSerialData(auditPartPayload);
        }
    }

    if (payload != NULL) {
        payloadRaw = malloc(sizeof(EvadtsPayloadRaw));

        if (payloadRaw != NULL) {
            payloadRaw->data = malloc(payloadLen + 1);
            if (payloadRaw->data != NULL) {
                memcpy(payloadRaw->data, payload, payloadLen);
                payloadRaw->size = payloadLen;
            } else {
                free(payloadRaw);
            }
        } else {
            payload = NULL;
        }

        free(payload);
    }

    evadtsSerial_freeSerialData(prevAuditPartPayload);
    return payloadRaw;
}