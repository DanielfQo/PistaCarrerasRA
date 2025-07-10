#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class VisionProcessor {
public:
    Mat outFrame;
    Mat hsv;
    bool handDetected = false;
    std::string estadoMano = "Desconocido";

    vector<Point> contour;
    vector<Point> hull;
    Vec4f fittingLine;

    void processHand(const Mat& inFrame);
    // Procesa el frame y dibuja convex hull + l�nea de ajuste

    // DETECCION DE MANO ABIERTA O CERRADA  

    double calculateSolidity() const;
    // Calcular si esta abierto o cerrado

    double calculateAngle() const;
    // Calcular la direccion del carro

    double calculateAspect() const;
    // Calculo de pu�o (forma cuadrada) o palma (forma rectangular)

    double calculateDefects() const;
    // Calculo de dedos

    void classifyHand();
};