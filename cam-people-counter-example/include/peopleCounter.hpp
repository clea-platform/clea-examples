
#pragma once


#include <QtCore/QObject>

#include <hemeraoperation.h>
#include <AstarteDeviceSDK.h>

#include <tracker.hpp>
#include <detector.hpp>
#include <utils/images_capture.h>

#include <memory>
#include <thread>



class PeopleCounter : public QObject
{
    Q_OBJECT

public:
    PeopleCounter(const QString &settings_file_path, std::unique_ptr<ImagesCapture> &img_source,
                    ObjectDetector &detector, std::unique_ptr<PedestrianTracker> &tracker,
                    QObject *parent = nullptr);
    ~PeopleCounter();

    void stop ();
    void wait_for_completion ();

private slots:
    void checkInitResult(Hemera::Operation *op);
    void sendValues();

    void handleIncomingData(const QByteArray &interface, const QByteArray &path, const QVariant &value);

private:
    void start_computation ();
    
    std::atomic_bool m_still_continue;
    std::thread m_people_counter_thread;
    void people_counter_function ();

    AstarteDeviceSDK *m_astarte_sdk;
    QString m_settings_file_path;
    QTimer *m_publish_timer;
    QByteArray m_interface;
    std::unique_ptr<ImagesCapture> &m_img_source;
    ObjectDetector &m_detector;
    std::unique_ptr<PedestrianTracker> &m_tracker;
    bool m_ready;
};
