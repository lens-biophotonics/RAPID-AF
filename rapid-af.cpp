#include "rapid-af.h"

#include <algorithm>
#include <thread>
#include <vector>

using namespace cv;
using namespace std;

namespace rapid_af {
/**
 * @brief Binarize image at given percentage of maximum value
 * @param image input image
 * @param percentage percentage of image maximum
 * @return binarized image
 *
 * Binarize input image using as threshold a user-specified percenntage of image
 * maximum value
 */
Mat binarize(const Mat &image, double percentage)
{
    Mat thrImage;

    double min, max;
    minMaxLoc(image, &min, &max);

    threshold(image, thrImage, percentage * max, 65535, THRESH_BINARY);
    return thrImage;
}

/**
 * @brief Apply a Difference-of-Gaussians (DoG) filter
 * @param image input image
 * @param ksize size of the filter kernel
 * @param sigma1 sigma of the first Gaussian
 * @param sigma2 sigmma of the second Gaussian
 * @return filtered image
 *
 * Apply a Difference-of-Gaussians (DoG) filter to the input image. The width
 * of the two Gaussians is specified by the user. This filter enhances
 * structures with spatial scales intermediate between the two sigmas. More
 * details can be found at
 * https://en.wikipedia.org/wiki/Difference_of_Gaussians
 */
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

/**
 * @brief Apply Canny edge detection filter
 * @param image input image
 * @param ksize size of the smoothing Gaussian kernel
 * @param sigma sigma of the smoothing Gaussian kernel
 * @param alpha minimum value for hysteresis thresholding
 * @param beta maximum value for hysteresis thresholding
 * @return filtered image
 *
 * Apply Canny edge detection filter to the input image. First, the image is
 * smoothed using a Gaussian filter of user-specified size. Then, bidimensional
 * gradient is computed, and local maxima are identified. To refine edge
 * detection, hysteresis threshold is performed: only those segments which are
 * completely above the minimum value and partially above the maximum value are
 * retained. More details can be found at
 * https://docs.opencv.org/master/da/d22/tutorial_py_canny.html
 */
Mat canny(const Mat &image, int ksize, double sigma, double alpha, double beta)
{
    Mat filter, edges, filtered;
    mulTransposed(getGaussianKernel(ksize, sigma), filter, false);
    filter2D(image, filtered, -1, filter);

    Canny(filtered, edges, alpha, beta);

    return edges;
}

/**
 * @brief Compute cross-correlation between the two input images
 * @param image1
 * @param image2
 * @param padding size of image padding
 * @return cross-correlation image
 *
 * Compute cross-correlation between the two input images.
 * The search area is specified by the padding parameter.
 */
Mat crossCorr(const Mat &image1, const Mat &image2, const uint padding)
{
    Mat temp1, temp2, result, padded;

    image1.convertTo(temp1, CV_32F);
    image2.convertTo(temp2, CV_32F);

    copyMakeBorder(temp1, padded, padding, padding, padding, padding, BORDER_CONSTANT);

    matchTemplate(padded, temp2, result, TM_CCORR_NORMED);

    return result;
}

/**
 * @brief Compute power spectrum of the input image
 * @param I input image
 * @return power spectrum with frequencies origin at the image center
 *
 * Compute the power spectrum of the input image, and swaps the quadrants in
 * order to place the frequencies origin at the image center. adapted from
 * https://github.com/opencv/opencv/blob/master/samples/cpp/dft.cpp
 */
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

/**
 * @brief Compute the fraction of total spectral power in a given frequency range
 * @param spectrum power spectrum image
 * @param radius center of the frequency range
 * @param thickness width of the frequency range
 * @return fraction of spectral power in the given range
 *
 * Compute the fraction of total spectral power in a given frequency range.
 * This information can be used as a measure of image quality.
 */
double spectralRatio(const Mat &spectrum, int radius, int thickness)
{
    Mat annularMask = Mat::zeros(spectrum.size(), CV_8U);
    circle(annularMask, Point(spectrum.size() / 2), radius, Scalar::all(255), thickness);

    Mat masked = Mat::zeros(spectrum.size(), spectrum.type());
    spectrum.copyTo(masked, annularMask);

    return sum(masked).val[0] / sum(spectrum).val[0];
}

/**
 * @brief Check image quality before perform registration
 * @param image input image
 * @param stdVarThreshold threshold for the coefficient of variation
 * @param sRatioThreshold threshold for the spectral ratio
 * @param radius center of the frequecy range
 * @param thickness width of the frequecy range
 * @return boolean indicating if quality check has been passed or not
 *
 * Perform image quality check using two different image quality metrics:
 * coefficient of variation (standard deviation / mean) and fraction of total
 * spectral power in given frequency range. If both values are above given
 * thresholds, the function returns a True boolean value, otherwise it returns
 * a False.
 */
bool checkImageQuality(Mat &image, double stdVarThreshold, double sRatioThreshold, int radius,
                       int thickness)
{
    Scalar mean, stddev;

    meanStdDev(image, mean, stddev);

    double stdVar = stddev.val[0] / mean.val[0];
    double sRatio = spectralRatio(dftSpectrum(image), radius, thickness);

    return (stdVar > stdVarThreshold) && (sRatio > sRatioThreshold);
}

/**
 * @brief Find the mutual displacement between two images
 * @param image1 input image 1
 * @param image2 input image 2
 * @param opt struct type containing registration options
 * @param ok reference to a boolean that reports if registration was successful or not
 * @return 2d displacement
 *
 * Perform registration between the input images. In the struct opt, the user
 * specifies which pre-processing method should be used (potentially also a
 * combination of multiple methods), and the parameters of the different
 * methods. The registration is considered 'successful' if the displacements
 * computed using different pre-processing methods are consistent within a
 * user-specified range.
 */
Point2f align(const Mat &image1, const Mat &image2, const struct Options opt, bool * const ok)
{
    double min, max;
    Point maxLoc;
    Point upperLeft(opt.padding, opt.padding);

    Point shifts[3];
    int c = 0;

    thread myTrheads[3];

    auto bin_f = [&](int j){
        Mat bin1, bin2;
        bin1 = binarize(image1, opt.bin_threshold);
        bin2 = binarize(image2, opt.bin_threshold);

        Mat cc = crossCorr(bin1, bin2, opt.padding);

        minMaxLoc(cc, &min, &max, nullptr, &maxLoc);

        shifts[j] = maxLoc - upperLeft;
    };

    auto dog_f = [&](int j){
        Mat dog1, dog2;
        dog1 = dog(image1, opt.dog_ksize, opt.dog_sigma1, opt.dog_sigma2);
        dog2 = dog(image2, opt.dog_ksize, opt.dog_sigma1, opt.dog_sigma2);

        Mat cc = crossCorr(dog1, dog2, opt.padding);

        minMaxLoc(cc, &min, &max, nullptr, &maxLoc);

        shifts[j] = maxLoc - upperLeft;
    };

    auto canny_f = [&](int j) {
        Mat canny1, canny2;
        canny1 = canny(image1, opt.canny_ksize, opt.canny_sigma, opt.canny_alpha, opt.canny_beta);
        canny2 = canny(image2, opt.canny_ksize, opt.canny_sigma, opt.canny_alpha, opt.canny_beta);

        Mat cc = crossCorr(canny1, canny2, opt.padding);

        minMaxLoc(cc, &min, &max, nullptr, &maxLoc);

        shifts[j] = maxLoc - upperLeft;
    };

    if (opt.bin_enable) {
        if (opt.multithreading_enable) {
            myTrheads[c] = thread(bin_f, c);
        } else {
            bin_f(c);
        }
        c++;
    }

    if (opt.dog_enable) {
        if (opt.multithreading_enable) {
            myTrheads[c] = thread(dog_f, c);
        } else {
            dog_f(c);
        }
        c++;
    }

    if (opt.canny_enable) {
        if (opt.multithreading_enable) {
            myTrheads[c] = thread(canny_f, c);
        } else {
            canny_f(c);
        }
        c++;
    }

    if (opt.multithreading_enable) {
        for (int i = 0; i < c; ++i) {
            myTrheads[i].join();
        }
    }

    Point2f finalShift = shifts[0] + shifts[1] + shifts[2];
    finalShift /= c;

    for (int i = c; i < 3; ++i) {
        shifts[i] = shifts[0];
    }

    if (ok != nullptr) {
        double n1 = norm(Mat(shifts[0]), Mat(shifts[1]));
        double n2 = norm(Mat(shifts[0]), Mat(shifts[2]));
        double n3 = norm(Mat(shifts[1]), Mat(shifts[2]));

        *ok = n1 < opt.agreement_threshold
              && n2 < opt.agreement_threshold
              && n3 < opt.agreement_threshold;
    }

    return finalShift;
}
} // namespace rapid_af
