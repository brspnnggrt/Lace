#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <direct.h>
#include <opencv2\opencv.hpp>
#include <vector>

cv::Mat floodFill;

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

cv::Mat addDottedBackground(cv::Mat source, cv::Mat ignoreWhereWhite = cv::Mat::zeros(0, 0, CV_8U)) 
{
    if (source.size() != ignoreWhereWhite.size()) {
        ignoreWhereWhite = cv::Mat::zeros(source.size(), CV_8U);
    }

    cv::Mat image = source.clone();

    int spacing_rows = 16;
    int spacing_columns = 32;
    int size = 2;

    bool drawCircle = false;

    for (int r = 0; r < image.size().height; r++)
    {
        for (int c = 0; c < image.size().width; c++)
        {
            drawCircle = false;

            if (r % spacing_rows == 0)
                if (c % spacing_columns == 0)
                    drawCircle = true;

            if (r % spacing_rows == spacing_rows / 2)
                if (c % spacing_columns == spacing_columns / 2)
                    drawCircle = true;

            if (drawCircle) 
            {
                cv::Point point = cv::Point(c + spacing_columns / 2, r + spacing_rows / 2);
                if (ignoreWhereWhite.at<uchar>(point) != 255)
                    cv::circle(image, point, size, cv::Scalar(0, 0, 255, 255), -1, cv::LINE_AA);
            }
        }
    }

    return image;
}

void onMouse(int event, int x, int y, int, void* param) 
{
    if (event != cv::EVENT_LBUTTONDOWN)
        return;

    cv::Mat* image = (cv::Mat*)param;

    cv::Point seedPoint = cv::Point(x, y);
    cv::Mat floodImage = floodFill;
    cv::Mat floodMask = cv::Mat::zeros(cv::Size(floodImage.size().width + 2, floodImage.size().height + 2), CV_8U);
    cv::Rect tolerance = cv::Rect(0, 0, 0, 0);
    cv::floodFill(floodImage, floodMask, seedPoint, cv::Scalar(255), &tolerance, cv::Scalar(0), cv::Scalar(0), CV_FLOODFILL_FIXED_RANGE);

    cv::Mat result = addDottedBackground(*image, floodImage);

    // cv::imshow("flood", floodImage);
    cv::imshow("dotted", result);
};

int main(int argc, char **argv)
{
    // Load image
    cv::Mat winnie = loadImage("winnie.png");
    logMat(winnie);

    // Run kmeans
    cv::Mat clustered = runkMeans(winnie, 2);

    logMat(clustered);

    cv::imshow("source", winnie);

    // Add initial dotted background
    cv::Mat dotted = addDottedBackground(winnie);

    cv::imshow("dotted", dotted);

    floodFill = clustered.clone();

    cv::setMouseCallback("source", onMouse, (void*)&winnie);

    cv::waitKey(0); // Wait for a keystroke in the window

    return 0;
}
