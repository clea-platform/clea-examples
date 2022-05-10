//
// Created by harlem88 on 28/12/20.
//
#include "data/evadtsPayloadRaw.h"
#include <malloc.h>

void evadtsPayloadRaw_destroy(EvadtsPayloadRaw *this) {
    if (this != NULL) {
        if (this->data != NULL) {
            free(this->data);
        }
        free(this);
    }
}
