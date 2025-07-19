#include <opencv2/opencv.hpp>
#include "vision/gesture_recognition.h"

using namespace cv;

int main() {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        printf("Error abriendo la camara\n");
        return -1;
    }

    Mat frame;
    VisionProcessor vp;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        vp.processHand(frame);
        vp.update();

        if (!vp.handDetected) {
            putText(vp.outFrame, "No hand detected", Point(20, 60), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0,0,255), 2);
        } else if (vp.isStop()) {
            putText(vp.outFrame, "STOP", Point(20, 60), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0,0,255), 2);
        } else if (vp.isAdvance()) {
            putText(vp.outFrame, "ADVANCE (W)", Point(20, 60), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0,0,255), 2);
        } else if (vp.isLeft()) {
            putText(vp.outFrame, "ADVANCE LEFT (A)", Point(20, 60), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0,0,255), 2);
        } else if (vp.isRight()) {
            putText(vp.outFrame, "ADVANCE RIGHT (D)", Point(20, 60), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0,0,255), 2);
        }

        imshow("Camara Normal", frame);
        imshow("Vision Mano", vp.outFrame);
        imshow("HSV", vp.hsv);

        char key = (char)waitKey(30);
        if (key == 27) break; // ESC para salir
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
