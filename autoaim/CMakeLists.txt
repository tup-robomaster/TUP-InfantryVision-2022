cmake_minimum_required(VERSION 3.15)


project(autoaim)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
find_package(OpenCV 4 REQUIRED)
find_package(InferenceEngine REQUIRED)
find_package(Eigen3 REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${EIGEN3_INCLUDE_DIRS})
include_directories(${InferenceEngine_INCLUDE_DIRS})

message(${EIGEN3_INCLUDE_DIRS})

aux_source_directory(detector detector_src)
aux_source_directory(predictor predictor_src)

# add_library(autoaim_lib OBJECT autoaim.cpp ${detector_src} ${predictor_src})
add_library(autoaim_lib OBJECT ${detector_src} ${predictor_src})
# target_link_libraries(autoaim ${OpenCV_LIBRARIES} ${InferenceEngine_LIBRARIES})
add_executable(autoaim autoaim.cpp
                        ./detector/inference.cpp
                        ./detector/inference.h
                        ./debug.h
                        ./predictor/particle_filter.cpp
                        ./predictor/particle_filter.h 
                        )
target_link_libraries(autoaim ${OpenCV_LIBRARIES} ${InferenceEngine_LIBRARIES} ${Eigen3_LIBRARIES})