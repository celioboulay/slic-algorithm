#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>
#include <array>
#include <vector>
#include <stdexcept>


struct Cluster { // better than class for data
    float L, a, b; // CIELAB colors
    int x, y; // center coords
    int label;
    Cluster(int label_, float L_, float a_, float b_, int x_, int y_)
        : label(label_), L(L_), a(a_), b(b_), x(x_), y(y_) {}
};


int compute_s(int rows, int cols, int k){
    if (k <= 0) return 1; // division 0 not good :(
    return std::max(1, (int)std::floor(std::sqrt((rows * cols) / (double)k)));
}



std::vector<Cluster> init_clusters(const cv::Mat& laplacian,
                                    const cv::Mat& labf, int k, int S){

    const int r = laplacian.rows;
    const int c = laplacian.cols;

    constexpr std::array<std::pair<int,int>, 9> neigh3x3 = {{
    {-1,-1}, {-1,0}, {-1,1},
    { 0,-1}, { 0,0}, { 0,1},
    { 1,-1}, { 1,0}, { 1,1}
    }};


    std::vector<Cluster> clusters;
    clusters.reserve(k);
    int cluster_label{};

    for (int x = S/2; x < c && clusters.size() < k; x += S) { 
        for (int y = S/2; y < r && clusters.size() < k; y += S) {
            
            int xk = x;
            int yk = y;
            
            float lowest_gradient = laplacian.at<float>(y, x);
            // Move (x,y) to lowest gradient position in 3x3 neighborhood,
            // assume it stays in grid
            for (const std::pair<int,int>& p : neigh3x3){
                int nx = x + p.first;
                int ny = y + p.second;
                if (nx < 0 || nx >= r || ny < 0 || ny >= c) continue; // bounds
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
            
            clusters.emplace_back(cluster_label, L, a, b, xk, yk); // emplace_back(args...) <=> constructeur T(args...)

            cluster_label++;
        }
    }

    for (Cluster& cluster : clusters){
        cluster.x = std::min(labf.cols - 1, std::max(0, cluster.x));
        cluster.y = std::min(labf.rows - 1, std::max(0, cluster.y));
    }
    return clusters;

}



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

Eigen::MatrixXf lab_to_grid(const cv::Mat& lab){
    Eigen::MatrixXf grid(lab.rows, lab.cols);
    return grid;
}



float compute_distance(Cluster c, int xi, int yi, float Li, float ai, float bi, int S){
    float dc2 = pow(c.L - Li,2) + pow(c.a - ai,2) + pow(c.b - bi, 2);
    float ds2 = pow(c.x - xi,2) + pow(c.y - yi, 2);

    int m = 10; // can be [1,40] when working with CIELAB

    float D = sqrt(dc2 + ((ds2 / pow(S,2)) * pow(m,2)));

    return D;
}

float L2_norm(){ // L2 norm is used to compute a residual error E
    return 0;
}


float update_clusters(std::vector<Cluster>& clusters, const cv::Mat& labf, // may need to optimize later
                    Eigen::MatrixXf& labels) // will return the residual error E 
{
    /* for each pixel somme cluster correspondant
    keep how much pixels in each clusters et hop on divise. */

    const int r = labels.cols();
    const int c = labels.rows(); // inversion ok 

    const int n_clusters = clusters.size();
    
    std::vector<int> pixel_per_cluster(n_clusters, 0);

    std::vector<Cluster> newClusters;
    newClusters.reserve(n_clusters);
    for (int i = 0; i < n_clusters; i++){
        newClusters.emplace_back(i,0,0,0,0,0);
    }

    for (int x = 0; x < r; x++) {
        for (int y = 0; y < c; y++){

            int lbl = (int)labels(y, x);
            if (lbl < 0 || lbl >= n_clusters) continue;
            pixel_per_cluster[lbl]++;

            cv::Vec3f lab = labf.at<cv::Vec3f>(y, x);
            newClusters[labels(y, x)].L += lab[0];
            newClusters[labels(y, x)].a += lab[1];
            newClusters[labels(y, x)].b += lab[2];
            newClusters[labels(y, x)].x += x;
            newClusters[labels(y, x)].y += y;

        }
    }

    for (Cluster& cluster : newClusters){
        if (pixel_per_cluster[cluster.label]==0){
            cluster = clusters[cluster.label];
            continue;
        }
        int pix = pixel_per_cluster[cluster.label];
        cluster.x = cluster.x / pix; // cluster.label sould never be empty (I hope)
        cluster.y = cluster.y / pix;
        cluster.L = cluster.L / pix;
        cluster.a = cluster.a / pix;
        cluster.b = cluster.b / pix;
    }



    float E{}; // L2 norm is used to compute a residual error E between the new/old cluster center locations
    for (int i = 0; i < n_clusters; i++){
        E += std::sqrt(
            (clusters[i].L - newClusters[i].L) * (clusters[i].L - newClusters[i].L)
            + (clusters[i].a - newClusters[i].a) * (clusters[i].a - newClusters[i].a)
            + (clusters[i].b - newClusters[i].b) * (clusters[i].b - newClusters[i].b)
            + (clusters[i].x - newClusters[i].x) * (clusters[i].x - newClusters[i].x)
            + (clusters[i].y - newClusters[i].y) * (clusters[i].y - newClusters[i].y)
        );
    }

    clusters = newClusters; // assignation

    return E;
}


/* runs one iteration of the algorithm and returns the residual error */
float slic(std::vector<Cluster>& clusters,
        const cv::Mat& labf,
        Eigen::MatrixXf& labels,
        Eigen::MatrixXf& distance,
        int S)
{ 
    const int r = labels.rows();
    const int c = labels.cols();

    distance.setConstant(INFINITY);
    /* Assignment */
    for (Cluster& cluster : clusters){ // for each cluster center Ck
        int xk = cluster.x;
        int yk = cluster.y;
        
        // for each pixel i in a 2S × 2S region around Ck
        for (int xi = std::max(0, xk - S); xi < std::min(c, xk + S); xi++) {
            for (int yi = std::max(0, yk - S); yi < std::min(r, yk + S); yi++) {
                if ((unsigned)yi >= (unsigned)r || (unsigned)xi >= (unsigned)c) continue;
                cv::Vec3f lab = labf.at<cv::Vec3f>(yi, xi);

                float Li = lab[0]; 
                float ai = lab[1];
                float bi = lab[2]; // get pixel colors

                float D = compute_distance(cluster, xi, yi, Li, ai, bi, S);

                if (D < distance(yi, xi)){
                    distance(yi, xi) = D;
                    labels(yi, xi) = cluster.label;
                }
            }
        }
    }
    
    /* Update */
    // compute new cluster centers update step adjusts the cluster centers to be the mean [l a b x y] of pixels in cluster
    float E = update_clusters(clusters, labf, labels);  // residual error
    
    return E;
}



cv::Mat render_output(const cv::Mat& labf,
                    const Eigen::MatrixXf& labels) // actually maybe not const
{
    // cv::convexHull surement pour commencer
    // need to fix  pixels that do not belong to the same connected component as their cluster center may remain (C. Post Processing section)
    // draw a 1px border around each cluster
    cv::Mat output_image = labf.clone();
    const int rows = output_image.rows;
    const int cols = output_image.cols;

    for (int y = 0; y < rows-1; y++){
        for (int x = 0; x < cols-1; x++){
            int l = labels(y, x);
            if (l != labels(y+1, x) || l != labels(y, x+1)){
                output_image.at<cv::Vec3f>(y, x) = cv::Vec3f(0.0, 0.0, 0.0);
            }
        }
    }
    // convert back labf to bgr
    cv::Mat lab8, bgr;
    output_image.convertTo(lab8, CV_8UC3);
    cv::cvtColor(lab8, bgr, cv::COLOR_Lab2BGR);
    return bgr;
}




int main() {
    // loading image
    std::string image_path = "banana.png";
    const cv::Mat labf = img_to_labf(image_path); // can access it fast just to get values, it will not change
    const cv::Mat laplacian = gradient_map(image_path);
    if (labf.empty()) return -1;

    const int r = laplacian.rows; // if can be const then it must be const
    const int c = laplacian.cols;

    const int n_pixels = 100; // k
    const int S = compute_s(r, c, n_pixels); // find size for sampling pixels at regular grid steps S

    // Initialise cluster centers Ck, clusters will change their centers as the algorithm runs
    std::vector<Cluster> clusters = init_clusters(laplacian, labf, n_pixels, S);

    // Label and Distance map to track l(i) and d(i)
    Eigen::MatrixXf labels = lab_to_grid(labf);
    labels.setConstant(-1);
    Eigen::MatrixXf distance = lab_to_grid(labf);
    distance.setConstant(INFINITY);

    //Slic
    const float threshold = 5; // fix later also f(image size)
    float E = threshold+1; // residual error
    while (E > threshold){
        E = slic(clusters, labf, labels, distance, S);
        std::cout << E << '\n';
    }

    // render final image
    cv::Mat output_image = render_output(labf, labels);

    cv::imshow("image", output_image);
    cv::waitKey(0);
    return 0;
}