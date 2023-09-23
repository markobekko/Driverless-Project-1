#include "image_fixing.hpp"

std::tuple<cv::Mat, cv::Mat> separate_images(cv::Mat input_image){
    // Create two blank images of the same size as the input image
    cv::Mat topPart(input_image.size(), input_image.type());
    cv::Mat bottomPart(input_image.size(), input_image.type());

    // Define the region of interest for the top part (half of the height)
    cv::Rect roiTop(0, 0, input_image.cols, input_image.rows / 2);

    // Define the region of interest for the bottom part (half of the height)
    cv::Rect roiBottom(0, input_image.rows / 2, input_image.cols, input_image.rows / 2);

    // Copy the pixels to the separated images
    input_image(roiTop).copyTo(topPart);
    input_image(roiBottom).copyTo(bottomPart);

    cv::imwrite("top_part.jpg", topPart);
    cv::imwrite("bottom_part.jpg", bottomPart);

    return std::make_tuple(topPart, bottomPart);
}

void combine_images(const cv::Mat& top, const cv::Mat& bottom, cv::Mat& combined_image) {
    // Check if the input images have the same dimensions and type
    if (top.size() != bottom.size() || top.type() != bottom.type()) {
        std::cerr << "Input images must have the same size and type." << std::endl;
        return;
    }

    // Create a blank image with twice the height of the input images
    combined_image = cv::Mat(top.rows * 2, top.cols, top.type());

    // Copy the pixels from the input images to the combined image
    bottom.copyTo(combined_image(cv::Rect(0, 0, top.cols, top.rows)));
    top.copyTo(combined_image(cv::Rect(0, top.rows, bottom.cols, bottom.rows)));
}

void correct_chromatic_aberration(cv::Mat &image){
    // Split the image into upper and lower halves
    cv::Rect upperHalfRect(0, 0, image.cols, image.rows / 2);
    cv::Mat upperHalf = image(upperHalfRect);
    cv::Rect lowerHalfRect(0, image.rows / 2, image.cols, image.rows / 2);
    cv::Mat lowerHalf = image(lowerHalfRect);

    // Calculate the average color for each half
    cv::Scalar upperHalfMean = cv::mean(upperHalf);
    cv::Scalar lowerHalfMean = cv::mean(lowerHalf);

    // Compute correction factors for each channel (color correction)
    cv::Scalar colorCorrectionFactor;
    for (int i = 0; i < 3; i++) {
        colorCorrectionFactor[i] = lowerHalfMean[i] / upperHalfMean[i];
    }

    // Apply color and brightness correction to the upper half
    for (int y = 0; y < image.rows / 2; y++) {
        for (int x = 0; x < image.cols; x++) {
            cv::Vec3b& pixel = image.at<cv::Vec3b>(y, x);
            for (int i = 0; i < 3; i++) {
                // Apply color correction
                pixel[i] *= colorCorrectionFactor[i];
            }
        }
    }
    cv::imwrite("GoodImage.jpg", image);
    cv::waitKey(0);
}

void draw_boundaries(cv::Mat image){
    // Convert the image to HSV color space
    cv::Mat hsvImage;
    cv::cvtColor(image, hsvImage, cv::COLOR_BGR2HSV);

    // Create binary masks for orange and white cones
    cv::Mat maskOrange, maskWhite, combinedMask;
    cv::inRange(hsvImage, lowerOrange, upperOrange, maskOrange);
    cv::inRange(hsvImage, lowerWhite, upperWhite, maskWhite);
    cv::bitwise_or(maskOrange, maskWhite, combinedMask);

    cv::Mat maskBlue, maskYellow;

    cv::inRange(hsvImage, lowerBlue, upperBlue, maskBlue);
    cv::bitwise_or(combinedMask, maskBlue, combinedMask);
    cv::inRange(hsvImage, lowerYellow, upperYellow, maskYellow);
    cv::bitwise_or(combinedMask, maskYellow, combinedMask);

    cv::imshow("Combined", combinedMask);

    int orangeCount = cv::countNonZero(maskOrange);
    int yellowCount = cv::countNonZero(maskYellow);
    int blueCount = cv::countNonZero(maskBlue);

    // Apply morphological operations to reduce noise
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(combinedMask, combinedMask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(combinedMask, combinedMask, cv::MORPH_CLOSE, kernel);

    // Find contours in the combined mask
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(combinedMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::ofstream outputFile("cone_coordinates.txt");

    // Filter and draw the detected cones
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area > MIN_AREA) {
            cv::Rect boundingRect = cv::boundingRect(contour);
            cv::rectangle(image, boundingRect, cv::Scalar(0, 255, 0), 2);

            // Get the coordinates of the rectangle
            int minX = boundingRect.x;
            int minY = boundingRect.y;
            int maxX = boundingRect.x + boundingRect.width;
            int maxY = boundingRect.y + boundingRect.height;

            // Determine the cone color
            std::string coneColor;
            int yellowCount = cv::countNonZero(maskYellow(boundingRect));
            int blueCount = cv::countNonZero(maskBlue(boundingRect));
            int orangeCount = cv::countNonZero(maskOrange(boundingRect));

            if (yellowCount > blueCount && yellowCount > orangeCount) {
                coneColor = "Yellow";
            } else if (blueCount > yellowCount && blueCount > orangeCount) {
                coneColor = "Blue";
            } else if (orangeCount > yellowCount && orangeCount > blueCount) {
                coneColor = "Orange";
            } else {
                coneColor = "Unknown";
            }
            outputFile << coneColor << ": (" << minX << ", " << minY << ", " << maxX << ", " << maxY << ")" << std::endl;
        }
    }
    outputFile.close();
    cv::imshow("Cones Detection", image);
    cv::waitKey(0);
}