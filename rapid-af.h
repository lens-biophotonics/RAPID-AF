#ifndef RAPID_AF_H
#define RAPID_AF_H

#include <opencv2/opencv.hpp>

using namespace cv;

namespace rapid_af {
struct Options {
    bool multiThreading = true;
    uint padding = 100;
    double agreementThreshold = 5;

    bool bin_enable = true;
    double bin_thresholdPercentage = 0.6;

    bool dog_enable = true;
    int dog_ksize = 100;
    double dog_sigma1 = 5;
    double dog_sigma2 = 10;

    bool canny_enable = true;
    int canny_ksize = 29;
    double canny_sigma = 20;
    double canny_alpha = 0;
    double canny_beta = 1;
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
