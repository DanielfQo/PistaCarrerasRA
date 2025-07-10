#ifndef MARKER_DETECTION_H
#define MARKER_DETECTION_H

#include <opencv2/opencv.hpp>
#include <vector>

struct MarkerData {
    bool detectado = false;
    cv::Mat rvec, tvec;
    int angulo = -1;
    std::vector<std::vector<int>> bits;
};

MarkerData procesarMarcadores(cv::Mat& frame, const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs);
void ordenarPuntos(const std::vector<cv::Point2f>& srcPts, std::vector<cv::Point2f>& orderedPts);
bool validarBordeNegro(const cv::Mat& warpBin);
std::vector<std::vector<int>> extraerBits(const cv::Mat& warpBin);
int detectarOrientacion(const std::vector<std::vector<int>>& bits, const std::vector<std::vector<int>>& ref);
void rotarOrderedPts(std::vector<cv::Point2f>& pts, int angulo);
#endif
