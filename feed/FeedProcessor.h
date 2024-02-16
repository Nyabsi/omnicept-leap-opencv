#ifndef OPENCV_ONNX_LEAP_FEEDPROCESSOR_H
#define OPENCV_ONNX_LEAP_FEEDPROCESSOR_H

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>

#include <queue>
#include <vector>
#include <condition_variable>

#include <opencv2/opencv.hpp>

#include "CameraFeed.h"
#include "../misc/euro_filter.h"

class FeedProcessor {
public:
    bool init();
    void add_feed(const CameraFeed& feed) { m_feeds.push_back(feed); }

    void fetch_frames();
private:
    std::thread process_frames();
    void connect();

    std::vector<CameraFeed> m_feeds;
    std::queue<std::pair<FeedType, cv::Mat>> m_frames;
    cv::dnn::Net m_nn;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_running;
    SOCKET m_fd;
    OneEuroFilter m_filter;
};


#endif //OPENCV_ONNX_LEAP_FEEDPROCESSOR_H
