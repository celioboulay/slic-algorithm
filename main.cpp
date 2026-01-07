#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <iostream>
#include <cmath>
#include <string>

struct Cluster { // better than class for data
    double L, a, b; // CIELAB colors
    int x, y;
    Cluster(double L_, double a_, double b_, int x_, int y_)
        : L(L_), a(a_), b(b_), x(x_), y(y_) {}
};


template <class Derived> 
Eigen::MatrixBase<Derived> Mcolor(const cv::Mat& labf, char modalitiy){
    switch (modalitiy)
    {
    case 'L':
        /* code */
        break;
    case 'a':
        /* code */
        break;
    case 'b':
        /* code */
        break;
    
    default:
        break;
    }

}


template <class Derived> // tell compiler it will work for any Derived type compatible with Eigen
void init(const Eigen::MatrixBase<Derived>& ML,
            const Eigen::MatrixBase<Derived>& Ma,
            const Eigen::MatrixBase<Derived>& Mb,
            int k)
{
    const int r = img.rows(); // if can be const then it must be const
    const int c = img.cols();

    // find size for sampling pixels at regular grid steps S
    const int sr = max(1, floor(r/sqrt(k))); // probably we will have r=c
    const int sc = max(1, floor(c/sqrt(k)));

    std::vector<Cluster> clusters;
    clusters.reserve(k);

    for (int x = sr/2; x < r && clusters.size() < k; x += sr) {
        for (int y = sc/2; y < c && clusters.size() < k; y += sc) {
            double L, a, b;
            
            clusters.emplace_back(L, a, b, x, y); // emplace_back(args...) <=> constructeur T(args...)
        }
    }
}


void slic(Eigen::Ref< // already a ref 
    Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>> M) {
    int r = M.rows();
    int c = M.cols();


}



int main() {
    cv::Mat img = cv::imread("image.png", cv::IMREAD_COLOR_BGR);
    cv::Mat lab;
    cv::cvtColor(img, lab, cv::COLOR_BGR2Lab);
    cv::Mat labf;
    lab.convertTo(labf, CV_32FC3);

    if (img.empty()) return -1;
    if (img.type() != CV_8UC1) return -2;

    Eigen::Map<
        Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>
    > M(img.data, img.rows, img.cols);


    cv::imshow("image", img);
    cv::waitKey(0);
    return 0;
}