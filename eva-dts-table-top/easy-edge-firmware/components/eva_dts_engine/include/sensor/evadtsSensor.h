//
// Created by harlem88 on 11/12/20.
//

#ifndef EVA_DTS_ENGINE_MICRO_EVADTSSENSOR_H
#define EVA_DTS_ENGINE_MICRO_EVADTSSENSOR_H

typedef union _evaDtsValue{
    float fValue;
    char* sValue;
} EvaDtsValue;

typedef enum _valueType {
    VALUE_TYPE_U8 = 0x00,
    VALUE_TYPE_U16 = 0x01,
    VALUE_TYPE_U32 = 0x02,
    VALUE_TYPE_FLOAT = 0x03,
    VALUE_TYPE_ASCII = 0x04
} ValueType;

typedef struct _evaDtsSensor {
    char id[51];
    char map[51];
    ValueType valueType;
    EvaDtsValue value;
    float price;
} EvaDtsSensor;

#endif //EVA_DTS_ENGINE_MICRO_EVADTSSENSOR_H
