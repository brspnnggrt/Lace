#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <direct.h>
#include <opencv2\opencv.hpp>

cv::Mat3b loadImage(cv::String imagePath) 
{
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);   // Read the file

    if (image.empty()) //check whether the image is loaded or not
    {
        std::cout << "Error : Image cannot be loaded..!!" << std::endl;
    }

    if(! image.data ) // Check for invalid input
    {
        std::cout <<  "Could not open or find the image" << std::endl;
    }
    return image;
}

int main(int argc,char** argv)
{
    // if( argc != 3)
    // {
    //     std::cout <<" Usage: lace.exe image" << std::endl;
    //     return -1;
    // }

    // Load image and set add alpha channel
    cv::Mat winnie = loadImage("winnie.png");
    cv::cvtColor(winnie, winnie, cv::COLOR_BGR2RGBA);

    // Create white transparent matrix
    cv::Mat transparent_img = cv::Mat4b::zeros(winnie.size().height, winnie.size().width);

    for (int r = 0; r < 600; r++) {
        for (int c = 0; c < 300; c++) {
            transparent_img.at<cv::Vec4b>(r, c) = cv::Vec4b(0, 255, 0, 255);
        }
    }

    for (int r = 0; r < winnie.size().height; r++) {
        for (int c = 0; c < winnie.size().width; c++) {
            cv::Vec4b coverpoint = transparent_img.at<cv::Vec4b>(r, c);
            if (coverpoint[3] == 255) 
            {
                winnie.at<cv::Vec4b>(r, c) = coverpoint;
            }
        }
    }

    // cv::Mat3b icon = loadImage("icon.png");

    // // Create a circular mask
    // cv::Mat1b mask(775, 600, uchar(0));
    // cv::circle(mask, cv::Point(250, 250), 500, cv::Scalar(255), cv::FILLED);

    // // Or load your own mask
    // //Mat1b mask = imread("path_to_mask", IMREAD_GRAYSCALE)

    // // Compute number of repetition in x and y
    // int nx = (mask.cols / icon.cols) + 1;
    // int ny = (mask.rows / icon.rows) + 1;

    // // Create repeated pattern
    // cv::Mat3b repeated = repeat(icon, ny, nx);

    // // Crop to meet mask size
    // cv::Mat3b crop = repeated(cv::Rect(0, 0, mask.cols, mask.rows));

    // // Create a white image same size as mask
    // cv::Mat3b result = cv::Mat3b(mask.rows, mask.cols, cv::Vec3b(255, 255, 255));

    // // Copy pattern with the mask
    // crop.copyTo(result, mask);

    // namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.
    // imshow("Display window", result); // Show our image inside it.

    // namedWindow("Display window3", cv::WINDOW_AUTOSIZE);// Create a window for display.
    // imshow("Display window3", winnie); // Show our image inside it.

    // cv::Mat output;
    
    // cv::add(winnie, transparent_img, output);

    namedWindow("Display window2", cv::WINDOW_AUTOSIZE);// Create a window for display.
    imshow("Display window2", winnie); // Show our image inside it.

    cv::waitKey(0); // Wait for a keystroke in the window

    return 0;
}
