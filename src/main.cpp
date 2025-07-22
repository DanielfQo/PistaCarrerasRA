#include <opencv2/opencv.hpp>
#include <atomic>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "../include/model_renderer.h"
#include "../include/marker_detection.h"
#include "../include/vision/gesture_recognition.h"
#include "../include/game_controller.h"
#include "../include/quad_renderer.h"

extern GLuint quadVAO, quadTex, quadShader;
extern void initQuad();

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int main() {
    cv::Mat K, dist;
    if (!GameController::inicializarCalibracion(K, dist, 0)) {
        std::cerr << "No se pudo continuar sin calibración.\n";
        return -1;
    }

    if (!glfwInit()) {
        std::cerr << "Error al inicializar GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "AR con fondo de cámara", nullptr, nullptr);
    if (!window) {
        std::cerr << "No se pudo crear la ventana GLFW\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Error al inicializar GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    initQuad();

    ModelRenderer renderer("../models/carro2/Carro.obj");
    ModelRenderer pistaRenderer("../models/pista/10605_Slot_Car_Race_Track_v1_L3.obj");

    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                            (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

    cv::VideoCapture capMarker(0);
    cv::VideoCapture capHand(1);
    if (!capMarker.isOpened() || !capHand.isOpened()) {
        std::cerr << "No se pudieron abrir las cámaras\n";
        return -1;
    }

    VisionProcessor vision;
    GameController game(renderer, vision, K, dist);

    cv::Mat frameMarker, frameHand;

    float angle = 0.0f;

    // Bucle principal
    while (!glfwWindowShouldClose(window)) {
        capMarker >> frameMarker;
        capHand   >> frameHand;
        if (frameMarker.empty() || frameHand.empty()) break;

        game.process(frameMarker, frameHand);

        cv::putText(frameMarker, game.getStatusText(), cv::Point(20, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            game.resetPosition();
        }

        cv::cvtColor(frameMarker, frameMarker, cv::COLOR_BGR2RGB);
        cv::flip(frameMarker, frameMarker, 0);
        glBindTexture(GL_TEXTURE_2D, quadTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
            frameMarker.cols, frameMarker.rows, 0,
            GL_RGB, GL_UNSIGNED_BYTE, frameMarker.data);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        glUseProgram(quadShader);
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, quadTex);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_DEPTH_TEST);

        game.drawStaticPista(projection, pistaRenderer);
        game.drawModel(projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    capMarker.release();
    capHand.release();
    glfwTerminate();
    return 0;
}



/*#include <opencv2/opencv.hpp>
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
*/