#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <opencv2/opencv.hpp>
#include <glm/glm.hpp>
#include <chrono>
#include <string>
#include "model_renderer.h"
#include "marker_detection.h"
#include "vision/gesture_recognition.h"


class GameController {
public:
    GameController(ModelRenderer& renderer, VisionProcessor& vision,
                   const cv::Mat& K, const cv::Mat& dist);

    void process(cv::Mat& frameMarker, cv::Mat& frameHand);
    void drawModel(const glm::mat4& projection);
    std::string getStatusText() const;
    glm::vec3 getPosition() const;
    void resetPosition();
    void drawStaticPista(const glm::mat4& projection, ModelRenderer& pistaRenderer);

private:
    ModelRenderer& renderer;
    VisionProcessor& vision;

    glm::vec3 position;
    std::string accion;

    PoseData pose, lastPose;
    bool hasLastPose;
    std::chrono::steady_clock::time_point lastDetectionTime;

    cv::Mat K, dist;
};

#endif
