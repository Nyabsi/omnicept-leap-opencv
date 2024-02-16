cmake_minimum_required(VERSION 3.27)
project(opencv_onnx_leap)

set(CMAKE_CXX_STANDARD 23)
set(OpenCV_DIR "C:\\opencv")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(opencv_onnx_leap main.cpp
        feed/FeedProcessor.cpp
        feed/FeedProcessor.h
        feed/CameraFeed.cpp
        feed/CameraFeed.h
        misc/euro_filter.h)

set(OpenCV_LIBS opencv_core opencv_imgproc opencv_highgui opencv_imgcodecs)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ws2_32)