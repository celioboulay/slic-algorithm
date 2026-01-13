#include "slic/render.hpp"


cv::Mat render_output(const cv::Mat& labf,
                    const Eigen::MatrixXf& labels)
{

    // need to fix pixels that do not belong to the same connected component as their cluster center may remain (C. Post Processing section)
    cv::Mat output_image = labf.clone();
    const int rows = output_image.rows;
    const int cols = output_image.cols;

    Eigen::MatrixXi smooth_labels = labels.cast<int>();

    for (int y = 1; y < rows-1; y++){
        for (int x = 1; x < cols-1; x++){
            int l = smooth_labels(y, x);

            if (l == smooth_labels(y+1, x) &&
                l == smooth_labels(y-1, x) &&
                l == smooth_labels(y, x+1) &&
                l == smooth_labels(y, x-1))
                continue;

            int counts[9];
            int keys[9];
            int n = 0;

            for (int dy = -1; dy <= 1; dy++){
                for (int dx = -1; dx <= 1; dx++){
                    int nl = smooth_labels(y + dy, x + dx);
                    int i = 0;
                    for (; i < n; i++){
                        if (keys[i] == nl){
                            counts[i]++;
                            break;
                        }
                    }
                    if (i == n){
                        keys[n] = nl;
                        counts[n] = 1;
                        n++;
                    }
                }
            }

            int best_label = l;
            int best_count = 0;
            for (int i = 0; i < n; i++){
                if (counts[i] > best_count){
                    best_count = counts[i];
                    best_label = keys[i];
                }
            }

            if (best_count >= 6){
                smooth_labels(y, x) = best_label;
            }
        }
    }

    for (int y = 1; y < rows-1; y++){
        for (int x = 1; x < cols-1; x++){
            int l = smooth_labels(y, x);
            if (l != smooth_labels(y+1, x) || l != smooth_labels(y, x+1)){
                output_image.at<cv::Vec3f>(y, x) = cv::Vec3f(0.0, 128.0, 128.0);
                output_image.at<cv::Vec3f>(y+1, x) = cv::Vec3f(0.0, 128.0, 128.0);
                output_image.at<cv::Vec3f>(y-1, x) = cv::Vec3f(0.0, 128.0, 128.0);
                output_image.at<cv::Vec3f>(y, x+1) = cv::Vec3f(0.0, 128.0, 128.0);
                output_image.at<cv::Vec3f>(y, x-1) = cv::Vec3f(0.0, 128.0, 128.0);
            }
        }
    }

    // convert back labf to bgr
    cv::Mat lab8, bgr;
    output_image.convertTo(lab8, CV_8UC3);
    cv::cvtColor(lab8, bgr, cv::COLOR_Lab2BGR);
    return bgr;
}
