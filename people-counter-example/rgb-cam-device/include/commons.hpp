
#pragma once


#include <QString>
#include <QList>


typedef struct {
    uint person_id;
    double confidence;
    uint zone_id;
    QString zone_name;
} DetectedPerson;

typedef struct {
    uint ms_timestamp;
    QList<DetectedPerson> detections;
} Detections;