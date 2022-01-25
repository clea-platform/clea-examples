QT += core websockets
QT -= gui
TARGET = cam-people-counter-example
TEMPLATE = app
CONFIG += console c++11

MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
UI_DIR      = build/ui
RCC_DIR     = build/

INCLUDEPATH += include \
                /usr/local/include/AstarteDeviceSDKQt5 \
                /opt/intel/openvino_2021.4.689/deployment_tools/inference_engine/include \
                /opt/intel/openvino_2021.4.689/deployment_tools/ngraph/include \
                /opt/intel/openvino_2021.4.689/opencv/include


LIBS += -L/opt/intel/openvino_2021.4.689/opencv/lib \
        -L/opt/intel/openvino_2021.4.689/inference_engine/lib/intel64 \
        -lmosquitto \
        -lmosquittopp \
        -lAstarteDeviceSDKQt5 \
        -lgflags \
        -lopencv_core \
        -lopencv_imgproc \
        -lopencv_videoio \
        -lopencv_highgui \
        -lopencv_imgcodecs \
        -linference_engine \
        -lwfrest \
        -lz \
        -lworkflow

HEADERS += include/peopleCounter.hpp \
            include/streamingServer.hpp

SOURCES += src/main.cpp \
            src/peopleCounter.cpp \
            src/streamingServer.cpp \
            src/polygon.cpp \
            src/cnn.cpp \
            src/detector.cpp \
            src/distance.cpp \
            src/tracker.cpp \
            src/utils.cpp \
            src/monitors/memory_monitor.cpp \
            src/utils/kuhn_munkres.cpp \
            src/monitors/cpu_monitor.cpp \
            src/monitors/presenter.cpp \
            src/utils/images_capture.cpp \
            src/utils/performance_metrics.cpp