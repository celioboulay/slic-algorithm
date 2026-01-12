#pragma once

#include <opencv2/opencv.hpp>
#include <Eigen/Dense>

int compute_s(int rows, int cols, int k);

Eigen::MatrixXf lab_to_grid(const cv::Mat& lab);