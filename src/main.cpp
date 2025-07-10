#include <opencv2/opencv.hpp>
#include "vision/gesture_recognition.h"
#include "marker_detection.h"

using namespace cv;

int main() {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "No se pudo abrir la cámara.\n";
        return -1;
    }

    VisionProcessor vp;

    // ⚙️ Calibración provisional de cámara
    Mat cameraMatrix = (Mat_<double>(3, 3) << 800, 0, 320, 0, 800, 240, 0, 0, 1);
    Mat distCoeffs = Mat::zeros(5, 1, CV_64F);

    while (true) {
        Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        // 🎯 Procesar detección de marcadores
        procesarMarcadores(frame, cameraMatrix, distCoeffs);

        // ✋ Procesar gesto de mano (usa copia para que no interfiera)
        vp.processHand(frame);

        // 🖼 Mostrar resultados
        imshow("Camara Normal", frame);
        imshow("Vision Mano", vp.outFrame);
        imshow("HSV", vp.hsv);

        if ((char)waitKey(30) == 27) break;  // ESC para salir
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
