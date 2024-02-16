#include "CameraFeed.h"

struct BEPacketHeader {
    unsigned char id;
    unsigned int len;
};

struct BEPacketImage {
    int32_t width;
    int32_t height;
    int32_t bpp;
    char data[10000];
};

bool CameraFeed::init(FeedType type) {

    m_type = type;

    connect();

    m_initialized = true;
    return true;
}

cv::Mat CameraFeed::fetch() {
    cv::Mat surface;

    char header[5];
    auto readBytes = recv(m_fd, header, sizeof(header), 0);

    if (readBytes < 0) {
        return surface;
    }

    BEPacketHeader BEHeader = {
            .id =  *(unsigned char*)(header + 0),
            .len = *(unsigned int *)(header + 1)
    };

    // TODO: this exists for the possibility of just ditching the Omnicept runtime entirely.
    switch (BEHeader.id) {
        case 0x1:
        {
            std::vector<char> buffer(BEHeader.len);
            recv(m_fd, buffer.data(), BEHeader.len, 0);

            BEPacketImage image = *((BEPacketImage*)&buffer);
            surface = cv::Mat(image.height, image.width, CV_8UC1, image.data);

            if (!surface.empty()) {
                cv::resize(surface, surface, cv::Size(128, 128), 0, 0, cv::INTER_LINEAR);
                cv::cvtColor(surface, surface, cv::COLOR_BGR2RGB);
                surface.convertTo(surface, CV_32F, 1.0 / 255.0);
            }

            break;
        }
        case 0x2:
        {
            std::vector<char> buffer(BEHeader.len);
            recv(m_fd, buffer.data(), BEHeader.len, 0);

            BEPacketImage image = *((BEPacketImage*)&buffer);
            surface = cv::Mat(image.height, image.width, CV_8UC1, image.data);

            if (!surface.empty()) {
                cv::resize(surface, surface, cv::Size(128, 128), 0, 0, cv::INTER_LINEAR);
                cv::cvtColor(surface, surface, cv::COLOR_BGR2RGB);
                surface.convertTo(surface, CV_32F, 1.0 / 255.0);
            }

            break;
        }
        default:
        {
            connect();
            break;
        }
    }

    return surface;
}

void CameraFeed::connect() {

    m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(1234);
    address.sin_addr.s_addr = inet_addr("127.0.0.1");

    ::connect(m_fd, (struct sockaddr*)&address, sizeof(address));

    if (m_type == FeedType::FEED_LEFT_EYE) {
        uint8_t data[] = { 0x1 };
        send(m_fd, reinterpret_cast<const char *>(data), 1, 0);
    } else {
        uint8_t data[] = { 0x2 };
        send(m_fd, reinterpret_cast<const char *>(data), 1, 0);
    }
}