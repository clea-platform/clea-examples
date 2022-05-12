# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "EDGEHOG-MICRO")
set(PROJECT_VER "0.2.1")
add_definitions(-DEHM_ENV="prod")
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
string(TIMESTAMP TODAY "%Y%m%d%H%M%S")
project(edgehog-micro-${TODAY})
