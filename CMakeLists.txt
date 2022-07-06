cmake_minimum_required(VERSION 3.15)

project(Infantry_Vision)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17 -Os -pthread -g")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17 -pthread")
find_package(OpenCV 4.5.0)
set(CMAKE_PREFIX_PATH /opt/intel/openvino_2021/deployment_tools/inference_engine/share)
find_package(InferenceEngine REQUIRED)
# set(CMAKE_PREFIX_PATH /opt/intel/openvino_2021/deployment_tools/ngraph/cmake)
# find_package(ngraph)
find_package(Eigen3 REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(matplotlib_cpp REQUIRED)
find_package(Ceres REQUIRED COMPONENTS EigenSparse)
find_package(fmt REQUIRED)
find_package(glog REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${EIGEN3_INCLUDE_DIRS})
include_directories(${InferenceEngine_INCLUDE_DIRS})

# aux_source_directory(detector detector_src)
# aux_source_directory(predictor predictor_src)

# add_library(autoaim_lib OBJECT autoaim.cpp ${detector_src} ${predictor_src})
# add_library(autoaim_lib OBJECT ${detector_src} ${predictor_src})
# target_link_libraries(autoaim ${OpenCV_LIBRARIES} ${InferenceEngine_LIBRARIES})
add_executable(Infantry_Vision main.cpp
            ${PROJECT_SOURCE_DIR}/autoaim/armor_tracker.cpp
            ${PROJECT_SOURCE_DIR}/autoaim/armor_tracker.h            
            ${PROJECT_SOURCE_DIR}/autoaim/autoaim.cpp
            ${PROJECT_SOURCE_DIR}/autoaim/autoaim.h
            ${PROJECT_SOURCE_DIR}/autoaim/detector/inference.cpp
            ${PROJECT_SOURCE_DIR}/autoaim/detector/inference.h
            ${PROJECT_SOURCE_DIR}/autoaim/predictor/predictor.cpp
            ${PROJECT_SOURCE_DIR}/autoaim/predictor/predictor.h
            ${PROJECT_SOURCE_DIR}/buff/fan_tracker.cpp
            ${PROJECT_SOURCE_DIR}/buff/fan_tracker.h            
            ${PROJECT_SOURCE_DIR}/buff/buff.cpp
            ${PROJECT_SOURCE_DIR}/buff/buff.h
            ${PROJECT_SOURCE_DIR}/buff/detector/inference.cpp
            ${PROJECT_SOURCE_DIR}/buff/detector/inference.h
            ${PROJECT_SOURCE_DIR}/buff/predictor/predictor.cpp
            ${PROJECT_SOURCE_DIR}/buff/predictor/predictor.h
            ${PROJECT_SOURCE_DIR}/debug.h
            ${PROJECT_SOURCE_DIR}/general/general.cpp
            ${PROJECT_SOURCE_DIR}/general/general.h
            ${PROJECT_SOURCE_DIR}/coordsolver/coordsolver.cpp
            ${PROJECT_SOURCE_DIR}/coordsolver/coordsolver.h
            ${PROJECT_SOURCE_DIR}/filter/particle_filter.cpp
            ${PROJECT_SOURCE_DIR}/filter/particle_filter.h
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
            ${PROJECT_SOURCE_DIR}/hik_camera/HaiKangCamera.cpp
)

# add_executable(autoaim autoaim.cpp
#                      ${PROJECT_SOURCE_DIR}/detector/inference.cpp
#                      ${PROJECT_SOURCE_DIR}/detector/inference.h
#                      ${PROJECT_SOURCE_DIR}/debug.h
#                         ../debug.h
#                         ../coordsolver/coordsolver.cpp
#                         ../coordsolver/coordsolver.h
#                         ../general/general.cpp
#                         ../general/general.h
#                         )
target_link_libraries(Infantry_Vision ${OpenCV_LIBRARIES} ${InferenceEngine_LIBRARIES} ${Eigen3_LIBRARIES}
                         yaml-cpp gxiapi Ceres::ceres fmt::fmt glog matplotlib_cpp::matplotlib_cpp)

# 包含海康相机动态链接库
find_library(MVS_LIB libMvCameraControl.so /opt/MVS/lib/64/)
target_link_libraries(Infantry_Vision ${MVS_LIB})

# set(LINK_DIR /opt/intel/openvino_2021/deployment_tools/inference_engine/lib/intel64)

# link_directories(${LINK_DIR})

# target_link_libraries(Infantry_Vision /opt/intel/openvino_2021/deployment_tools/inference_engine/lib/intel64/libinference_engine_transformations.so)