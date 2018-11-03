#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <direct.h>
#include <opencv2\opencv.hpp>
#include <vector>

cv::Mat3b loadImage(cv::String imagePath)
{
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR); // Read the file

    if (image.empty()) //check whether the image is loaded or not
    {
        std::cout << "Error : Image cannot be loaded..!!" << std::endl;
    }

    if (!image.data) // Check for invalid input
    {
        std::cout << "Could not open or find the image" << std::endl;
    }
    return image;
}

void onMouse(int event, int x, int y, int, void* param) 
{
    if (event != cv::EVENT_LBUTTONDOWN)
        return;

    cv::Mat* winnie = (cv::Mat*)param;

    cv::Point point = cv::Point(x, y);

    cv::Mat greyScale;
    cv::cvtColor(*winnie, greyScale, cv::COLOR_RGBA2GRAY);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(greyScale, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    cv::Mat dst = cv::Mat::zeros(greyScale.rows, greyScale.cols, CV_8U);

    // cv::grabCut(greyScale, dst, cv::Rect(point, cv::Point(20,30)), dst, dst, 5, cv::GC_INIT_WITH_RECT);


    // int idx = 0;
    // for (; idx >= 0; idx = hierarchy[idx][0])
    // {
    //     cv::Scalar color(rand() & 255, rand() & 255, rand() & 255);

    //     auto points = contours[idx];
    //     for (int i = 0; i < points.size(); i++)
    //     {
    //         auto currentPoint = points[i];
    //         if (currentPoint.x == point.x && currentPoint.y == point.y) 
    //         {
    //             cv::drawContours(dst, contours, idx, color, CV_FILLED, 8, hierarchy);
    //         }
    //     }
        
    //     size_t length = points.size();
    //     // if (length == 4)
    //     cv::drawContours(dst, contours, idx, color, CV_FILLED, 8, hierarchy);
    // }

    // cv::namedWindow("Original", 1);
    // cv::imshow("Original", *winnie);

    // cv::namedWindow("Components", 1);
    // cv::imshow("Components", dst);
};

int main(int argc, char **argv)
{
    // Load image and set add alpha channel
    cv::Mat winnie = loadImage("winnie.png");
    cv::cvtColor(winnie, winnie, cv::COLOR_BGR2RGBA);

    

    // if( argc != 3)
    // {
    //     std::cout <<" Usage: lace.exe image" << std::endl;
    //     return -1;
    // }

    // Create white transparent matrix
    // cv::Mat transparent_img = cv::Mat4b::zeros(winnie.size().height, winnie.size().width);

    cv::Mat winnieDotted = winnie.clone();

    int spacing_rows = 16;
    int spacing_columns = 32;
    int size = 2;

    for (int r = 0; r < winnieDotted.size().height; r++)
    {
        for (int c = 0; c < winnieDotted.size().width; c++)
        {
            if (r % spacing_rows == 0)
            {
                if (c % spacing_columns == 0)
                {
                    cv::circle(winnieDotted, cv::Point(c + spacing_columns / 2, r + spacing_rows / 2), size, cv::Scalar(0, 0, 0, 255), -1, cv::LINE_AA);
                }
            }
            if (r % spacing_rows == spacing_rows / 2)
            {
                if (c % spacing_columns == spacing_columns / 2)
                {
                    cv::circle(winnieDotted, cv::Point(c + spacing_columns / 2, r + spacing_rows / 2), size, cv::Scalar(0, 0, 0, 255), -1, cv::LINE_AA);
                }
            }
        }
    }

    // for (int r = 0; r < winnie.size().height; r++) {
    //     for (int c = 0; c < winnie.size().width; c++) {
    //         cv::Vec4b coverpoint = transparent_img.at<cv::Vec4b>(r, c);
    //         if (coverpoint[3] != 0)
    //         {
    //             winnie.at<cv::Vec4b>(r, c) = coverpoint;
    //         }
    //     }
    // }

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

    namedWindow("Display window", cv::WINDOW_AUTOSIZE); // Create a window for display.
    imshow("Display window", winnieDotted);             // Show our image inside it.

    cv::setMouseCallback("Display window", onMouse, (void*)&winnie);

    cv::waitKey(0); // Wait for a keystroke in the window

    return 0;
}
