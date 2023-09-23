#include <iostream>
#include <fstream>
#include <tuple>
#include <opencv2/opencv.hpp>

// Minimum area threshold
static const int MIN_AREA = 9000;

// Scalar values defining lower and upper bounds of orange color
const cv::Scalar lowerOrange(0, 100, 100);
const cv::Scalar upperOrange(15, 255, 255);
// Scalar values defining lower and upper bounds of yellow color
const cv::Scalar lowerYellow(20, 100, 100); 
const cv::Scalar upperYellow(40, 255, 255);
// Scalar values defining lower and upper bounds of blue color
const cv::Scalar lowerBlue(90, 50, 100);
const cv::Scalar upperBlue(200, 255, 255);
// Scalar values defining lower and upper bounds of white color
const cv::Scalar lowerWhite(0, 0, 220);
const cv::Scalar upperWhite(255, 60, 255);

/**
 * Splits the corrupted image into two parts: top and bottom
 * 
 * @param input_image The input image to be separated
 * @return A tuple containing two cv::Mat objects, representing the top and bottom parts of the image.
*/
std::tuple<cv::Mat, cv::Mat> separate_images(cv::Mat input_image);
/**
 * It takes the 2 separated images and combines them into one correctly
 * 
 * @param top The top part of the original image
 * @param bottom the bottom part of the original image
 * @param combined_image the combined image variable to use
*/
void combine_images(const cv::Mat& top, const cv::Mat& bottom, cv::Mat& combined_image);
/**
 * Fixes the chromatic aberration on the corrupted part of the image as GoodImage.jpg
 * 
 * @param image The input image to be corrected
*/
void correct_chromatic_aberration(cv::Mat &image);
/**
 * Draws the rectangle around the 3 cones and then writes their coordinates in the cone_coordinates.txt file
 * 
 * @param image The input image to be corrected
*/
void draw_boundaries(cv::Mat image);