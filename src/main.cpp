#include <opencv2/opencv.hpp>
#include "vision/gesture_recognition.h"
#include "marker_detection.h"

#include <GLUT/glut.h>
#include <thread>
#include <atomic>
#include <iostream>

std::atomic<bool> markerVisible(false);
std::atomic<bool> handOpen(false);
std::atomic<bool> handDetected(false);
float zPos = -5.0f;
const float sphereRadius = 0.3f;\

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, zPos);

    if (markerVisible.load()) {
        glColor3f(0.3f, 0.7f, 1.0f);
        glutSolidSphere(sphereRadius, 30, 30);
    }

    glutSwapBuffers();
}

void idle() {
    if (markerVisible.load()) {
        if (handDetected.load()) {
            zPos += handOpen.load() ? 0.05f : -0.05f;
        }
        // Limitar posición Z para que no se salga de la vista
        zPos = std::max(-10.0f, std::min(0.0f, zPos));
    }
    glutPostRedisplay();
}

void initGL() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0, 1.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void startGL(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Esfera RA");
    initGL();
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutMainLoop();
}

void visionLoop() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "No se pudo abrir la cámara.\n";
        return;
    }

    VisionProcessor vp;
    cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) << 800, 0, 320,
                                                      0, 800, 240,
                                                      0,   0,   1);
    cv::Mat distCoeffs = cv::Mat::zeros(5, 1, CV_64F);

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        // Procesar mano
        vp.processHand(frame);
        handDetected = vp.handDetected;
        handOpen = (vp.estadoMano == "Abierta");

        // Procesar marcadores
        MarkerData marker = procesarMarcadores(frame, cameraMatrix, distCoeffs);
        markerVisible = marker.detectado;

        // Mostrar información de depuración
        std::cout << "\r"
                  << "Mano: " << (handDetected ? "si " : "no ")
                  << (handDetected ? (handOpen ? "Abierta" : "Cerrada") : "-")
                  << " | Marcador: " << (markerVisible ? "si" : "no")
                  << " | ZPos: " << zPos
                  << std::flush;

        if ((char)cv::waitKey(30) == 27) break;
    }

    cap.release();
    std::exit(0);
}

int main(int argc, char** argv) {
    std::thread visionThread(visionLoop);
    startGL(argc, argv);
    visionThread.join();
    return 0;
}