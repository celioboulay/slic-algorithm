#include "slic/io.hpp"


/* Loads image into CIELAB format from file path */
cv::Mat img_to_labf(const std::string& img_path){ // "image.png"
    cv::Mat img = cv::imread(img_path, cv::IMREAD_COLOR);
    if (img.empty()) return cv::Mat();

    cv::Mat lab;
    cv::cvtColor(img, lab, cv::COLOR_BGR2Lab);
    
    cv::Mat labf;
    lab.convertTo(labf, CV_32FC3);
    
    return labf;
}

cv::Mat gradient_map(const std::string& img_path){ // useful for clusters init
    cv::Mat img = cv::imread(img_path, cv::IMREAD_GRAYSCALE);
    CV_Assert(!img.empty());

    cv::Mat lap;
    cv::Laplacian(img, lap, CV_32F, 3);

    return cv::abs(lap);
}
