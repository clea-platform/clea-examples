//
// Created by harlem88 on 06/10/20.
//

#ifndef EVADTS_RETRIEVER_H
#define EVADTS_RETRIEVER_H

#include <stdbool.h>
#include "data/evadtsPayloadRaw.h"

EvadtsPayloadRaw* evadtsRetriever_readDataCollection(bool init_connection, bool reset_values);


#endif //   EVADTS_RETRIEVER_H
