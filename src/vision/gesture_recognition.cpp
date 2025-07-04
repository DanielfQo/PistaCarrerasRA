#include "vision/gesture_recognition.h"
#include <vector>

using namespace cv;
using namespace std;

void VisionProcessor::processHand(const Mat& inputFrame, Mat& outputFrame) {
    // Convertir a HSV
    Mat hsv;
    cvtColor(inputFrame, hsv, COLOR_BGR2HSV);

    // Rango de color piel aproximado (ajusta según condiciones)
    Scalar lower(0, 30, 60);
    Scalar upper(20, 150, 255);

    Mat mask;
    inRange(hsv, lower, upper, mask);

    // Suavizar
    GaussianBlur(mask, mask, Size(5, 5), 0);
    erode(mask, mask, Mat(), Point(-1, -1), 2);
    dilate(mask, mask, Mat(), Point(-1, -1), 2);

    // Encontrar contornos
    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    // Copiar frame para dibujar
    outputFrame = inputFrame.clone();

    if (contours.empty()) {
        putText(outputFrame, "No hand detected", Point(20, 30), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 255), 2);
        return;
    }

    // Tomar el contorno más grande
    size_t maxIdx = 0;
    double maxArea = 0;
    for (size_t i = 0; i < contours.size(); i++) {
        double area = contourArea(contours[i]);
        if (area > maxArea) {
            maxArea = area;
            maxIdx = i;
        }
    }

    const vector<Point>& handContour = contours[maxIdx];

    // Dibujar contorno
    drawContours(outputFrame, contours, (int)maxIdx, Scalar(0, 255, 0), 2);

    // Calcular convex hull
    vector<Point> hull;
    convexHull(handContour, hull);

    // Dibujar convex hull
    polylines(outputFrame, hull, true, Scalar(255, 0, 0), 2);

    // Fitting line
    Vec4f line;
    fitLine(handContour, line, DIST_L2, 0, 0.01, 0.01);

    Point2f pointOnLine(line[2], line[3]);
    Point2f direction(line[0], line[1]);

    float length = 200.0f;
    Point2f p1 = pointOnLine - direction * length;
    Point2f p2 = pointOnLine + direction * length;
 
    // Dibuja la línea con Point
    cv::line(outputFrame, Point(cvRound(p1.x), cvRound(p1.y)),
        Point(cvRound(p2.x), cvRound(p2.y)),
        Scalar(0, 255, 255), 2);

}
