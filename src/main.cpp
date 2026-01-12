#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <cmath>
#include <array>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <stdexcept>

#include "slic/io.hpp"
#include "slic/core.hpp"
#include "slic/utils.hpp"
#include "slic/render.hpp"
#include "slic/cluster.hpp"


int main() {

    std::string image_path = "../data/banana.png";
    const cv::Mat labf = img_to_labf(image_path);
    const cv::Mat laplacian = gradient_map(image_path);
    if (labf.empty()) return -1;


    const int n_pixels = 150; // k also should depend on the grid size
    const int S = compute_s(laplacian.rows, laplacian.cols, n_pixels); // find size for sampling pixels at regular grid steps S

    // Initialise cluster centers Ck, clusters will change their centers as the algorithm runs
    std::vector<Cluster> clusters = init_clusters(laplacian, labf, n_pixels, S);

    // Label and Distance map to track l(i) and d(i)
    Eigen::MatrixXf labels = lab_to_grid(labf);
    labels.setConstant(-1);
    Eigen::MatrixXf distance = lab_to_grid(labf);
    distance.setConstant(INFINITY);

    //Slic
    const float threshold = 10; // fix later also f(image size)
    float E = threshold+1; // residual error
    while (E > threshold){
        E = slic(clusters, labf, labels, distance, S);
        std::cout << E << '\n';
    }

    
    cv::Mat output_image = render_output(labf, labels);

    cv::imshow("image", output_image);
    cv::waitKey(0);
    return 0;
}