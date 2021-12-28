cmake_minimum_required(VERSION 3.15)


project(autoaim)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
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
add_executable(autoaim autoaim.cpp
                        autoaim.h
                        ./detector/inference.cpp
                        ./detector/inference.h
                        ./debug.h
                        ../debug.h
                        ../general/general.cpp
                        ../general/general.h
                        ../coordsolver/coordsolver.cpp
                        ../coordsolver/coordsolver.h
                        ./predictor/particle_filter.cpp
                        ./predictor/particle_filter.h
                        ./predictor/predictor.cpp
                        ./predictor/predictor.h
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
target_link_libraries(autoaim ${OpenCV_LIBRARIES} ${InferenceEngine_LIBRARIES} ${Eigen3_LIBRARIES} yaml-cpp)