cmake_minimum_required(VERSION 3.15)


project(Buff)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17 -Os -pthread")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17 -pthread")
find_package(OpenCV 4.5.0)
set(CMAKE_PREFIX_PATH /opt/intel/openvino_2021.4.752/deployment_tools/inference_engine/share)
find_package(InferenceEngine REQUIRED)
# set(CMAKE_PREFIX_PATH /opt/intel/openvino_2021/deployment_tools/ngraph/cmake)
# find_package(ngraph)
find_package(Eigen3 REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(matplotlib_cpp REQUIRED)
find_package(Ceres REQUIRED COMPONENTS EigenSparse)
find_package(fmt REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${EIGEN3_INCLUDE_DIRS})
include_directories(${InferenceEngine_INCLUDE_DIRS})

# aux_source_directory(detector detector_src)
# aux_source_directory(predictor predictor_src)

#TODO: CMAKE as a submodule
# add_library(buff_lib OBJECT buff.cpp ${detector_src} ${predictor_src})
# add_library(buff_lib OBJECT ${detector_src} ${predictor_src})
# target_link_libraries(buff ${OpenCV_LIBRARIES} ${InferenceEngine_LIBRARIES})
add_executable(Buff main.cpp
            ${PROJECT_SOURCE_DIR}/buff/fan_tracker.h            
            ${PROJECT_SOURCE_DIR}/buff/buff.cpp
            ${PROJECT_SOURCE_DIR}/buff/buff.h
            ${PROJECT_SOURCE_DIR}/buff/detector/inference.cpp
            ${PROJECT_SOURCE_DIR}/buff/detector/inference.h
            ${PROJECT_SOURCE_DIR}/buff//debug.h
            ${PROJECT_SOURCE_DIR}/debug.h
            ${PROJECT_SOURCE_DIR}/general/general.cpp
            ${PROJECT_SOURCE_DIR}/general/general.h
            ${PROJECT_SOURCE_DIR}/coordsolver/coordsolver.cpp
            ${PROJECT_SOURCE_DIR}/coordsolver/coordsolver.h
            ${PROJECT_SOURCE_DIR}/buff/predictor/particle_filter.cpp
            ${PROJECT_SOURCE_DIR}/buff/predictor/particle_filter.h
            ${PROJECT_SOURCE_DIR}/buff/predictor/predictor.cpp
            ${PROJECT_SOURCE_DIR}/buff/predictor/predictor.h
            ${PROJECT_SOURCE_DIR}/thread/thread.cpp
            ${PROJECT_SOURCE_DIR}/thread/thread.h
            ${PROJECT_SOURCE_DIR}/camera/DaHengCamera.h
            ${PROJECT_SOURCE_DIR}/camera/DaHengCamera.cpp
            ${PROJECT_SOURCE_DIR}/camera/DaHengCamera.cpp
            ${PROJECT_SOURCE_DIR}/camera/DxImageProc.h
            ${PROJECT_SOURCE_DIR}/camera/GxIAPI.h
            ${PROJECT_SOURCE_DIR}/serial/serialport.cpp
            ${PROJECT_SOURCE_DIR}/serial/serialport.h
            ${PROJECT_SOURCE_DIR}/serial/wt61pc.cpp
            ${PROJECT_SOURCE_DIR}/serial/wt61pc.h
            ${PROJECT_SOURCE_DIR}/serial/CRC_Check.cpp
            ${PROJECT_SOURCE_DIR}/serial/CRC_Check.h
)

# add_executable(buff buff.cpp
#                      ${PROJECT_SOURCE_DIR}/detector/inference.cpp
#                      ${PROJECT_SOURCE_DIR}/detector/inference.h
#                      ${PROJECT_SOURCE_DIR}/debug.h
#                         ../debug.h
#                         ../coordsolver/coordsolver.cpp
#                         ../coordsolver/coordsolver.h
#                         ../general/general.cpp
#                         ../general/general.h
#                         )
target_link_libraries(Buff ${OpenCV_LIBRARIES} ${InferenceEngine_LIBRARIES} ${Eigen3_LIBRARIES}
                         yaml-cpp gxiapi Ceres::ceres fftw3 fmt::fmt ${matplotlib_cpp_INCLUDE_DIRS}
                                                                    matplotlib_cpp::matplotlib_cpp)

# set(LINK_DIR /opt/intel/openvino_2021/deployment_tools/inference_engine/lib/intel64)

# link_directories(${LINK_DIR})

# target_link_libraries(Buff /opt/intel/openvino_2021/deployment_tools/inference_engine/lib/intel64/libinference_engine_transformations.so)