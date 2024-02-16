#include "FeedProcessor.h"

#include <thread>
#include <chrono>

#include <opencv2/opencv.hpp>

#include "../oscpp/client.hpp"

using namespace std::chrono_literals;

bool FeedProcessor::init() {

    try {
        m_nn = cv::dnn::readNetFromONNX("leap.onnx");
    } catch (...) {
        return false;
    }

    m_nn.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    m_nn.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);

    std::vector<std::thread> nn_threads;
    nn_threads.reserve(2);

    for (int i = 0; i < 2; ++i) {
        nn_threads.emplace_back([this]{ process_frames(); });
    }

    for (auto& thread : nn_threads) {
        thread.detach();
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.05f, 1.0f);

    m_filter = OneEuroFilter(dis(gen), 0.9f, 5.0f);

    m_running.store(true);

    connect();

    return true;
}

void FeedProcessor::fetch_frames() {
    for (auto& feed : m_feeds) {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto pair = std::make_pair(feed.type(), feed.fetch());
        m_frames.push(pair);
        m_cv.notify_all();
    }
}

std::thread FeedProcessor::process_frames() {
    while (m_running.load()) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return !m_frames.empty(); });

        auto [type, frame] = m_frames.front();

        if (!frame.empty()) {
            cv::Mat blob;
            cv::dnn::blobFromImage(frame, blob, 1.0, cv::Size(112, 112));

            m_nn.setInput(blob);

            cv::Mat pre_landmarks = m_nn.forward();
            cv::Mat landmarks = pre_landmarks.reshape(2, 7);

            float openness = abs(landmarks.at<float>(0, 0) - landmarks.at<float>(3, 1));

            float filtered_openness = m_filter(openness);

            switch(type) {
                case FeedType::FEED_LEFT_EYE:
                {
                    std::array<char, 2048> buffer{};
                    OSCPP::Client::Packet packet(buffer.data(), buffer.size());

                    packet.openMessage("/avatar/parameters/LeftEyeLidExpandedSqueeze", 1)
                            .float32(filtered_openness)
                            .closeMessage();

                    int len = packet.size();
                    send(m_fd, reinterpret_cast<const char *>(buffer.data()), len, 0);
                    break;
                }
                case FeedType::FEED_RIGHT_EYE:
                {
                    std::array<char, 2048> buffer{};
                    OSCPP::Client::Packet packet(buffer.data(), buffer.size());

                    packet.openMessage("/avatar/parameters/RightEyeLidExpandedSqueeze", 1)
                            .float32(filtered_openness)
                            .closeMessage();

                    int len = packet.size();
                    send(m_fd, reinterpret_cast<const char *>(buffer.data()), len, 0);
                    break;
                }
            }
        }
        m_frames.pop();
    }
}

void FeedProcessor::connect() {

    m_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(9000);
    address.sin_addr.s_addr = inet_addr("127.0.0.1");

    int result = ::connect(m_fd, (struct sockaddr*)&address, sizeof(address));

    if (result != 0) {
        printf("Could not connect to module.\n");
    }

    printf("Connected to the VRCFT Omnicept module.\n");
}