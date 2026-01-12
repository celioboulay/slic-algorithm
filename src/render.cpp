#include "slic/render.hpp"


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
                output_image.at<cv::Vec3f>(y, x) = cv::Vec3f(0.0, 128.0, 128.0);
            }
        }
    }
    // convert back labf to bgr
    cv::Mat lab8, bgr;
    output_image.convertTo(lab8, CV_8UC3);
    cv::cvtColor(lab8, bgr, cv::COLOR_Lab2BGR);
    return bgr;
}
