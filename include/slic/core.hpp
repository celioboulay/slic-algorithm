#pragma once

#include <opencv2/opencv.hpp>
#include <Eigen/Dense>
#include <vector>

#include "slic/cluster.hpp"

std::vector<Cluster> init_clusters(const cv::Mat& laplacian,
                                    const cv::Mat& labf, int k, int S);

float compute_distance(Cluster c, int xi, int yi, float Li, float ai, float bi, int S);

float L2_norm();

float update_clusters(std::vector<Cluster>& clusters, const cv::Mat& labf,
                    Eigen::MatrixXf& labels);

float slic(std::vector<Cluster>& clusters,
        const cv::Mat& labf,
        Eigen::MatrixXf& labels,
        Eigen::MatrixXf& distance,
        int S);
