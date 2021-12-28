cmake_minimum_required(VERSION 3.15)


project(Infantry_Vision)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17 -Os -pthread")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++17 -pthread")
find_package(OpenCV 4 REQUIRED)
find_package(InferenceEngine REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(yaml-cpp REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${EIGEN3_INCLUDE_DIRS})
include_directories(${InferenceEngine_INCLUDE_DIRS})

# aux_source_directory(detector detector_src)
# aux_source_directory(predictor predictor_src)

#TODO: CMAKE as a submodule
# add_library(autoaim_lib OBJECT autoaim.cpp ${detector_src} ${predictor_src})
# add_library(autoaim_lib OBJECT ${detector_src} ${predictor_src})
# target_link_libraries(autoaim ${OpenCV_LIBRARIES} ${InferenceEngine_LIBRARIES})
add_executable(Infantry_Vision main.cpp
                        ./autoaim/autoaim.cpp
                        ./autoaim/autoaim.h
                        ./autoaim/detector/inference.cpp
                        ./autoaim/detector/inference.h
                        ./autoaim//debug.h
                        ./debug.h
                        ./general/general.cpp
                        ./general/general.h
                        ./coordsolver/coordsolver.cpp
                        ./coordsolver/coordsolver.h
                        ./autoaim/predictor/particle_filter.cpp
                        ./autoaim/predictor/particle_filter.h
                        ./autoaim/predictor/predictor.cpp
                        ./autoaim/predictor/predictor.h
                        ./thread/thread.cpp
                        ./thread/thread.h
                        ./camera/DaHengCamera.h
                        ./camera/DaHengCamera.cpp
                        ./camera/DaHengCamera.cpp
                        ./camera/DxImageProc.h
                        ./camera/GxIAPI.h
                        ./serial/serialport.cpp
                        ./serial/serialport.h
                        ./serial/CRC_Check.cpp
                        ./serial/CRC_Check.h
                        )

# add_executable(autoaim autoaim.cpp
#                         ./detector/inference.cpp
#                         ./detector/inference.h
#                         ./debug.h
#                         ../debug.h
#                         ../coordsolver/coordsolver.cpp
#                         ../coordsolver/coordsolver.h
#                         ../general/general.cpp
#                         ../general/general.h
#                         )
target_link_libraries(Infantry_Vision ${OpenCV_LIBRARIES} ${InferenceEngine_LIBRARIES} ${Eigen3_LIBRARIES}
                         yaml-cpp gxiapi)
