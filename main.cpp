#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>

struct Cluster { // better than class for data
    float L, a, b; // CIELAB colors
    int x, y;
    Cluster(float L_, float a_, float b_, int x_, int y_)
        : L(L_), a(a_), b(b_), x(x_), y(y_) {}
};



std::vector<Cluster> init_clusters(const cv::Mat& laplacian,
                                    const cv::Mat& labf, int k){

    const int r = laplacian.rows; // if can be const then it must be const
    const int c = laplacian.cols;

    // find size for sampling pixels at regular grid steps S
    const int sr = std::max(double(1), floor(r/sqrt(k))); // probably we will have r=c
    const int sc = std::max(double(1), floor(c/sqrt(k)));

    constexpr std::array<std::pair<int,int>, 9> neigh3x3 = {{
    {-1,-1}, {-1,0}, {-1,1},
    { 0,-1}, { 0,0}, { 0,1},
    { 1,-1}, { 1,0}, { 1,1}
    }};


    std::vector<Cluster> clusters;
    clusters.reserve(k);

    for (int x = sr/2; x < r && clusters.size() < k; x += sr) {
        for (int y = sc/2; y < c && clusters.size() < k; y += sc) {
            
            int xk = x;
            int yk = y;
            float lowest_gradient = laplacian.at<float>(y, x);
            // Move (x,y) to lowest gradient position in 3x3 neighborhood,
            // assume it stays in grid
            for (const std::pair<int,int>& p : neigh3x3){
                int nx = x + p.first;
                int ny = y + p.second;
                float curr_gradient = laplacian.at<float>(y, x);
                
                if (curr_gradient < lowest_gradient){
                    lowest_gradient = curr_gradient;
                    xk = nx; yk = ny;
                }
            }

            cv::Vec3f lab = labf.at<cv::Vec3f>(yk, xk);
            float L = lab[0]; 
            float a = lab[1];
            float b = lab[2];
            
            clusters.emplace_back(L, a, b, xk, yk); // emplace_back(args...) <=> constructeur T(args...)
        }
    }
    return clusters;
}



/* Loads image into CIELAB format from file path */
cv::Mat img_to_labf(const std::string& img_path){ // "image.png"
    cv::Mat img = cv::imread(img_path, cv::IMREAD_COLOR_BGR);
    
    cv::Mat lab;
    cv::cvtColor(img, lab, cv::COLOR_BGR2Lab);
    
    cv::Mat labf;
    lab.convertTo(labf, CV_32FC3);
    
    return labf;
}

cv::Mat gradient_map(const std::string& img_path){ // useful for clusters init
    cv::Mat img = cv::imread(img_path, cv::IMREAD_GRAYSCALE);
    CV_Assert(!img.empty());

    cv::Mat lap, abs_lap;
    cv::Laplacian(img, lap, CV_32F, 3);
    cv::convertScaleAbs(lap, abs_lap);

    return abs_lap;
}


Eigen::MatrixXf lab_to_grid(const cv::Mat& lab){
    Eigen::MatrixXf grid(lab.rows, lab.cols);
    return grid;
}



float D(Cluster c, int xi, int yi, float Li, float ai, float bi){
    float dc2 = pow(c.L - Li,2) + pow(c.a - ai,2) + pow(c.b - bi, 2);
    float ds2 = pow(c.x - xi,2) + pow(c.y - yi, 2);

    int m = 10; // can be [1,40] when working with CIELAB
    float S; // sqrt(N/K) what is used in init

    float D = sqrt(dc2 + ((ds2 / pow(S,2)) * pow(m,2)));

    return D;
}



/* runs one iteration of the algorithm and returns the residual error */
float slic(const std::vector<Cluster>& clusters,
        Eigen::MatrixXf& labels,
        Eigen::MatrixXf& distance)
{ 
    float E;




    return E;
}





int main() {
    // loading image
    std::string image_path = "image.png";
    const cv::Mat labf = img_to_labf(image_path); // can access it fast just to get values, it will not change
    const cv::Mat laplacian = gradient_map(image_path);
    if (labf.empty()) return -1;

    const int n_pixels = 100; // k
    // Initialise cluster centers Ck, clusters will not change as the algorithm runs
    const std::vector<Cluster> clusters = init_clusters(laplacian, labf, n_pixels);

    // Label and Distance map to track l(i) and d(i)
    Eigen::MatrixXf labels = lab_to_grid(labf);
    labels.setConstant(-1);
    Eigen::MatrixXf distance = lab_to_grid(labf);
    distance.setConstant(INFINITY);

    //Slic
    const float threshold = 1e-4; // fix later
    float E = 42; // residual error
    while (E > threshold){
        E = slic(clusters, labels, distance);
    }



    // render final image
    cv::Mat output_image;

    cv::imshow("image", output_image);
    cv::waitKey(0);
    return 0;
}