#include <opencv2/opencv.hpp>
#include "vision/gesture_recognition.h"

using namespace cv;
using namespace std;

int main() {
    VideoCapture camMano(1);
    VideoCapture camPatron(0);

    if (!camMano.isOpened() || !camPatron.isOpened()) {
        cerr << "Error abriendo cámaras.\n";
        return -1;
    }

    const int widthTotal = 1280;
    const int heightTotal = 720;

    const int widthLado = widthTotal / 3;
    const int widthJuego = widthTotal - widthLado;
    const int heightCam = heightTotal / 2;

    Mat frameMano, framePatron, canvas(Size(widthTotal, heightTotal), CV_8UC3);
    VisionProcessor vp;

    while (true) {
        camMano >> frameMano;
        camPatron >> framePatron;

        if (frameMano.empty() || framePatron.empty()) break;

        vp.processHand(frameMano);
        vp.update();

        if (!vp.handDetected) {
            putText(vp.outFrame, "No hand detected", Point(20, 60), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 255), 2);
        } else if (vp.isStop()) {
            putText(vp.outFrame, "STOP", Point(20, 60), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 255), 2);
        } else if (vp.isAdvance()) {
            putText(vp.outFrame, "ADVANCE (W)", Point(20, 60), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 255), 2);
        } else if (vp.isLeft()) {
            putText(vp.outFrame, "ADVANCE LEFT (A)", Point(20, 60), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 255), 2);
        } else if (vp.isRight()) {
            putText(vp.outFrame, "ADVANCE RIGHT (D)", Point(20, 60), FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 255), 2);
        }

        // Mostrar interfaz completa
        resize(vp.outFrame, frameMano, Size(widthLado, heightCam));
        resize(framePatron, framePatron, Size(widthLado, heightCam));

        canvas.setTo(Scalar(180, 255, 255));
        rectangle(canvas, Rect(0, 0, widthJuego, heightTotal), Scalar(255, 200, 100), FILLED);
        putText(canvas, "UI DEL JUEGO - AQUI VA EL MODELO 3D", Point(20, heightTotal / 2),
                FONT_HERSHEY_SIMPLEX, 0.8, Scalar(0, 0, 0), 2);

        frameMano.copyTo(canvas(Rect(widthJuego, 0, widthLado, heightCam)));
        framePatron.copyTo(canvas(Rect(widthJuego, heightCam, widthLado, heightCam)));

        imshow("Interfaz Principal", canvas);
        imshow("HSV", vp.hsv);

        char key = (char)waitKey(30);
        if (key == 27) break;
    }

    camMano.release();
    camPatron.release();
    destroyAllWindows();
    return 0;
}
