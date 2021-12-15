
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

include_directories     (${CMAKE_CURRENT_SOURCE_DIR}/include
                            /opt/intel/openvino_2021.4.689/deployment_tools/inference_engine/include)
add_executable          (people-counter ${SOURCES})
target_link_libraries   (people-counter ${OpenCV_LIBRARIES} ngraph::ngraph utils gflags monitors)
