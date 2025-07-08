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

        // procesar mano
        vp.processHand(frame);

        // Mostrar las ventanas
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
