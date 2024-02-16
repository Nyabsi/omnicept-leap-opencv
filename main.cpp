#include <iostream>
#include <thread>

#include <opencv2/opencv.hpp>

#include "feed/CameraFeed.h"
#include "feed/FeedProcessor.h"

using namespace std;

int main() {

    cout << "G2 Omnicept Eyelid tracking." << "\n";

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return EXIT_FAILURE;

    FeedProcessor processor;

    bool result;
    CameraFeed left_feed;
    CameraFeed right_feed;

    result = left_feed.init(FeedType::FEED_LEFT_EYE);

    if (!result) {
        printf("Failed to initialize LeftEye feed.\n");
        return EXIT_FAILURE;
    }

    result = right_feed.init(FeedType::FEED_RIGHT_EYE);

    if (!result) {
        printf("Failed to initialize RightEye feed.\n");
        return EXIT_FAILURE;
    }

    processor.add_feed(left_feed);
    processor.add_feed(right_feed);

    result = processor.init();

    if (!result) {
        printf("Failed to initialize FeedProcessor.\n");
        return EXIT_FAILURE;
    }

    while (true) {
        processor.fetch_frames();
        std::this_thread::sleep_for(10ms);
    }
}
