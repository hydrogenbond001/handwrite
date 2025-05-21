#include <opencv2/opencv.hpp>

// cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);


int main() {
    cv::Mat img = cv::imread("test.jpg");
    if (img.empty()) return -1;
    cv::imshow("Image", img);
    cv::waitKey();
    return 0;
}
