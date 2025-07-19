// marker_detection.h
#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <vector>
#include <string>

struct PoseData {
    cv::Mat rvec;
    cv::Mat tvec;
    bool poseValida = false;
};

constexpr int warpSize = 200;
constexpr int totalGrid = 6;
constexpr int internalGrid = 4;

extern const std::vector<std::vector<int>> matriz0;

int contarDiferencias(const std::vector<std::vector<int>>& a, const std::vector<std::vector<int>>& b);
std::vector<std::vector<int>> rotar90(const std::vector<std::vector<int>>& mat);
int detectarOrientacion(const std::vector<std::vector<int>>& bits, const std::vector<std::vector<int>>& ref);
void rotarOrderedPts(std::vector<cv::Point2f>& pts, int angulo);
bool validarBordeNegro(const cv::Mat& warpBin);
void ordenarPuntos(const std::vector<cv::Point2f>& srcPts, std::vector<cv::Point2f>& orderedPts);
std::vector<std::vector<int>> extraerBits(const cv::Mat& warpBin);

void poseToModelViewMatrix(const cv::Mat& rvec, const cv::Mat& tvec, float modelViewMatrix[16]);
void cameraMatrixToGLProjection(const cv::Mat& cameraMatrix, float width, float height, 
                               float nearPlane, float farPlane, float projectionMatrix[16]);

bool obtenerPoseDelMarcador(const std::vector<cv::Point2f>& imagenPts, const cv::Mat& cameraMatrix,
                           const cv::Mat& distCoeffs, cv::Mat& rvec, cv::Mat& tvec);
void procesarFrame(cv::Mat& frame, const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs, PoseData& poseData);

bool captureCalibrationImages(int num_images = 20, const std::string& filename_prefix = "calibrate/calib_", 
                             int g_cameraIndex = 0);
double calibrateCameraFromImages(const std::string& images_path, const cv::Size& boardSize, float squareSize,
                                cv::Mat& cameraMatrix, cv::Mat& distCoeffs, bool showCorners = true);

void mostrarMenu();
void ejecutarDeteccion(const cv::Mat& cameraMatrix = cv::Mat(), const cv::Mat& distCoeffs = cv::Mat(), 
                      int g_cameraIndex = 0);
void ejecutarMenu(int cameraIndex = 0);