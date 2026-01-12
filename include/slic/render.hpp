#pragma once

#include <opencv2/opencv.hpp>
#include <Eigen/Dense>

cv::Mat render_output(const cv::Mat& labf,
                    const Eigen::MatrixXf& labels);
