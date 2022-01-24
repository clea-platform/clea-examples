
#include "peopleCounter.hpp"

#include <AstarteDeviceSDK.h>

#include <HemeraCore/Operation>
#include <monitors/presenter.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtCore/QList>
#include <QtCore/QVariantHash>
#include <QtCore/QJsonDocument>

#include <climits>
#include <iostream>
#include <algorithm>
#include <math.h>

#define ESC_KEY 27
#define Q_KEY   113

PeopleCounter::PeopleCounter (QSettings &settings, std::unique_ptr<ImagesCapture> &img_source,
                                ObjectDetector &detector, std::unique_ptr<PedestrianTracker> &tracker,
                                QObject *parent)
                                : QObject(parent), m_still_continue(true), m_settings (settings),
                                m_astarte_sdk(nullptr), m_publish_timer(new QTimer(this)),
                                m_img_source(img_source), m_detector(detector), m_tracker(tracker) {
    
    // Setting up scene settings
    QFile scene_settings_file (m_settings.value ("AppSettings/sceneSettingsFile").toString());
    if (!scene_settings_file.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open scene settings file " << m_settings.value ("AppSettings/sceneSettingsFile").toString();
        throw std::runtime_error ("Cannot open scene settings file!");
    }
        // Reading the file and close it.
	QByteArray json_data = scene_settings_file.readAll();
	scene_settings_file.close();
	QJsonParseError parse_error;
	QJsonDocument json_document = QJsonDocument::fromJson(json_data, &parse_error);
	    // Failing if file content is invalid
	if (parse_error.error != QJsonParseError::NoError)
	{
		qCritical() << "Failed to parse scene settings file:" << parse_error.errorString()
					 << "at offset " << QString::number(parse_error.offset);
		throw std::runtime_error ("");
	}
        // Making sure the root is an object.
	if (!json_document.isObject())
	{
		qCritical() << "Invalid scene settings JSON: Root should be an array.";
		throw std::runtime_error ("");
	}
	QJsonObject json_scene  = json_document.object();
    load_scene (json_scene);

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
    // FIXME Consider possibility to destroy object before astarte connection
    stop ();
    m_people_counter_thread.wait();
}




void PeopleCounter::load_scene (QJsonObject &json_scene) {
    // Loading scene from "json_scene" object
    QVariantMap map_scene  = json_scene.toVariantMap();

    for (auto it = map_scene.begin(); it != map_scene.end(); ++it) {
        int zone_idx            = it.key().toInt();
        QString zone_name       = it.value().toMap().value("zone_name").toString();
        QVariantList j_vertices = it.value().toMap().value("vertices").toList();
        Polygon polygon (zone_idx, zone_name);
        
        for (auto j_vertex : j_vertices) {
            Point pt;
            pt.x    = j_vertex.toMap().value("x").toDouble();
            pt.y    = j_vertex.toMap().value("y").toDouble();
            polygon.add_vertex (pt);
        }
        
        m_scene.push_back (polygon);
    }
}




void PeopleCounter::start_computation () {
    m_people_counter_thread.start();
    m_publish_timer->start();
}




void PeopleCounter::stop () {
    m_still_continue    = false;
    m_publish_timer->stop ();
    
    if (m_people_counter_thread.isRunning())
        m_people_counter_thread.exit();
}




void PeopleCounter::wait_for_completion () {
    m_people_counter_thread.wait();
}




void PeopleCounter::check_init_result(Hemera::Operation *op) {
    
    qDebug() << "Checking init result..\n";
    if (op->isError()) {
        qWarning() << "PeopleCounter init error: " << op->errorName() << "\n\t" << op->errorMessage();
        throw std::runtime_error ("");
    } else {
        start_computation ();
    }
}




void PeopleCounter::send_values() {
    QVariantHash payload;

    {
        std::unique_lock<std::mutex> detections_lock (m_detections_mutex);

        if (m_detections_list.isEmpty()) {
            qCritical() << "NO FRAME PROCESSED UNTIL LAST SENT!";
            return ;
        }
        else {
            auto last_detection    = m_detections_list.back();
            
            // Building data to be sent to Astarte
            QList<QString> items;
            // Building "items" list with detected people
            payload["/1/reading_timestamp"]    = last_detection.ms_timestamp;
            payload["/1/people_count"]         = last_detection.detections.count();
            for (auto &it : last_detection.detections) {
                // item : {id, conf, pos_zone:{id, name}}
                QJsonObject j_item;
                QJsonObject j_zone;
                j_zone.insert ("id", (int) it.zone_id);
                j_zone.insert ("name", it.zone_name);
                j_item.insert ("pos_zone", j_zone);
                j_item.insert ("id", (int) it.person_id);
                j_item.insert ("conf", it.confidence);
                QString s_item  = QJsonDocument(j_item).toJson(QJsonDocument::Compact);
                items.push_back (s_item);
            }
            payload["/1/people"]    = QVariant(items);

            m_detections_list.clear();
        }
    }
    
    // Actually sending data to Astarte
    bool sent_result    = m_astarte_sdk->sendData (m_settings.value("DeviceSettings/interfaceName").toByteArray(),
                                                    payload, QDateTime::currentDateTime());
    if (!sent_result) {
        qWarning() << "Send operation reports an error!";
    }

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
    TrackedObjects detector_results;


    if (!frame.data)
        throw std::runtime_error("Can't read an image from the input");
    cv::Size first_frame_size   = frame.size();


    // Computing actual vertices of polygons zones depending on frame size
    Scene actual_scene;
    for (auto &s : m_scene) {
        Polygon scaled_polygon (s.get_zone_id(), s.get_zone_name());
        for (auto &pt : s.get_vertices()) {
            Point scaled_point;
            scaled_point.x  = pt.x*first_frame_size.width;
            scaled_point.y  = pt.y*first_frame_size.height;
            scaled_polygon.add_vertex (scaled_point);
        }
        actual_scene.push_back (scaled_polygon);
    }


    /* Finding lines which indentify the zones. Do not add a side if it is already considered
        or it is a image side
     */
    using Segment           = std::pair<Point, Point>;
    auto point_comparator   = [] (Point &a, Point &b) -> bool {
        return (a.x == b.x && a.y == b.y);
    };
    auto segment_comparator = [point_comparator] (Segment &a, Segment &b) -> bool {
        return ((point_comparator(a.first, b.first) && point_comparator(a.second, b.second)) ||
                (point_comparator(a.first, b.second) && point_comparator(a.second, b.first)));
    };
    auto is_on_scene_side   = [] (Segment &a, cv::Size &scene_size) -> bool {
        return ((a.first.x == 0 && a.second.x == 0) || (a.first.y == 0 && a.second.y == 0) ||
                (a.first.x == scene_size.width && a.second.x == scene_size.width) ||
                (a.first.y == scene_size.height && a.second.y == scene_size.height));
    };
    std::vector<Segment> sides_segments;

    // Obtaining only segments that has to be drawn
    for (auto &polygon: actual_scene) {
        auto vertices   = polygon.get_vertices();
        for (auto it=vertices.begin(); it!=vertices.end(); ++it) {
            auto second         = (std::next(it) == vertices.end()) ? vertices.begin() : std::next(it);
            Segment new_segment = std::make_pair<> (*it, *second);
            bool segment_found  = false;
            if (!is_on_scene_side(new_segment, first_frame_size)) {
                for (auto &side : sides_segments) {
                    if (segment_comparator(side, new_segment)) {
                        segment_found   = true;
                        break;
                    }
                }

                if (!segment_found) {
                    sides_segments.push_back(new_segment);
                }
            }
        }
    }
    
    if (video_fps == 0.0) {
        video_fps   = 60.0;
    }

    cv::Size graphSize{static_cast<int>(frame.cols / 4), 60};
    Presenter presenter("", 10, graphSize);

    qInfo() << "\n\nTo close the application, press 'CTRL+C'\n\n";

    while (m_still_continue) {
        ++frame_idx;
        
        try {
            m_detector.submitFrame(frame, frame_idx);
            m_detector.waitAndFetchResults();

            detector_results    = m_detector.getResults();

            // timestamp in milliseconds
            uint64_t cur_timestamp  = static_cast<uint64_t>(1000.0 / video_fps * frame_idx);
            m_tracker->Process(frame, detector_results, cur_timestamp);

            presenter.drawGraphs(frame);
            // Drawing colored "worms" (tracks).
            frame   = m_tracker->DrawActiveTracks(frame);

            // Drawing all detected objects on a frame by BLUE COLOR
            for (const auto &detection : detector_results) {
                cv::rectangle(frame, detection.rect, cv::Scalar(255, 0, 0), 3);
            }

            // Drawing on frame lines which indentify the zones
            for (auto &segment : sides_segments) {
                cv::Point   a (segment.first.x, segment.first.y),
                            b (segment.second.x, segment.second.y);
                cv::line (frame, a, b, cv::Scalar(255, 0, 0), 2);
            }

            // Creating "current_detections" object which contains detected people in current frame
            Detections current_detections;
            current_detections.ms_timestamp = cur_timestamp;

            {
                // Taking mutex for "detections_list" shared object
                std::unique_lock<std::mutex> detections_lock (m_detections_mutex);

                // Drawing tracked detections only by RED color and print ID and detection confidence level.
                auto detected_objects   = m_tracker->TrackedDetections();
                for (const auto &detection : detected_objects) {
                    // Drawing person bounding box, id and confidence on frame
                    cv::rectangle(frame, detection.rect, cv::Scalar(0, 0, 255), 3);
                    std::string text = std::to_string(detection.object_id) +
                        " conf: " + std::to_string(detection.confidence);
                    cv::putText(frame, text, detection.rect.tl(), cv::FONT_HERSHEY_COMPLEX,
                                1.0, cv::Scalar(0, 0, 255), 3);
                    
                    // Finding out person center and the zone which belongs to
                    double x_center = ((double) detection.rect.x + ((double) detection.rect.width/2));
                    double y_center = ((double) detection.rect.y + ((double)detection.rect.height/2));
                    Point person_center = {x_center, y_center};
                    cv::Point cv_point_center (x_center, y_center);
                    cv::circle (frame, cv_point_center, 2, cv::Scalar(0, 0, 255));

                    for (auto &polygon: actual_scene) {
                        if (polygon.contains(person_center)) {
                            DetectedPerson detected_person;
                            detected_person.person_id   = detection.object_id;
                            detected_person.confidence  = detection.confidence;
                            detected_person.zone_id     = polygon.get_zone_id();
                            detected_person.zone_name   = polygon.get_zone_name();
                            // Adding detected person to "current_detections" object
                            current_detections.detections.push_back (detected_person);
                            break;
                        }
                    }
                }

                // Adding "current_detections" to "detections_list" object
                m_detections_list.push_back (current_detections);
            }

            frames_processed++;
            
            cv::imshow("dbg", frame);
            char k  = cv::waitKey(5);
            
            frame   = m_img_source->read();
            if (!frame.data) {
                qCritical() << "No data in just read frame!";
                break;
            }
            if (frame.size() != first_frame_size)
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