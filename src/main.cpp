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

        vp.processHand(frame);
        bool manoDetectada = vp.handDetected;
        std::string estadoMano = vp.estadoMano;

        MarkerData marker = procesarMarcadores(frame, cameraMatrix, distCoeffs);

        int y = 30;

        putText(frame, manoDetectada ? "Mano detectada" : "Mano NO detectada",
                Point(20, y), FONT_HERSHEY_SIMPLEX, 0.8,
                manoDetectada ? Scalar(0, 255, 0) : Scalar(0, 0, 255), 2); y += 30;

        putText(frame, "Estado mano: " + estadoMano,
                Point(20, y), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(200, 255, 200), 2); y += 30;

        if (marker.detectado) {
            putText(frame, "Patron detectado", Point(20, y),
                    FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 0), 2); y += 30;

            // Mostrar información de rotación y traslación
            std::ostringstream poseInfo;
            poseInfo << "RotZ: " << std::round(marker.rvec.at<double>(2) * 100) / 100
                     << " | Z: " << std::round(marker.tvec.at<double>(2) * 100) / 100
                     << " | Ang: " << marker.angulo;
            putText(frame, poseInfo.str(), Point(20, y),
                    FONT_HERSHEY_SIMPLEX, 0.7, Scalar(255, 255, 255), 2);
            y += 30;

        } else {
            putText(frame, "Buscando patron...", Point(20, y),
                    FONT_HERSHEY_SIMPLEX, 0.8, Scalar(100, 100, 100), 2); y += 30;
        }

        // Mostrar la vista final
        imshow("Camara RA", frame);

        if ((char)waitKey(30) == 27) break;
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
