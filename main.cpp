#include <chrono>

#include <opencv2/opencv.hpp>

#include <rapid-af.h>


using namespace cv;
using namespace std;
using namespace rapid_af;


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

    struct AlignOptions opt;
    opt.multithreading_enable = parser.get<bool>("mt-enable");
    opt.padding = parser.get<int>("padding");
    opt.agreement_threshold = parser.get<int>("agreement");

    opt.prefilter_enable = parser.get<bool>("pf-enable");
    opt.prefilter_ksize = parser.get<int>("pf-ksize");
    opt.prefilter_sigma = parser.get<double>("pf-sigma");

    opt.bin_enable = parser.get<bool>("bin-enable");
    opt.bin_threshold = parser.get<double>("bin-threshold-perc");

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

    if (parser.get<bool>("q")) {
        ImageQualityOptions iqOpt;

        iqOpt.sigma_threshold = parser.get<double>("q-std");
        iqOpt.sratio_threshold = parser.get<double>("q-sratio");
        iqOpt.sratio_radius = parser.get<int>("q-sratio-radius");
        iqOpt.sratio_thickness = parser.get<int>("q-sratio-thickness");

        if (!checkImageQuality(image1, iqOpt)|| !checkImageQuality(image2, iqOpt)) {
            cerr << "Error: Low image quality (-q option)" << endl;
            exit(EXIT_FAILURE);
        }
    }

    bool ok;

    Point2f shift = align(image1, image2, opt, &ok);

    auto end = chrono::steady_clock::now();

    cout << "Shift: " << shift << endl;
    cerr << "Elapsed time: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "ms" << endl;
    if (!ok) {
        cerr << "Agreement threshold not met" << endl;
    }

    String output = parser.get<String>("output");
    if (output != "") {
        imwrite(output, rapid_af::merge(image1, image2, shift));
        cerr << "Saved debug image to " << output << endl;
    }

    return 0;
}
