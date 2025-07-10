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

    // Calibración provisional de cámara
    Mat cameraMatrix = (Mat_<double>(3, 3) << 800, 0, 320, 0, 800, 240, 0, 0, 1);
    Mat distCoeffs = Mat::zeros(5, 1, CV_64F);

    while (true) {
        Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        // Procesar gesto de mano
        vp.processHand(frame);
        bool manoDetectada = vp.handDetected;

        // Detectar marcador y obtener estado
        bool patronDetectado = procesarMarcadores(frame, cameraMatrix, distCoeffs);

        // Dibujar estado actual sobre el frame
        int y = 30;
        if (manoDetectada)
            putText(frame, "Mano detectada", Point(20, y), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 255, 0), 2), y += 30;
        else
            putText(frame, "Mano NO detectada", Point(20, y), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 255), 2), y += 30;

        if (patronDetectado)
            putText(frame, "Patron detectado", Point(20, y), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 0), 2);
        else
            putText(frame, "Buscando patron...", Point(20, y), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(100, 100, 100), 2);

        // Mostrar frame con overlays
        imshow("Camara RA", frame);

        if ((char)waitKey(30) == 27) break;  // ESC para salir
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
