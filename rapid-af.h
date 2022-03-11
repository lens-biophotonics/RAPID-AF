#ifndef RAPID_AF_H
#define RAPID_AF_H

#include <opencv2/opencv.hpp>

namespace rapid_af {
struct AlignOptions {
    bool multithreading_enable = true;      //!< Compute image preprocessing using multiple threads
    uint padding = 100;              //!< Padding used for cross correlation
    double agreement_threshold = 5;   //!< Maximum allowed discrepancy between displacements

    bool prefilter_enable = true;  //!< Apply Gaussian prefiltering
    double prefilter_ksize = 20;   //!< Kernel size for Gaussian filter
    double prefilter_sigma = 5;    //!< Sigma for Gaussian filter

    bool bin_enable = true;                 //!< Enable binarize
    double bin_threshold = 0.6;   //!< Threshold of maximum for binarization. Value in [0, 1].

    bool dog_enable = true;  //!< Enable Difference-of-Gaussians (DoG)
    int dog_ksize = 100;     //!< Kernel size for DoG
    double dog_sigma1 = 5;   //!< Sigma of first Gaussian
    double dog_sigma2 = 10;  //!< Sigma of second Gaussian

    bool canny_enable = true;  //!< Enable Canny filter
    int canny_ksize = 29;      //!< Kernel size for Canny filter
    double canny_sigma = 20;   //!< Smoothing sigma for Canny filter
    double canny_alpha = 0;    //!< Lower threshold for hysteresis thresholding
    double canny_beta = 1;     //!< Higher threshold for hysteresis thresholding
};

cv::Mat binarize(const cv::Mat &image, double percentage);
cv::Mat dog(const cv::Mat &image, int ksize, double sigma1, double sigma2);
cv::Mat canny(const cv::Mat &image, int ksize, double sigma, double alpha, double beta);

cv::Mat crossCorr(const cv::Mat &image1, const cv::Mat &image2, const uint padding);
bool checkImageQuality(cv::Mat &image, double stdVarThreshold, double sRatioThreshold, int radius,
                       int thickness);

cv::Point2f align(const cv::Mat &image1, const cv::Mat &image2, const struct AlignOptions opt,
                  bool * const ok = nullptr);
}

#endif // RAPID_AF_H
