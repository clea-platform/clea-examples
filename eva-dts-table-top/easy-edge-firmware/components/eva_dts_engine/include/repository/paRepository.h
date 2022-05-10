//
// Created by harlem88 on 17/12/20.
//

#ifndef EVA_DTS_ENGINE_MICRO_PAREPOSITORY_H
#define EVA_DTS_ENGINE_MICRO_PAREPOSITORY_H

#include "repository/engineRepository.h"
#include "sensor/paSensor.h"
#include <esp_err.h>

#define PA_FREE_KEY "free"
#define PA_PAID_KEY "paid"
#define PA_CASHED_KEY "cashed"

esp_err_t paRepository_set(EngineRepository* repository, PASensor*);
PASensor* paRepository_get(EngineRepository* repository,const char*);
esp_err_t paRepository_update(EngineRepository* repository, PASensor*);
esp_err_t paRepository_remove(EngineRepository* repository, PASensor*);

#endif //EVA_DTS_ENGINE_MICRO_PAREPOSITORY_H
