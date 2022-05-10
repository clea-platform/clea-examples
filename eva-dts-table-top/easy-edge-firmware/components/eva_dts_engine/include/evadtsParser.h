//
// Created by harlem88 on 09/10/20.
//

#ifndef EVADTS_PARSER_H
#define EVADTS_PARSER_H

#include "data/evadtsDataBlockList.h"
#include "data/evadtsPayloadRaw.h"

EvadtsDataBlockList* evadtsParser_parse(EvadtsPayloadRaw *);

#endif //EVADTS_PARSER_H
