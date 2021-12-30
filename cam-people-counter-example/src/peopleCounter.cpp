
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

PeopleCounter::PeopleCounter (const QString &settings_file_path, std::unique_ptr<ImagesCapture> &img_source,
                                ObjectDetector &detector, std::unique_ptr<PedestrianTracker> &tracker,
                                QObject *parent)
                                : QObject(parent), m_settings_file_path(settings_file_path), m_img_source(img_source),
                                m_detector(detector), m_tracker(tracker), m_publish_timer(new QTimer(this)),
                                m_ready(false), m_still_continue(true) {
    
    // TODO Loading settings file

    // TODO Setting up "m_publish_timer"
    //m_publish_timer->setInterval();
    connect(m_publish_timer, &QTimer::timeout, this, &PeopleCounter::sendValues);

    // people_counter_thread will be built in 'checkInitResults' method

    // TODO Building astarte SDK
    //m_astarte_sdk = new AstarteDeviceSDK(QDir::currentPath() + QStringLiteral("/astarte-device-%1-conf/transport-astarte.conf").arg(device), QDir::currentPath() + QStringLiteral("/interfaces"), device.toLatin1());
    //QString interfaces_dir  = "";   // TODO
    /*m_astarte_sdk = new AstarteDeviceSDK(settings_file_path, QDir::currentPath() + QStringLiteral("/interfaces"), device.toLatin1());
    connect(m_astarte_sdk->init(), &Hemera::Operation::finished, this, &PeopleCounter::checkInitResult);
    connect(m_astarte_sdk, &AstarteDeviceSDK::dataReceived, this, &PeopleCounter::handleIncomingData);*/

    // FIXME Remove me: test purposes
    checkInitResult (nullptr);
}




PeopleCounter::~PeopleCounter() {
    // TODO
}




void PeopleCounter::start_computation () {
    m_ready                 = true;
    // Building people_counter thread
    m_people_counter_thread = std::thread (&PeopleCounter::people_counter_function, this);
    m_publish_timer->start();
}




void PeopleCounter::stop () {
    m_publish_timer->stop ();
    m_still_continue    = false;
}




void PeopleCounter::wait_for_completion () {
    m_people_counter_thread.join ();
}




void PeopleCounter::checkInitResult(Hemera::Operation *op) {
    
    // FIXME Remove me: test purposes
    if (op == nullptr) {
        start_computation ();
        return ;
    }

    std::cout << "Checking init result..\n";
    if (op->isError()) {
        qWarning() << "PeopleCounter init error: " << op->errorName() << op->errorMessage();
    } else {
        qWarning() << "Starting..\n";
        start_computation ();
    }
}




void PeopleCounter::sendValues() {
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


    if (!frame.data)
        throw std::runtime_error("Can't read an image from the input");
    cv::Size firstFrameSize = frame.size();
    
    if (video_fps == 0.0) {
        video_fps   = 60.0;
    }

    /*cv::VideoWriter videoWriter;
    if (!FLAGS_o.empty() && !videoWriter.open(FLAGS_o, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
                                            cap->fps(), firstFrameSize)) {
        throw std::runtime_error("Can't open video writer");
    }*/
    cv::Size graphSize{static_cast<int>(frame.cols / 4), 60};
    Presenter presenter("", 10, graphSize);

    std::cout << "\n\nTo close the application, press 'CTRL+C' here or switch to the output window " \
                    "and press ESC key or Q key\n\n";

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

            // Drawing tracked detections only by RED color and print ID and detection
            // confidence level.
            auto detected_objects   = m_tracker->TrackedDetections();
            //std::cout << "Seen " << detected_objects.size() << " objects\n";
            for (const auto &detection : detected_objects) {
                /*std::cout << "Object " << detection.object_id << "->\n\tconf: " << detection.confidence << "\n\tframe: " << detection.frame_idx << "\n\ttime:" << detection.timestamp <<
                                "\n\trect.x: " << detection.rect.x << "\n\trect.y: " << detection.rect.y << "\n\trect.width: " << detection.rect.width << "\n\theght: " << detection.rect.height << std::endl;*/
                cv::rectangle(frame, detection.rect, cv::Scalar(0, 0, 255), 3);
                std::string text = std::to_string(detection.object_id) +
                    " conf: " + std::to_string(detection.confidence);
                cv::putText(frame, text, detection.rect.tl(), cv::FONT_HERSHEY_COMPLEX,
                            1.0, cv::Scalar(0, 0, 255), 3);
            }

            frames_processed++;
            /*if (videoWriter.isOpened() && (FLAGS_limit == 0 || frames_processed <= FLAGS_limit)) {
                videoWriter.write(frame);
            }*/
            
            cv::imshow("dbg", frame);
            char k  = cv::waitKey(5);
            if (k == ESC_KEY || k == Q_KEY)
                break;

            /*if (should_save_det_log && (frame_idx % 100 == 0)) {
                DetectionLog log = tracker->GetDetectionLog(true);
                SaveDetectionLogToTrajFile(detlog_out, log);
            }*/
            
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
        
        
        /*if (should_keep_tracking_info) {
            DetectionLog log = tracker->GetDetectionLog(true);

            if (should_save_det_log)
                SaveDetectionLogToTrajFile(detlog_out, log);
            if (should_print_out)
                PrintDetectionLog(log);
        }
        if (should_use_perf_counter) {
            m_detector.PrintPerformanceCounts(getFullDeviceName(ie, FLAGS_d_det));
            tracker->PrintReidPerformanceCounts(getFullDeviceName(ie, FLAGS_d_reid));
        }*/
    }
    
    std::cout << presenter.reportMeans() << '\n';
}