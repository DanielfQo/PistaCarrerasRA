#include <opencv2/opencv.hpp>
#include "vision/gesture_recognition.h"

using namespace cv;

int main() {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        printf("Error abriendo la camara\n");
        return -1;
    }

    Mat frame, processed;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // Procesar para convex hull y linea
        VisionProcessor::processHand(frame, processed);

        // Mostrar las ventanas
        imshow("Camara Normal", frame);
        imshow("Vision Mano", processed);

        char key = (char)waitKey(30);
        if (key == 27) break; // ESC para salir
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
