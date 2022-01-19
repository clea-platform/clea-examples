
#pragma once


#include <QtCore/QObject>
#include <QtCore/QSettings>
#include <QtCore/QThread>
#include <QtCore/QJsonObject>

#include <hemeraoperation.h>
#include <AstarteDeviceSDK.h>

#include <polygon.hpp>
#include <tracker.hpp>
#include <detector.hpp>
#include <utils/images_capture.h>

#include <memory>
#include <thread>
#include <future>


using Scene = std::vector<Polygon>;


class PeopleCounter : public QObject {
    Q_OBJECT

public:
    PeopleCounter(QSettings &settings, std::unique_ptr<ImagesCapture> &img_source,
                    ObjectDetector &detector, std::unique_ptr<PedestrianTracker> &tracker,
                    QObject *parent = nullptr);
    ~PeopleCounter();

    void stop ();
    void wait_for_completion ();

private slots:
    void check_init_result(Hemera::Operation *op);
    void send_values();

    void handleIncomingData(const QByteArray &interface, const QByteArray &path, const QVariant &value);

    void people_counter_function ();

private:
    void start_computation ();

    void load_scene (QJsonObject &json_scene);
    Scene m_scene;
    
    std::atomic_bool m_still_continue;
    QThread m_people_counter_thread;

    // Settings object
    QSettings &m_settings;
    AstarteDeviceSDK *m_astarte_sdk;
    
    QTimer *m_publish_timer;

    //QByteArray m_interface;
    std::unique_ptr<ImagesCapture> &m_img_source;
    ObjectDetector &m_detector;
    std::unique_ptr<PedestrianTracker> &m_tracker;
};
