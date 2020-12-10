#include <chrono>

#include <opencv2/opencv.hpp>

#include <rapid-af.h>


using namespace cv;
using namespace std;
using namespace rapid_af;


Mat merge(Mat image1, Mat image2, Point2f shift)
{
    Mat M = Mat(2, 3, CV_64FC1); // Allocate memory

    M.at<double>(0, 0) =  1;  //p1
    M.at<double>(1, 0) =  0;  //p2;
    M.at<double>(0, 1) = 0;   //p3;
    M.at<double>(1, 1) = 1;   //p4;
    M.at<double>(0, 2) = shift.x;   //p5;
    M.at<double>(1, 2) = shift.y;   //p6;

    Mat shifted;

    Mat channels[3];
    channels[0] = Mat::zeros(image1.size(), CV_8UC1);
    warpAffine(image2, channels[1], M, image2.size());
    channels[1].convertTo(channels[1], CV_8UC1);
    image1.convertTo(channels[2], CV_8UC1);

    Mat merged;
    merge(channels, 3, merged);
    return merged;
}


int main(int argc, char** argv)
{
    const String keys =
        "{help h usage ?     |      | print this message}"
        "{@image1            |      | image1 for alignment}"
        "{@image2            |      | image2 for alignment}"
        "{mt-enable          |true  | enable multithreading}"
        "{o output           |      | path of debug output image}"
        "{p padding          |100   | search range for alignment in px}"
        "{pf-enable          |true  | enable prefiltering}"
        "{pf-ksize           |20    | prefilter gaussian kernel size}"
        "{pf-sigma           |5.    | prefilter gaussian kernel sigma}"
        "{agreement          |5     | agreement threshold in px}"
        "{q                  |true  | check image quality before alignment}"
        "{q-std              |0.1   | quality check: minimum standard deviation}"
        "{q-sratio           |1e-4  | quality check: minimum spectral ratio (within annular ring)}"
        "{q-sratio-radius    |30    | quality check: radius for spectral ratio}"
        "{q-sratio-thickness |5     | quality check: thickness for spectral ratio}"
        "{bin-enable         |true  | enable binary thresholding}"
        "{bin-threshold-perc |0.6   | binary threshold percentage}"
        "{dog-enable         |true  | enable Difference of Gaussian}"
        "{dog-ksize          |100   | DoG gaussian kernel size}"
        "{dog-sigma1         |5.    | DoG gaussian kernel sigma1}"
        "{dog-sigma2         |10.   | Dog gaussian kernel sigma2}"
        "{canny-enable       |true  | enable edge detection (8 bit only)}"
        "{canny-ksize        |29    | canny gaussian kernel size}"
        "{canny-sigma        |20.   | canny gaussian kernel sigma}"
        "{canny-alpha        |0.    | canny alpha param}"
        "{canny-beta         |1.    | canny beta param}"
    ;

    CommandLineParser parser(argc, argv, keys);
    parser.about("RAPID-AF demo");

    String image1String = parser.get<String>("@image1");
    String image2String = parser.get<String>("@image2");

    if (parser.has("help") || image1String == "" || image2String == "") {
        parser.printMessage();
        exit(EXIT_FAILURE);
    }

    Mat image1 = imread(image1String, IMREAD_ANYDEPTH);
    Mat image2 = imread(image2String, IMREAD_ANYDEPTH);

    struct Options opt;
    opt.multiThreading = parser.get<bool>("mt-enable");
    opt.padding = parser.get<int>("padding");
    opt.agreementThreshold = parser.get<int>("agreement");

    opt.bin_enable = parser.get<bool>("bin-enable");
    opt.bin_thresholdPercentage = parser.get<double>("bin-threshold-perc");

    opt.dog_enable = parser.get<bool>("dog-enable");
    opt.dog_ksize = parser.get<double>("dog-ksize");
    opt.dog_sigma1 = parser.get<double>("dog-sigma1");
    opt.dog_sigma2 = parser.get<double>("dog-sigma2");

    opt.canny_enable = parser.get<bool>("canny-enable");
    opt.canny_ksize = parser.get<int>("canny-ksize");
    opt.canny_sigma = parser.get<double>("canny-sigma");
    opt.canny_alpha = parser.get<double>("canny-alpha");
    opt.canny_beta = parser.get<double>("canny-beta");

    auto begin = chrono::steady_clock::now();

    double stdVarThreshold = parser.get<double>("q-std");
    double sRatioThreshold = parser.get<double>("q-sratio");
    int radius = parser.get<int>("q-sratio-radius");
    int thickness = parser.get<int>("q-sratio-thickness");

    if (parser.get<bool>("q")) {
        if (!checkImageQuality(image1, stdVarThreshold, sRatioThreshold, radius, thickness)
            || !checkImageQuality(image2, stdVarThreshold, sRatioThreshold, radius, thickness)) {
            cerr << "Error: Low image quality (-q option)" << endl;
            exit(EXIT_FAILURE);
        }
    }

    Mat i1, i2;
    if (parser.get<bool>("pf-enable")) {
        Mat filter;
        int ksize = parser.get<int>("pf-ksize");
        double sigma = parser.get<double>("pf-sigma");
        mulTransposed(getGaussianKernel(ksize, sigma), filter, false);
        filter2D(image1, i1, -1, filter);
        filter2D(image2, i2, -1, filter);
    } else {
        i1 = image1;
        i2 = image2;
    }

    bool ok;

    Point2f shift = align(i1, i2, opt, &ok);

    auto end = chrono::steady_clock::now();

    cout << "Shift: " << shift << endl;
    cerr << "Elapsed time: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "ms" << endl;
    if (!ok) {
        cerr << "Agreement threshold not met" << endl;
    }

    String output = parser.get<String>("output");
    if (output != "") {
        imwrite(output, merge(image1, image2, shift));
        cerr << "Saved debug image to " << output << endl;
    }

    return 0;
}
