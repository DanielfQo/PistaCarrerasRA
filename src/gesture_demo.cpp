/*#include <opencv2/opencv.hpp>
#include "vision/gesture_recognition.h"

using namespace cv;

int main() {
    VideoCapture cap(0); // Usa la cámara predeterminada
    if (!cap.isOpened()) {
        std::cerr << "No se pudo abrir la cámara\n";
        return -1;
    }

    Mat frame;
    VisionProcessor vp;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        vp.processHand(frame);

        imshow("Camara Original", frame);
        imshow("Salida Procesada", vp.outFrame);
        imshow("HSV", vp.hsv);

        char key = (char)waitKey(30);
        if (key == 27) break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
*/