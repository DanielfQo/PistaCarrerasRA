#include "vision/gesture_recognition.h"
#include <vector>

void VisionProcessor::processHand(const Mat& inFrame) {

    // hsv
    cvtColor(inFrame, hsv, COLOR_BGR2HSV);

    Scalar lower(0, 58, 89);
    Scalar upper(25, 173, 255);

    Mat mask;
    inRange(hsv, lower, upper, mask);

    // procesamiento previo
    GaussianBlur(mask, mask, Size(5, 5), 0);
    erode(mask, mask, Mat(), Point(-1, -1), 2);
    erode(mask, mask, Mat(), Point(-1, -1), 2);
    dilate(mask, mask, Mat(), Point(-1, -1), 2);

    vector<vector<Point>> contours;
    findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    outFrame = inFrame.clone();

    if (contours.empty()) {
        putText(outFrame, "No hand detected", Point(20, 30), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 255), 2);
        return;
    }

    int maxIdx = 0;
    double maxArea = 0;
    for (int i = 0; i < contours.size(); i++) {

        double area = contourArea(contours[i]);
        if (area > maxArea) {
            maxArea = area;
            maxIdx = i;
        }
    }

    contour = contours[maxIdx];
    drawContours(outFrame, contours, (int)maxIdx, Scalar(0, 255, 0), 2);

    // procesamiento de convex hull
    convexHull(contour, hull);
    polylines(outFrame, hull, true, Scalar(255, 0, 0), 2);

    // procesamiento de fitting line
    fitLine(contour, fittingLine, DIST_L2, 0, 0.01, 0.01);
    Point2f pointOnLine(fittingLine[2], fittingLine[3]);
    Point2f direction(fittingLine[0], fittingLine[1]);

    float length = 200.0f;
    Point2f p1 = pointOnLine - direction * length;
    Point2f p2 = pointOnLine + direction * length;
 
    line(outFrame, 
        Point(cvRound(p1.x), cvRound(p1.y)),
        Point(cvRound(p2.x), cvRound(p2.y)),
        Scalar(0, 255, 255), 2);

    this->classifyHand();
}

double VisionProcessor::calculateSolidity() const {
    // no hay nada
    if (hull.empty()) return 0; 

    double areaHull = contourArea(hull);
    double areaContour = contourArea(contour);

    return areaContour / areaHull;
}

double VisionProcessor::calculateAngle() const {
    if (fittingLine == Vec4f()) return 0.0f;

    double angleRad = atan2(fittingLine[1], fittingLine[0]);
    double angleDeg = angleRad * 180.0f / CV_PI;
    return angleDeg;
}

double VisionProcessor::calculateAspect() const{
    if (contour.empty()) return 0.0f;

    Rect bound = boundingRect(contour);

    return bound.width / bound.height;
    // cuadrado = 1, rectangulo < 1
}

double VisionProcessor::calculateDefects() const {

    if (contour.empty() || hull.empty()) return 0;

    // indices del hull
    vector<int> hullIndices;
    convexHull(contour, hullIndices, false, false);

    // Calcular defectos
    vector<Vec4i> defects;
    convexityDefects(contour, hullIndices, defects);

    return defects.size();
}

void VisionProcessor::classifyHand() {

    double solidity = calculateSolidity();
    cout << "solidity" << solidity << endl;

    float aspect = calculateAspect();
    cout << "aspect" << aspect << endl;

    int defects = calculateDefects();
    cout << "defects" << defects << endl;


    if (defects >= 20 && solidity < 0.90 && aspect < 0.75) {
        putText(outFrame, "HAND OPEN (ADVANCE)", Point(20, 30),
            FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 2);
    }
    else {
        putText(outFrame, "HAND CLOSED (STOP)", Point(20, 30),
            FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 255), 2);
    }
}

