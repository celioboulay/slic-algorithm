#include "slic/utils.hpp"

#include <cmath>
#include <algorithm>


int compute_s(int rows, int cols, int k){
    if (k <= 0) return 1; // division 0 not good :(
    return std::max(1, (int)std::floor(std::sqrt((rows * cols) / (double)k)));
}



Eigen::MatrixXf lab_to_grid(const cv::Mat& lab){
    Eigen::MatrixXf grid(lab.rows, lab.cols);
    return grid;
}