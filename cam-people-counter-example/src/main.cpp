
#include "peopleCounter.hpp"

#include <QtCore/QCommandLineParser>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QObject>

#include <HemeraCore/Operation>

#include "core.hpp"
#include "utils.hpp"
#include "tracker.hpp"
#include "descriptor.hpp"
#include "distance.hpp"
#include "detector.hpp"
#include "pedestrian_tracker_demo.hpp"

#include <monitors/presenter.h>
#include <utils/images_capture.h>

#include <opencv2/core.hpp>

#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <csignal>
#include <gflags/gflags.h>

using namespace InferenceEngine;


PeopleCounter *people_counter_ptr = nullptr;

std::unique_ptr<PedestrianTracker>
CreatePedestrianTracker(const std::string& reid_model,
                        const InferenceEngine::Core & ie,
                        const std::string & deviceName,
                        bool should_keep_tracking_info) {
    TrackerParams params;

    if (should_keep_tracking_info) {
        params.drop_forgotten_tracks = false;
        params.max_num_objects_in_track = -1;
    }

    std::unique_ptr<PedestrianTracker> tracker(new PedestrianTracker(params));

    // Load reid-model.
    std::shared_ptr<IImageDescriptor> descriptor_fast =
        std::make_shared<ResizedImageDescriptor>(
            cv::Size(16, 32), cv::InterpolationFlags::INTER_LINEAR);
    std::shared_ptr<IDescriptorDistance> distance_fast =
        std::make_shared<MatchTemplateDistance>();

    tracker->set_descriptor_fast(descriptor_fast);
    tracker->set_distance_fast(distance_fast);

    /* FIXME
    if (!reid_model.empty()) {
        CnnConfig reid_config(reid_model);
        reid_config.max_batch_size = 16;   // defaulting to 16

        std::shared_ptr<IImageDescriptor> descriptor_strong =
            std::make_shared<DescriptorIE>(reid_config, ie, deviceName);

        if (descriptor_strong == nullptr) {
            throw std::runtime_error("[SAMPLES] internal error - invalid descriptor");
        }
        std::shared_ptr<IDescriptorDistance> distance_strong =
            std::make_shared<CosDistance>(descriptor_strong->size());

        tracker->set_descriptor_strong(descriptor_strong);
        tracker->set_distance_strong(distance_strong);
    } else {
        std::cout << "WARNING: Reid model "
            << "was not specified. "
            << "Only fast reidentification approach will be used." << std::endl;
    }//*/

    return tracker;
}


bool ParseAndCheckCommandLine(int argc, char *argv[]) {
    // ---------------------------Parsing and validation of input args--------------------------------------

    gflags::ParseCommandLineNonHelpFlags(&argc, &argv, true);
    if (FLAGS_h) {
        showUsage();
        showAvailableDevices();
        return false;
    }

    if (FLAGS_i.empty()) {
        throw std::logic_error("Parameter -i is not set");
    }

    if (FLAGS_m_det.empty()) {
        throw std::logic_error("Parameter -m_det is not set");
    }

    if (FLAGS_m_reid.empty()) {
        throw std::logic_error("Parameter -m_reid is not set");
    }

    return true;
}


void signal_handler (int sig_num) {
    if (sig_num == SIGINT || sig_num == SIGTERM) {
        people_counter_ptr->stop ();
    }
    else {
        std::cerr << "Unhandled signal " << sig_num << std::endl;
    }
}




int main(int argc, char **argv) {
    try {
        std::cout << "InferenceEngine: " << printable(*GetInferenceEngineVersion()) << std::endl;

        if (!ParseAndCheckCommandLine(argc, argv)) {
            return 0;
        }

        // Reading command line parameters
        auto det_model  = FLAGS_m_det;
        auto reid_model = FLAGS_m_reid;
        auto detlog_out = FLAGS_out;

        auto detector_mode  = FLAGS_d_det;
        auto reid_mode      = FLAGS_d_reid;

        auto custom_cpu_library         = FLAGS_l;
        auto path_to_custom_layers      = FLAGS_c;
        bool should_use_perf_counter    = FLAGS_pc;

        bool should_print_out   = FLAGS_r;

        bool should_show        = !FLAGS_no_show;
        int delay               = FLAGS_delay;
        if (!should_show)
            delay = -1;
        should_show = (delay >= 0);

        bool should_save_det_log = !detlog_out.empty();

        std::vector<std::string> devices{detector_mode, reid_mode};
        InferenceEngine::Core ie    = LoadInferenceEngine(devices, custom_cpu_library,
                                                            path_to_custom_layers, should_use_perf_counter);

        DetectorConfig detector_confid(det_model);
        ObjectDetector pedestrian_detector(detector_confid, ie, detector_mode);

        bool should_keep_tracking_info = should_save_det_log || should_print_out;
        std::unique_ptr<PedestrianTracker> tracker =
            CreatePedestrianTracker(reid_model, ie, reid_mode,
                                    should_keep_tracking_info);

        std::unique_ptr<ImagesCapture> source = openImagesCapture(FLAGS_i, FLAGS_loop, FLAGS_first, FLAGS_read_limit);


        PeopleCounter people_counter ("./app-settings.ini", std::ref(source), std::ref(pedestrian_detector),
                                        std::ref(tracker));
        people_counter_ptr  = &people_counter;

        std::signal (SIGINT, signal_handler);
        std::signal (SIGTERM, signal_handler);

        // Waiting for people_counter completion
        people_counter.wait_for_completion ();
    }
    catch (const std::exception& error) {
        qCritical() << "[ ERROR ] " << error.what() << "\n";
        return EXIT_FAILURE;
    }
    catch (...) {
        qCritical() << "[ ERROR ] Unknown/internal exception happened.\n";
        return EXIT_FAILURE;
    }

    std::cout << "Execution successful" << std::endl;

    return 0;
}