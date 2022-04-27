//
// Created by harlem88 on 17/12/20.
//

#ifndef EVA_DTS_ENGINE_MICRO_SAREPOSITORY_H
#define EVA_DTS_ENGINE_MICRO_SAREPOSITORY_H

#include "repository/engineRepository.h"
#include "sensor/saSensor.h"
#include <esp_err.h>

esp_err_t saRepository_set(EngineRepository* repository, SASensor*);
SASensor* saRepository_get(EngineRepository* repository, SASensor*);
esp_err_t saRepository_update(EngineRepository* repository, SASensor*);
esp_err_t saRepository_remove(EngineRepository* repository, SASensor*);

#endif //EVA_DTS_ENGINE_MICRO_SAREPOSITORY_H
