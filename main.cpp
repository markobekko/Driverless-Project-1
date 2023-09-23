#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "image_fixing.hpp"

using namespace cv;
using namespace std;

int main(int argc, char** argv )
{
    Mat input_image = imread("../../corrupted.png");
    Mat correct_image;
    cv::Mat top, bottom;
    if ( input_image.empty() ){
        std::cerr << "Error: Couldn't load the original image." << std::endl;
        return -1;
    }
    std::tie(top, bottom) = separate_images(input_image);
    combine_images(top, bottom, correct_image);
    correct_chromatic_aberration(correct_image);
    draw_boundaries(correct_image);

    waitKey(0);
    return 0;
}