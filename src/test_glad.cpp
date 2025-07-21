#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../include/model_renderer.h"
#include "../include/marker_detection.h"
#include <chrono>
#include <iostream>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char* quadVS = R"(#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
out vec2 uv;
void main(){
    uv = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
})";

const char* quadFS = R"(#version 330 core
in vec2 uv;
out vec4 Frag;
uniform sampler2D tex;
void main(){
    Frag = texture(tex, uv);
})";

GLuint quadVAO = 0, quadVBO = 0, quadShader = 0, quadTex = 0;


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

GLuint compile(GLenum type, const char* src){
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(sh, 512, nullptr, log);
        std::cerr << "Shader compile error:\n" << log << std::endl;
    }
    return sh;
}

void initQuad(){
    float verts[] = {
        //  (x, y)     (u, v)
        -1,  1,       0, 1,
        -1, -1,       0, 0,
         1, -1,       1, 0,
        -1,  1,       0, 1,
         1, -1,       1, 0,
         1,  1,       1, 1
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint vs = compile(GL_VERTEX_SHADER, quadVS);
    GLuint fs = compile(GL_FRAGMENT_SHADER, quadFS);
    quadShader = glCreateProgram();
    glAttachShader(quadShader, vs);
    glAttachShader(quadShader, fs);
    glLinkProgram(quadShader);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // Inicializar textura de video
    glGenTextures(1, &quadTex);
    glBindTexture(GL_TEXTURE_2D, quadTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Error al inicializar GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
                                          "AR con fondo de cámara", nullptr, nullptr);
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

    ModelRenderer renderer("models/perfumes.obj");

    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                            (float)SCR_WIDTH / SCR_HEIGHT,
                                            0.1f, 100.0f);

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "No se pudo abrir la cámara\n";
        return -1;
    }

    cv::Mat K, dist;
    if (!cv::FileStorage("src/calibracion.yml", cv::FileStorage::READ).isOpened()) {
        K = (cv::Mat_<double>(3,3) << 800,0,SCR_WIDTH/2,
                                      0,800,SCR_HEIGHT/2,
                                      0,0,1);
        dist = cv::Mat::zeros(5,1,CV_64F);
    } else {
        cv::FileStorage fs("src/calibracion.yml", cv::FileStorage::READ);
        fs["cameraMatrix"] >> K;
        fs["distCoeffs"]  >> dist;
        fs.release();
    }

    PoseData pose, lastPose;
    bool   hasLastPose = false;
    auto   lastDetectionTime = std::chrono::steady_clock::now();

    cv::Mat frame;

    while (!glfwWindowShouldClose(window)) {
        cap >> frame;
        if (frame.empty()) break;

        procesarFrame(frame, K, dist, pose);
        if (pose.poseValida) {
            lastPose         = pose;
            hasLastPose      = true;
            lastDetectionTime = std::chrono::steady_clock::now();
        }

        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
        cv::flip(frame, frame, 0);
        glBindTexture(GL_TEXTURE_2D, quadTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                     frame.cols, frame.rows, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, frame.data);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        glUseProgram(quadShader);
        glBindVertexArray(quadVAO);
        glBindTexture(GL_TEXTURE_2D, quadTex);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glEnable(GL_DEPTH_TEST);

        double elapsed =
            std::chrono::duration<double>(
                std::chrono::steady_clock::now() - lastDetectionTime).count();

        if (hasLastPose && elapsed < 2.0) {
            glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
            glm::mat4 view  = cvPoseToView(lastPose.rvec, lastPose.tvec);
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
