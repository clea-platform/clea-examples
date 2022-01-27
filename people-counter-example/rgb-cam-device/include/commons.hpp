
#pragma once


#include <QString>
#include <QList>

#include <polygon.hpp>


using Scene = std::vector<Polygon>;


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