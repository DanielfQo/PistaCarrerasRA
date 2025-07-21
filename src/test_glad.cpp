#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/model_renderer.h"
#include "../include/marker_detection.h"

#include <iostream>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::mat4 cvPoseToView(const cv::Mat& rvec, const cv::Mat& tvec){
    cv::Mat R;
    cv::Rodrigues(rvec, R);

    cv::Mat view_cv = cv::Mat::eye(4, 4, CV_64F);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j)
            view_cv.at<double>(i, j) = R.at<double>(i, j);
        view_cv.at<double>(i, 3) = tvec.at<double>(i);
    }

    cv::Mat cvToGl = (cv::Mat_<double>(4,4) <<
         1,  0,  0, 0,
         0, -1,  0, 0,
         0,  0, -1, 0,
         0,  0,  0, 1);
    view_cv = cvToGl * view_cv;

    glm::mat4 view(1.0f);
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            view[c][r] = static_cast<float>(view_cv.at<double>(r, c));
    return view;
}           

int main() {
\    if (!glfwInit()) {
        std::cerr << "Error al inicializar GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "AR Sin Fondo", nullptr, nullptr);
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

    ModelRenderer renderer("models/perfumes.obj");

    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                            (float)SCR_WIDTH / SCR_HEIGHT,
                                            0.1f, 100.0f);

    // Configurar cámara OpenCV
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "No se pudo abrir la cámara\n";
        return -1;
    }

    // Leer calibración si existe
    cv::Mat K, dist;
    if (!cv::FileStorage("src/calibracion.yml", cv::FileStorage::READ).isOpened()) {
        K = (cv::Mat_<double>(3, 3) << 800, 0, SCR_WIDTH / 2,
                                       0, 800, SCR_HEIGHT / 2,
                                       0, 0, 1);
        dist = cv::Mat::zeros(5, 1, CV_64F);
    } else {
        cv::FileStorage fs("src/calibracion.yml", cv::FileStorage::READ);
        fs["cameraMatrix"] >> K;
        fs["distCoeffs"] >> dist;
        fs.release();
    }

    // Loop principal
    PoseData pose;
    cv::Mat frame;

    while (!glfwWindowShouldClose(window)) {
        cap >> frame;
        if (frame.empty()) break;

        procesarFrame(frame, K, dist, pose);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (pose.poseValida) {
            glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
            glm::mat4 view = cvPoseToView(pose.rvec, pose.tvec);
            renderer.SetViewProjection(view, projection);
            renderer.SetModelMatrix(model);
            renderer.Draw();
        }


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cap.release();
    glfwTerminate();
    return 0;
}
