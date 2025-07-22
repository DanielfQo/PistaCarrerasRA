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

    bool drawTrack = true; 
    std::cout << "\n¿Deseas dibujar la pista de carreras? (1 = sí, 0 = no): ";
    int respuesta = 1;
    std::cin >> respuesta;
    if (respuesta == 1) {
        drawTrack = true;
    } else {
        drawTrack = false;
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

        if (drawTrack) {
            game.drawStaticPista(projection, pistaRenderer);
        }
        game.drawModel(projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    capMarker.release();
    capHand.release();
    glfwTerminate();
    return 0;
}
