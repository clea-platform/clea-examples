
cmake_minimum_required(VERSION 3.10)

project(cam-people-counter-example)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(OpenCV REQUIRED COMPONENTS core highgui videoio imgproc imgcodecs gapi)
find_package(InferenceEngine REQUIRED)
find_package(ngraph REQUIRED)

include_directories (common/monitors/include)

add_subdirectory(thirdparty/gflags)
add_subdirectory(common)
add_subdirectory(device)