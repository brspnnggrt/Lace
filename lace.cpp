#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <direct.h>
#include <opencv2\opencv.hpp>
#include <vector>
#include <opencv2\core.hpp>

cv::String type2str(int type) {
  cv::String r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}

void logMat(cv::Mat mat) 
{
    cv::String ty = type2str(mat.type());
    printf("Matrix: %s %dx%d \n", ty.c_str(), mat.cols, mat.rows );
}

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

cv::Mat runkMeans(cv::Mat winnie, int k) 
{
    cv::Mat samples = winnie.reshape(1, winnie.rows* winnie.cols); // change to a Nx3 column vector
    samples.convertTo(samples, CV_32FC3, 1.0/255.0); // convert to floating point
    logMat(samples);

    cv::Mat labels, centers;
    cv::TermCriteria criteria = cv::TermCriteria(CV_TERMCRIT_EPS|CV_TERMCRIT_ITER, 10, 1.0);

    cv::kmeans(samples, k, labels, criteria, 4, cv::KMEANS_PP_CENTERS, centers);

    int colors[k];
    for (int i=0; i < sizeof(colors) / sizeof(colors[0]); i++) {
        colors[i] = i * (255 / k);
    }

    cv::Mat clustered = cv::Mat(winnie.rows, winnie.cols, CV_32F);
    for (int i=0; i < winnie.cols * winnie.rows; i++) {
        clustered.at<float>(i / winnie.cols, i % winnie.cols) = (float)(colors[labels.at<int>(0,i)]);
    }

    clustered.convertTo(clustered, CV_8U);

    return clustered;
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
    // Load image
    cv::Mat winnie = loadImage("winnie.png");
    logMat(winnie);

    // Run kmeans
    cv::Mat clustered = runkMeans(winnie, 2);

    logMat(clustered);

    cv::imshow("clustered", clustered);

    cv::Mat floodImage = clustered.clone();
    cv::Mat floodMask = cv::Mat::zeros(cv::Size(floodImage.size().width + 2, floodImage.size().height + 2), CV_8U);
    cv::Point seedPoint = cv::Point(0, 0);
    cv::Rect tolerance = cv::Rect(0, 0, 0, 0);
    cv::floodFill(floodImage, floodMask, seedPoint, cv::Scalar(1), &tolerance, cv::Scalar(0), cv::Scalar(0), CV_FLOODFILL_FIXED_RANGE);

    cv::imshow("test", floodImage);

    // Add background

    // cv::Mat winnieDotted = winnie.clone();

    // int spacing_rows = 16;
    // int spacing_columns = 32;
    // int size = 2;

    // for (int r = 0; r < winnieDotted.size().height; r++)
    // {
    //     for (int c = 0; c < winnieDotted.size().width; c++)
    //     {
    //         if (r % spacing_rows == 0)
    //         {
    //             if (c % spacing_columns == 0)
    //             {
    //                 cv::circle(winnieDotted, cv::Point(c + spacing_columns / 2, r + spacing_rows / 2), size, cv::Scalar(0, 0, 0, 255), -1, cv::LINE_AA);
    //             }
    //         }
    //         if (r % spacing_rows == spacing_rows / 2)
    //         {
    //             if (c % spacing_columns == spacing_columns / 2)
    //             {
    //                 cv::circle(winnieDotted, cv::Point(c + spacing_columns / 2, r + spacing_rows / 2), size, cv::Scalar(0, 0, 0, 255), -1, cv::LINE_AA);
    //             }
    //         }
    //     }
    // }

    // cv::Mat labelsImg = labels.reshape(1, winnie.size().height);

    // namedWindow("Display window", cv::WINDOW_AUTOSIZE); // Create a window for display.
    // imshow("Display window", labelsImg);             // Show our image inside it.

    // namedWindow("Display window2", cv::WINDOW_AUTOSIZE); // Create a window for display.
    // imshow("Display window2", winnie);             // Show our image inside it.

    cv::setMouseCallback("Display window", onMouse, (void*)&winnie);

    cv::waitKey(0); // Wait for a keystroke in the window

    return 0;
}
