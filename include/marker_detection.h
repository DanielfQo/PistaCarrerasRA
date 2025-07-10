#ifndef MARKER_DETECTION_H
#define MARKER_DETECTION_H

#include <opencv2/opencv.hpp>

bool procesarMarcadores(cv::Mat& frame, const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs);

#endif
