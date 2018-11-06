#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>
#include <direct.h>
#include <opencv2\opencv.hpp>

class ClosedCv {
     public:
        static cv::String Type2String(int type);
        static void LogMat(cv::Mat mat);
        static cv::Mat3b LoadImage(cv::String imagePath);
        static cv::Mat RunKMeans(cv::Mat winnie, int k);
        static cv::Mat AddDottedBackground(cv::Mat source, cv::Mat ignoreWhereWhite = cv::Mat::zeros(0, 0, CV_8U));
        static cv::Mat RunFloodFill(cv::Mat image, cv::Mat floodFill, cv::Point point);
};