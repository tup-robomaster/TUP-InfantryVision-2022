cmake_minimum_required(VERSION 2.8)
SET(CMAKE_BUILD_TYPE "Debug") # "Debug" "Release"
PROJECT(Adjust_Version)

find_package(OpenCV 4.5.0)
find_package(Eigen3 REQUIRED)
find_package(fmt REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -march=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native")

include_directories(${CMAKE_SOURCE_DIR} ./include)
include_directories(${EIGEN3_INCLUDE_DIR})

include_directories(${OpenCV_INCLUDE_DIRS})
ADD_EXECUTABLE(Adjust_Version  ./main.cpp
                               ./HaiKangCamera.cpp)

target_link_libraries(Adjust_Version ${OpenCV_LIBS})
find_package(Threads)
target_link_libraries(Adjust_Version ${CMAKE_THREAD_LIBS_INIT})
find_package(X11 REQUIRED)
include_directories(${X11_INLUCDE_DIR})
target_link_libraries(Adjust_Version ${X11_LIBRARIES})

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${EIGEN3_LIBRARIES} gxiapi fmt::fmt)  # 包含大恒动态链接库

# 包含海康相机动态链接库
find_library(MVS_LIB libMvCameraControl.so /opt/MVS/lib/64/)
target_link_libraries(Adjust_Version ${MVS_LIB})