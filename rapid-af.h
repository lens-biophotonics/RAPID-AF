#ifndef RAPID_AF_H
#define RAPID_AF_H

#include <opencv2/opencv.hpp>

using namespace cv;

namespace rapid_af {
struct Options {
    bool multiThreading = true;      //!< Compute image preprocessing using multiple threads
    uint padding = 100;              //!< Padding used for cross correlation
    double agreementThreshold = 5;   //!< Maximum allowed discrepancy between displacements

    bool bin_enable = true;                 //!< Enable binarize
    double bin_thresholdPercentage = 0.6;   //!< Percentage of maximum for binarization

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

Mat binarize(const Mat &image, double percentage);
Mat dog(const Mat &image, int ksize, double sigma1, double sigma2);
Mat canny(const Mat &image, int ksize, double sigma, double alpha, double beta);

Mat crossCorr(const Mat &image1, const Mat &image2, const uint padding);
bool checkImageQuality(Mat &image, double stdVarThreshold, double sRatioThreshold, int radius, int thickness);

Point2f align(const Mat &image1, const Mat &image2,
              const struct Options opt, bool * const ok = nullptr);
}

#endif // RAPID_AF_H
