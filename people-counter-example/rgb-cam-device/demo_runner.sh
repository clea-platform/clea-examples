
#!/bin/bash

if [[ $1 == "-h" || $1 == "--help" ]] ; then
    echo -e "People counter CLEA demo executor. Usage:\n\t\tbash demo_runner.sh [-h,--help] [settings_file]\n"
    echo -e "\t\t-h, --help\tDisplay this message\n\t\tsettings_file\tUse the provided settings file as demo settings file"
    exit
fi

CONFIG_FILE="$1"
echo -e "Using this configuration file: $CONFIG_FILE\n"


source /opt/intel/openvino_2021/bin/setupvars.sh

if [ -z ${INTEL_OPENVINO_DIR+x} ] ; then
    echo -e "VERY DANGEROUS: INTEL_OPENVINO_DIR environment variable NOT set"
    exit
fi

# Extending environment variable LD_LIBRARY_PATH
export LD_LIBRARY_PATH=:/usr/local/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INTEL_OPENVINO_DIR/data_processing/dl_streamer/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INTEL_OPENVINO_DIR/data_processing/gstreamer/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INTEL_OPENVINO_DIR/opencv/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INTEL_OPENVINO_DIR/deployment_tools/ngraph/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INTEL_OPENVINO_DIR/deployment_tools/inference_engine/external/tbb/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INTEL_OPENVINO_DIR/deployment_tools/inference_engine/external/hddl/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INTEL_OPENVINO_DIR/deployment_tools/inference_engine/external/omp/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INTEL_OPENVINO_DIR/deployment_tools/inference_engine/external/gna/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INTEL_OPENVINO_DIR/deployment_tools/inference_engine/external/mkltiny_lnx/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INTEL_OPENVINO_DIR/deployment_tools/inference_engine/lib/intel64

./rgb-cam-people-counter-example $CONFIG_FILE