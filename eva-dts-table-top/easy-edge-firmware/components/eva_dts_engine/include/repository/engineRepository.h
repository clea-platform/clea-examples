//
// Created by harlem88 on 17/12/20.
//

#ifndef EVA_DTS_ENGINE_MICRO_ENGINEREPOSITORY_H
#define EVA_DTS_ENGINE_MICRO_ENGINEREPOSITORY_H

#include <esp_err.h>

typedef struct _engineRepositoryData EngineRepositoryData;

typedef struct _engineRepository {
    EngineRepositoryData* data;
    esp_err_t (*setI32)(struct _engineRepository *this, const char* key, int32_t value);
    esp_err_t (*getI32)(struct _engineRepository *this, const char* key, int32_t* out_value);
}EngineRepository;

EngineRepository* engineRepository_init();
void engineRepository_destroy(EngineRepository*);
#endif //EVA_DTS_ENGINE_MICRO_ENGINEREPOSITORY_H
