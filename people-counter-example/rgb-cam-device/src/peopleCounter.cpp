
#include "peopleCounter.hpp"

#include <AstarteDeviceSDK.h>

#include <HemeraCore/Operation>
#include <monitors/presenter.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QTimer>

#include <climits>
#include <iostream>
#include <math.h>

#define ESC_KEY 27
#define Q_KEY   113

PeopleCounter::PeopleCounter (QSettings &settings, std::unique_ptr<ImagesCapture> &img_source,
                                ObjectDetector &detector, std::unique_ptr<PedestrianTracker> &tracker,
                                QObject *parent)
                                : QObject(parent), m_still_continue(true), m_settings (settings),
                                m_astarte_sdk(nullptr), m_publish_timer(new QTimer(this)),
                                m_img_source(img_source), m_detector(detector), m_tracker(tracker) {
    // Setting up "m_publish_timer"
    m_publish_timer->setInterval(m_settings.value ("DeviceSettings/publishInterval").toInt());
    connect(m_publish_timer, &QTimer::timeout, this, &PeopleCounter::send_values);

    // Building "m_people_counter_thread"
    connect (&m_people_counter_thread, &QThread::started, this, &PeopleCounter::people_counter_function);
    
    // Building astarte SDK
    //                                  path to config file
    m_astarte_sdk = new AstarteDeviceSDK(settings.fileName(),
                                            m_settings.value ("DeviceSettings/interfacesDirectory").toString(),
                                            m_settings.value ("DeviceSettings/deviceID").toByteArray(),
                                            parent);
    connect(m_astarte_sdk->init(), &Hemera::Operation::finished, this, &PeopleCounter::check_init_result);
    connect(m_astarte_sdk, &AstarteDeviceSDK::dataReceived, this, &PeopleCounter::handleIncomingData);
}




PeopleCounter::~PeopleCounter() {
    stop ();
    /*if (!m_initialized_future.get()) {
        qCritical() << "\n\nPeopleCounterobject not succesfully initialized!";
    }
    if (m_people_counter_thread.joinable())
        m_people_counter_thread.join ();*/
    m_people_counter_thread.wait();
}




void PeopleCounter::start_computation () {
    m_people_counter_thread.start();
    m_publish_timer->start();
    //m_initialized_promise.set_value (true);
}




void PeopleCounter::stop () {
    /*try {
        m_initialized_promise.set_value(false);     // FIXME Handle concurrency and check if is already set
        //qWarning() << "\nUninitialized object!";
    } catch (std::future_error &e) {
        //qWarning() << "\nCurrent object is already initialized";
    }*/
    m_still_continue    = false;
    m_publish_timer->stop ();
    
    if (m_people_counter_thread.isRunning())
        m_people_counter_thread.exit();
}




void PeopleCounter::wait_for_completion () {
    /*if (!m_initialized_future.valid() ||
        (m_initialized_future.valid() && !m_initialized_future.get())) {
        return ;
    }*/

    /*qDebug() << "m_people_counter_thread.isRunning() : " << m_people_counter_thread.isRunning() << "\n" <<
                    "m_people_counter_thread.isFinished() : " << m_people_counter_thread.isFinished();*/
    m_people_counter_thread.wait();
}




void PeopleCounter::check_init_result(Hemera::Operation *op) {
    
    qDebug() << "Checking init result..\n";
    qDebug() << "Timer started: " << m_publish_timer->isActive();
    if (op->isError()) {
        qWarning() << "PeopleCounter init error: " << op->errorName() << op->errorMessage();
        //m_initialized_promise.set_value (false);
    } else {
        start_computation ();
    }
}




void PeopleCounter::send_values() {
    // TODO
    std::cout << "Sending values..\n";
    return ;
}




void PeopleCounter::handleIncomingData(const QByteArray &interface, const QByteArray &path, const QVariant &value) {
    // TODO
    qDebug() << "Received data, interface: " << interface << "path: " << path << ", value: " << value << ", Qt type name: " << value.typeName();
}




void PeopleCounter::people_counter_function () {
    uint32_t frame_idx          = 0;
    uint32_t frames_processed   = 0;
    cv::Mat frame               = m_img_source->read();
    double video_fps            = m_img_source->fps();
    TrackedObjects detections;


    // Waiting for startup completion
    /*m_initialized_future.wait();
    if (!m_initialized_future.valid() ||
        (m_initialized_future.valid() && !m_initialized_future.get())) {
        return ;
    }*/


    if (!frame.data)
        throw std::runtime_error("Can't read an image from the input");
    cv::Size firstFrameSize = frame.size();
    
    if (video_fps == 0.0) {
        video_fps   = 60.0;
    }

    cv::Size graphSize{static_cast<int>(frame.cols / 4), 60};
    Presenter presenter("", 10, graphSize);

    std::cout << "\n\nTo close the application, press 'CTRL+C'\n\n";

    while (m_still_continue) {
        ++frame_idx;
        
        try {
            m_detector.submitFrame(frame, frame_idx);
            m_detector.waitAndFetchResults();

            detections   = m_detector.getResults();

            // timestamp in milliseconds
            uint64_t cur_timestamp  = static_cast<uint64_t>(1000.0 / video_fps * frame_idx);
            m_tracker->Process(frame, detections, cur_timestamp);

            presenter.drawGraphs(frame);
            // Drawing colored "worms" (tracks).
            frame   = m_tracker->DrawActiveTracks(frame);

            // Drawing all detected objects on a frame by BLUE COLOR
            for (const auto &detection : detections) {
                cv::rectangle(frame, detection.rect, cv::Scalar(255, 0, 0), 3);
            }

            // Drawing tracked detections only by RED color and print ID and detection confidence level.
            auto detected_objects   = m_tracker->TrackedDetections();
            //std::cout << "Seen " << detected_objects.size() << " objects\n";
            for (const auto &detection : detected_objects) {
                std::cout << "Object " << detection.object_id << "->\n\tconf: " << detection.confidence << "\n\tframe: " << detection.frame_idx << "\n\ttime:" << detection.timestamp <<
                                "\n\trect.x: " << detection.rect.x << "\n\trect.y: " << detection.rect.y << "\n\trect.width: " << detection.rect.width << "\n\theght: " << detection.rect.height << std::endl;
                cv::rectangle(frame, detection.rect, cv::Scalar(0, 0, 255), 3);
                std::string text = std::to_string(detection.object_id) +
                    " conf: " + std::to_string(detection.confidence);
                cv::putText(frame, text, detection.rect.tl(), cv::FONT_HERSHEY_COMPLEX,
                            1.0, cv::Scalar(0, 0, 255), 3);
            }

            frames_processed++;
            
            cv::imshow("dbg", frame);
            char k  = cv::waitKey(5);
            if (k == ESC_KEY || k == Q_KEY)
                break;
            
            frame   = m_img_source->read();
            if (!frame.data)
                break;
            if (frame.size() != firstFrameSize)
                throw std::runtime_error("Can't track objects on images of different size");
        }
        catch (std::runtime_error &e) {
            qCritical() << "Catched this error in people counter execution:\n\t" << e.what() << "\n";
        }
        catch (std::exception &e) {
            qCritical() << "Catched this exception in people counter execution:\n\t" << e.what() << "\n";
        }
        catch (...) {
            qCritical() << "Catched an unexpected error in people counter execution\n";
        }
    }

    std::cout << presenter.reportMeans() << '\n';
}