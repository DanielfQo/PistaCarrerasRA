#include <opencv2/opencv.hpp>
#include <atomic>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vision/gesture_recognition.h"
#include "model_renderer.h"

using namespace cv;

// Variable compartida entre hilos
std::atomic<bool> handOpen(false);

// Función que corre en un hilo para procesar la cámara
void visionThread() {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        printf("Error abriendo la camara\n");
        return;
    }

    Mat frame;
    VisionProcessor vp;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        vp.processHand(frame);
        handOpen = vp.handOpen;  // actualizar variable global

        imshow("Camara", frame);
        imshow("Vision Mano", vp.outFrame);
        imshow("HSV", vp.hsv);

        char key = (char)waitKey(30);
        if (key == 27) break; // ESC
    }

    cap.release();
    destroyAllWindows();
}

int main() {
    // Lanzar hilo para procesamiento de visión
    std::thread camThread(visionThread);

    // Inicializar GLFW y ventana
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(800, 600, "Modelo con Gesto", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glEnable(GL_DEPTH_TEST);
    // Opcional: glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // modo alámbrico

    ModelRenderer model("models/perfumes.obj");

    float angle = 0.0f;

    // Bucle principal
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Cámara y proyección
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f),
                                     glm::vec3(0.0f, 0.0f, 0.0f),
                                     glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                                800.0f / 600.0f,
                                                0.1f, 100.0f);

        model.SetViewProjection(view, projection);

        // Rotar solo si la mano está abierta
        if (handOpen) angle += 0.01f;

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0.0f, 1.0f, 0.0f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.01f));
        model.SetModelMatrix(modelMatrix);

        model.Draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    camThread.join();
    glfwTerminate();
    return 0;
}
