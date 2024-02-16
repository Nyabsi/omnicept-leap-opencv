#ifndef OPENCV_ONNX_LEAP_CAMERAFEED_H
#define OPENCV_ONNX_LEAP_CAMERAFEED_H

#include <opencv2/opencv.hpp>

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>

enum class FeedType {
    FEED_RIGHT_EYE,
    FEED_LEFT_EYE
};

class CameraFeed {
public:
    bool init(FeedType type);
    cv::Mat fetch();
    FeedType type() { return m_type; }
private:
    void connect();

    bool m_initialized = false;
    SOCKET m_fd;
    FeedType m_type;
    cv::VideoCapture m_capture;
};


#endif //OPENCV_ONNX_LEAP_CAMERAFEED_H
