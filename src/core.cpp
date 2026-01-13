#include "slic/core.hpp"

#include <cmath>
#include <algorithm>
#include <array>
#include <utility>
#include <vector>


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



float compute_distance(Cluster c, int xi, int yi, float Li, float ai, float bi, int S){
    float dc2 = pow(c.L - Li,2) + pow(c.a - ai,2) + pow(c.b - bi, 2);
    float ds2 = pow(c.x - xi,2) + pow(c.y - yi, 2);

    int m = 35; // can be [1,40] when working with CIELAB

    float D = sqrt(dc2 + ((ds2 / pow(S,2)) * pow(m,2)));

    return D;
}


float update_clusters(std::vector<Cluster>& clusters, const cv::Mat& labf, 
                    Eigen::MatrixXf& labels)
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
