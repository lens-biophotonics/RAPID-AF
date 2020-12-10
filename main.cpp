#include <chrono>

#include <opencv2/opencv.hpp>

#include "rapid-af.h"


using namespace cv;
using namespace std;


void alignAndMerge(Mat orig1, Mat orig2, Mat image1, Mat image2, string suffix)
{
    chrono::steady_clock::time_point begin = chrono::steady_clock::now();
    Mat output = crossCorr(image1, image2, 100);
    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    cout << "Time difference CC = " << chrono::duration_cast<chrono::microseconds>(end - begin).count() / 1000 << "ms" << endl;

    imwrite("ccopencv_" + suffix + ".tiff", output);

    double min, max;
    Point maxLoc;
    Point upperLeft(100, 100);
    cv::minMaxLoc(output, &min, &max, nullptr, &maxLoc);

    Point shift = maxLoc - upperLeft;

    cout << suffix << " " << "shift: " << shift.x << " " << shift.y << endl;


    Mat M = Mat(2, 3, CV_64FC1); // Allocate memory

    M.at<double>(0, 0) =  1;  //p1
    M.at<double>(1, 0) =  0;  //p2;
    M.at<double>(0, 1) = 0; //p3;
    M.at<double>(1, 1) = 1;  //p4;
    M.at<double>(0, 2) = shift.x;   //p5;
    M.at<double>(1, 2) = shift.y; //p6;

    Mat shifted;

    Mat channels[3];
    channels[0] = Mat::zeros(orig1.size(), CV_8UC1);
    warpAffine(orig2, channels[1], M, orig2.size());
    channels[1].convertTo(channels[1], CV_8UC1);
    orig1.convertTo(channels[2], CV_8UC1);

    Mat merged;
    merge(channels, 3, merged);

    imwrite("merged_" + suffix + ".tiff", merged);
    imwrite(suffix + "_1.tiff", image1);
    imwrite(suffix + "_2.tiff", image2);
}


int main(int argc, char** argv)
{
    if (argc != 4)
    {
        printf("usage: phase_detector <Image_Path> <Image_Path2> threshold\n");
        return -1;
    }

    Mat image1 = imread(argv[1], IMREAD_ANYDEPTH);
    Mat image2 = imread(argv[2], IMREAD_ANYDEPTH);

    Mat filter;
    mulTransposed(getGaussianKernel(20, 5), filter, false);
    filter2D(image1, image1, -1, filter);
    filter2D(image2, image2, -1, filter);

    chrono::steady_clock::time_point begin = chrono::steady_clock::now();

    Mat bin1, bin2;
    bin1 = binarize(image1, atof(argv[3]));
    bin2 = binarize(image2, atof(argv[3]));

    alignAndMerge(image1, image2, bin1, bin2, "bin");

    Mat dog1, dog2;
    dog1 = dog(image1, 100, 5, 10);
    dog2 = dog(image2, 100, 5, 10);

    alignAndMerge(image1, image2, dog1, dog2, "dog");

    Mat edge1, edge2;
    edge1 = canny(image1, 29, 20, 0, 1);
    edge2 = canny(image2, 29, 20, 0, 1);

    alignAndMerge(image1, image2, edge1, edge2, "edge");

    chrono::steady_clock::time_point end = chrono::steady_clock::now();
    cout << "Time difference total = " << chrono::duration_cast<chrono::microseconds>(end - begin).count() / 1000 << "ms" << endl;

    begin = chrono::steady_clock::now();
    struct Options opt;
    Point2f shift = align(image1, image2, opt);
    cout << "shift: " << shift.x << " " << shift.y << endl;
    end = chrono::steady_clock::now();
    cout << "Time difference total = " << chrono::duration_cast<chrono::microseconds>(end - begin).count() / 1000 << "ms" << endl;

    return 0;
}
