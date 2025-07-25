#include "../../include/vision/gesture_recognition.h"
#include <vector>

VisionProcessor::VisionProcessor() {
    solidity = 0; defects = 0; aspect = 0; angle = 0;
    handDetected = false;
}

void VisionProcessor::processHand(const Mat& inFrame) {

    // hsv
    cvtColor(inFrame, hsv, COLOR_BGR2HSV);

    Scalar lower(0, 20, 70);
    Scalar upper(20, 255, 255);

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

    //this->classifyHand();
}

double VisionProcessor::calculateSolidity() const {
    // no hay nada
    if (hull.empty()) return 0.0; 

    double areaHull = contourArea(hull);
    double areaContour = contourArea(contour);

    return areaContour / areaHull;
}

double VisionProcessor::calculateAngle() const {
    if (fittingLine == Vec4f()) return 0.0;

    double angleRad = atan2(fittingLine[1], fittingLine[0]);
    double angleDeg = angleRad * 180.0f / CV_PI;
    return angleDeg;
}

double VisionProcessor::calculateAspect() const{
    if (contour.empty()) return 0.0;

    Rect bound = boundingRect(contour);

    return bound.width / bound.height;
    // cuadrado = 1, rectangulo < 1
}

double VisionProcessor::calculateDefects() const {

    if (contour.empty() || hull.empty()) return 0.0;

    // indices del hull
    vector<int> hullIndices;
    convexHull(contour, hullIndices, false, false);

    // calcular defectos
    vector<Vec4i> defects;
    convexityDefects(contour, hullIndices, defects);

    return defects.size();
}

void VisionProcessor::classifyHand() {
    double solidity = calculateSolidity(); // avanzar si < 90
    double aspect = calculateAspect(); // solo detecta el puño, pero tambien devuelve 1 si es que la mano esta muy abierta, error
    double defects = calculateDefects(); // a veces es puño cuando los defectos son mayores a 31
    double angle = calculateAngle();

    double result = 0.0;

    double solidityCont = min(1.0, max(0.0, (solidity - 0.85) / (1.0 - 0.85)));
    double defectsCont = 1.0 - min(1.0, defects / 10.0);
    double aspectCont = aspect;  // Ahora aspect está en [0,1]

    // Pesos ajustados (mayor peso a solidity)
    const double SOLIDITY_WEIGHT = 0.7;
    const double DEFECTS_WEIGHT = 0.2;
    const double ASPECT_WEIGHT = 0.1;

    result = (solidityCont * SOLIDITY_WEIGHT) + 
            (defectsCont * DEFECTS_WEIGHT) + 
            (aspectCont * ASPECT_WEIGHT);

    const double FIST_THRESHOLD = 0.5;
    bool isFist = (result >= FIST_THRESHOLD);

    const int var1 = 50, var2 = 90;
    
    if (isFist) {
        putText(outFrame, "STOP", Point(20,60), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0,0,255), 2);
    }
    else {
        if ((angle > var1 && angle < var2) || (angle > -var2 && angle < -var1)) {
            putText(outFrame, "ADVANCE (W)", Point(20,60), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0,255,0), 2);
        } 
        else if (angle > 0 && angle <= var1) {
            putText(outFrame, "ADVANCE LEFT (A)", Point(20,60), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0,255,200), 2);
        } 
        else if (angle > -var1 && angle <= 0) {
            putText(outFrame, "ADVANCE RIGHT (D)", Point(20,60), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(200,255,0), 2);
        }
    }

    // info
    putText(outFrame, "Angle: " + to_string(int(angle)), 
            Point(20, 90), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(200,200,0), 1);

    putText(outFrame, "Solidity: " + to_string(solidity).substr(0,4) + 
           " (" + to_string(int(solidityCont*100)) + "%)", 
            Point(20, 120), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(200,200,0), 1);

    putText(outFrame, "Defects: " + to_string(defects) + 
           " (" + to_string(int(defectsCont*100)) + "%)", 
            Point(20, 150), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(200,200,0), 1);

    putText(outFrame, "Aspect: " + to_string(aspect).substr(0,4) + 
           " (" + to_string(int(aspectCont*100)) + "%)", 
            Point(20, 180), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(200,200,0), 1);

    putText(outFrame, "Result: " + to_string(result), 
            Point(20, 210), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255,150,0), 1);

}

void VisionProcessor::update() {
    handDetected = !contour.empty() && !hull.empty();
    if (!handDetected) return;

    solidity = calculateSolidity();
    defects = calculateDefects();
    aspect = calculateAspect();
    angle = calculateAngle();
}

bool VisionProcessor::isStop() const {
    if (!handDetected) return false;

    double solidityCont = min(1.0, max(0.0, (solidity - 0.85) / (1.0 - 0.85)));
    double defectsCont = 1.0 - min(1.0, defects / 10.0);
    double aspectCont = aspect;

    double result = (solidityCont * 0.7) + (defectsCont * 0.2) + (aspectCont * 0.1);

    return result >= 0.5;
}

bool VisionProcessor::isAdvance() const {
    if (!handDetected || isStop()) return false;
    int var1 = 50, var2 = 90;

    return ( (angle > var1 && angle < var2) || (angle > -var2 && angle < -var1) );
}

bool VisionProcessor::isLeft() const {
    if (!handDetected || isStop()) return false;
    int var = 50;

    return (angle > 0 && angle <= var);
}

bool VisionProcessor::isRight() const {
    if (!handDetected || isStop()) return false;
    int var = 50;

    return (angle > -var && angle <= 0);
}
