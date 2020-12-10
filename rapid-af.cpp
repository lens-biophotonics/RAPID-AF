#include <algorithm>
#include <vector>

#include "rapid-af.h"

using namespace cv;
using namespace std;

Mat binarize(const Mat &image, double percentage)
{
    Mat thrImage;

    double min, max;
    minMaxLoc(image, &min, &max);

    threshold(image, thrImage, percentage * max, 65535, THRESH_BINARY);
    return thrImage;
}

Mat dog(const Mat &image, int ksize, double sigma1, double sigma2)
{
    Mat filter1, filter2;
    mulTransposed(getGaussianKernel(ksize, sigma1), filter1, false);
    mulTransposed(getGaussianKernel(ksize, sigma2), filter2, false);
    normalize(filter1, filter1, 1, 0, NORM_L1);
    normalize(filter2, filter2, 1, 0, NORM_L1);
    Mat filter = filter1 - filter2;

    Mat ret;

    filter2D(image, ret, -1, filter);

    return ret;
}

Mat canny(const Mat &image, int ksize, double sigma, double alpha, double beta)
{
    Mat filter, edges, filtered;
    mulTransposed(getGaussianKernel(ksize, sigma), filter, false);
    filter2D(image, filtered, -1, filter);

    Canny(filtered, edges, alpha, beta);

    return edges;
}

Mat crossCorr(const Mat &image1, const Mat &image2, const uint padding)
{
    Mat temp1, temp2, result, padded;

    image1.convertTo(temp1, CV_32F);
    image2.convertTo(temp2, CV_32F);

    copyMakeBorder(temp1, padded, padding, padding, padding, padding, BORDER_CONSTANT);

    matchTemplate(padded, temp2, result, TM_CCORR_NORMED);

    return result;
}

// adapted from https://github.com/opencv/opencv/blob/master/samples/cpp/dft.cpp
Mat dftSpectrum(const Mat &I)
{
    Mat padded;                            //expand input image to optimal size
    int m = getOptimalDFTSize(I.rows);
    int n = getOptimalDFTSize(I.cols);     // on the border add zero values
    copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, BORDER_CONSTANT, Scalar::all(0));
    Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};
    Mat complexI;
    merge(planes, 2, complexI);           // Add to the expanded another plane with zeros
    dft(complexI, complexI);              // this way the result may fit in the source matrix

    // compute the magnitude
    split(complexI, planes);                    // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
    magnitude(planes[0], planes[1], planes[0]); // planes[0] = magnitude
    Mat mag = planes[0];

    // crop the spectrum, if it has an odd number of rows or columns
    mag = mag(Rect(0, 0, mag.cols & -2, mag.rows & -2));

    int cx = mag.cols / 2;
    int cy = mag.rows / 2;

    // rearrange the quadrants of Fourier image
    // so that the origin is at the image center
    Mat tmp;
    Mat q0(mag, Rect(0, 0, cx, cy));
    Mat q1(mag, Rect(cx, 0, cx, cy));
    Mat q2(mag, Rect(0, cy, cx, cy));
    Mat q3(mag, Rect(cx, cy, cx, cy));

    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);

    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);

    return mag;
}

double spectralRatio(const Mat &spectrum, int radius, int thickness)
{
    Mat annularMask = Mat::zeros(spectrum.size(), CV_8U);
    circle(annularMask, Point(spectrum.size() / 2), radius, Scalar::all(255), thickness);

    Mat masked = Mat::zeros(spectrum.size(), spectrum.type());
    spectrum.copyTo(masked, annularMask);

    return sum(masked).val[0] / sum(spectrum).val[0];
}

bool checkImageQuality(Mat &image, double stdVarThreshold, double sRatioThreshold)
{
    Scalar mean, stddev;

    meanStdDev(image, mean, stddev);

    double stdVar = stddev.val[0] / mean.val[0];
    double sRatio = spectralRatio(dftSpectrum(image), 30, 5);

    return (stdVar > stdVarThreshold) && (sRatio > sRatioThreshold);
}

Point2f align(const Mat &image1, const Mat &image2, const struct Options opt, bool * const ok)
{
    Mat cc;
    double min, max;
    Point maxLoc;
    Point upperLeft(opt.padding, opt.padding);

    Point shiftBin, shiftDog, shiftCanny;
    int c = 0;

    if (opt.bin_enable) {
        c++;
        Mat bin1, bin2;
        bin1 = binarize(image1, opt.bin_thresholdPercentage);
        bin2 = binarize(image2, opt.bin_thresholdPercentage);

        cc = crossCorr(bin1, bin2, opt.padding);

        minMaxLoc(cc, &min, &max, nullptr, &maxLoc);

        shiftBin = maxLoc - upperLeft;
    }

    if (opt.dog_enable) {
        c++;
        Mat dog1, dog2;
        dog1 = dog(image1, opt.dog_ksize, opt.dog_sigma1, opt.dog_sigma2);
        dog2 = dog(image2, opt.dog_ksize, opt.dog_sigma1, opt.dog_sigma2);

        cc = crossCorr(dog1, dog2, opt.padding);

        minMaxLoc(cc, &min, &max, nullptr, &maxLoc);

        shiftDog = maxLoc - upperLeft;
    }


    if (opt.canny_enable) {
        c++;
        Mat canny1, canny2;
        canny1 = canny(image1, opt.canny_ksize, opt.canny_sigma, opt.canny_alpha, opt.canny_beta);
        canny2 = canny(image2, opt.canny_ksize, opt.canny_sigma, opt.canny_alpha, opt.canny_beta);

        cc = crossCorr(canny1, canny2, opt.padding);

        minMaxLoc(cc, &min, &max, nullptr, &maxLoc);

        shiftCanny = maxLoc - upperLeft;
    }

    Point2f finalShift = shiftBin + shiftDog + shiftCanny;
    finalShift /= c;

    if (ok != nullptr) {
        double n1 = norm(Mat(shiftBin), Mat(shiftDog));
        double n2 = norm(Mat(shiftBin), Mat(shiftCanny));
        double n3 = norm(Mat(shiftDog), Mat(shiftCanny));

        *ok = n1 < opt.agreementThreshold
              && n2 < opt.agreementThreshold
              && n3 < opt.agreementThreshold;
    }

    return finalShift;
}
