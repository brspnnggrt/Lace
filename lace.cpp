#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <direct.h>
#include <opencv2\opencv.hpp>
#include <vector>
#include "closedcv.h"

cv::Mat floodFill;
cv::Mat winnie;

void onMouse(int event, int x, int y, int, void* param) 
{
    if (event != cv::EVENT_LBUTTONDOWN)
        return;

    // cv::Mat* image = (cv::Mat*)param;

    cv::Point seedPoint = cv::Point(x, y);
    cv::Mat floodImage = ClosedCv::RunFloodFill(winnie, floodFill, seedPoint);
    
    cv::Mat result = ClosedCv::AddDottedBackground(winnie, floodImage);

    cv::imshow("flood", floodImage);
    cv::imshow("dotted", result);
};

int main(int argc, char **argv)
{
    // Load image
    winnie = ClosedCv::LoadImage("winnie.png");
    ClosedCv::LogMat(winnie);

    // Run kmeans
    cv::Mat clustered = ClosedCv::RunKMeans(winnie, 2);

    ClosedCv::LogMat(clustered);

    cv::imshow("source", winnie);

    // Add initial dotted background
    cv::Mat dotted = ClosedCv::AddDottedBackground(winnie);

    cv::imshow("dotted", dotted);

    floodFill = clustered.clone();

    cv::setMouseCallback("source", onMouse, (void*)&winnie);

    cv::waitKey(0); // Wait for a keystroke in the window

    return 0;
}
