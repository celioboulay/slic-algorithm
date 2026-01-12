#pragma once

#include <opencv2/opencv.hpp>
#include <string>

cv::Mat img_to_labf(const std::string& img_path);

cv::Mat gradient_map(const std::string& img_path);
