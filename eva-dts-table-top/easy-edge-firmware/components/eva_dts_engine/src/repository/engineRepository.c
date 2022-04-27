//
// Created by harlem88 on 17/12/20.
//
#include "repository/engineRepository.h"
#include <nvs.h>
#include <esp_log.h>

#define ENGINE_STORAGE_NAME "evadtsEngNVS"
static const char* TAG = "Engine_Repository";

struct _engineRepositoryData {
    nvs_handle_t handle;
};

static esp_err_t setI32(struct _engineRepository *this, const char *key, int32_t value);
static esp_err_t getI32(struct _engineRepository *this, const char* key, int32_t* out_value);

EngineRepository* engineRepository_init(){
    EngineRepository* engineRepository = malloc(sizeof(EngineRepository));
    if(engineRepository == NULL) return NULL;

    engineRepository->data = malloc(sizeof(struct _engineRepositoryData));

    if(engineRepository->data == NULL){
        engineRepository_destroy(engineRepository);
        return NULL;
    }

    engineRepository->setI32=& setI32;
    engineRepository->getI32=& getI32;

    esp_err_t error = nvs_open(ENGINE_STORAGE_NAME, NVS_READWRITE, &engineRepository->data->handle);

    if (error != ESP_OK) {
        if (error == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(TAG, "No data in NVS\n");
        } else {
            ESP_LOGE(TAG, "Error init NVS: %s\n", esp_err_to_name(error));
            engineRepository_destroy(engineRepository);
            engineRepository = NULL;
        }
    }

    return engineRepository;
}

static esp_err_t setI32(struct _engineRepository *this, const char *key, int32_t value) {
    esp_err_t err;

    if (this != NULL && this->data != NULL){
        err = nvs_set_i32(this->data->handle, key, value);
    } else{
        err = ESP_FAIL;
    }

    return err;
}

static esp_err_t getI32(struct _engineRepository *this, const char* key, int32_t* out_value){
    esp_err_t err;

    if (this != NULL && this->data != NULL){
        err = nvs_get_i32(this->data->handle, key, out_value);
    } else{
        err = ESP_FAIL;
    }

    return err;
}

void engineRepository_destroy(EngineRepository *repository) {
    if (repository != NULL) {

        if (repository->data != NULL) {
            nvs_close(repository->data->handle);
            free(repository->data);
        }

        free(repository);
    }
}