#include "../include/model_renderer.h"
#include "../include/marker_detection.h"
#include "../include/vision/gesture_recognition.h"
#include "../include/game_controller.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using Clock = std::chrono::steady_clock;

namespace {
static glm::mat4 cvPoseToView(const cv::Mat& rvec, const cv::Mat& tvec) {
    cv::Mat R;  cv::Rodrigues(rvec, R);
    cv::Mat viewCV = cv::Mat::eye(4, 4, CV_64F);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) viewCV.at<double>(i, j) = R.at<double>(i, j);
        viewCV.at<double>(i, 3) = tvec.at<double>(i);
    }
    cv::Mat cvToGl = (cv::Mat_<double>(4,4) << 1,0,0,0,  0,-1,0,0,  0,0,-1,0,  0,0,0,1);
    viewCV = cvToGl * viewCV;

    glm::mat4 view(1.0f);
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            view[c][r] = static_cast<float>(viewCV.at<double>(r, c));
    return view;
}
}
GameController::GameController(ModelRenderer& rend, VisionProcessor& vis,
                               const cv::Mat& K_, const cv::Mat& dist_)
    : renderer(rend), vision(vis), position(0.0f), accion("Sin gesto"),
      hasLastPose(false), lastDetectionTime(Clock::now()),
      K(K_.clone()), dist(dist_.clone()) {}


void GameController::process(cv::Mat& frameMarker, cv::Mat& frameHand) {
    procesarFrame(frameMarker, K, dist, pose);
    if (pose.poseValida) {
        lastPose = pose;
        hasLastPose = true;
        lastDetectionTime = Clock::now();
    }

    vision.processHand(frameHand);
    vision.update();

    double elapsed = std::chrono::duration<double>(Clock::now() - lastDetectionTime).count();
    if (!(hasLastPose && elapsed < 2.0)) {
        accion = "Sin gesto";
        return;
    }

    const float step = 0.15f;
    if (vision.isAdvance()) {
        position.z += step;  accion = "Arriba";
    } else if (vision.isLeft()) {
        position.x -= step;  accion = "Izquierda";
    } else if (vision.isRight()) {
        position.x += step;  accion = "Derecha";
    } else if (vision.isStop()) {
        position.z -= step;  accion = "Abajo";
    } else {
        accion = "Sin gesto";
    }
}

void GameController::drawModel(const glm::mat4& projection) {
    if (!hasLastPose) return;
    double elapsed = std::chrono::duration<double>(Clock::now() - lastDetectionTime).count();
    if (elapsed >= 2.0) return;

    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));
    model = glm::rotate(model, glm::radians(90.0f),  glm::vec3(1,0,0));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0,0,1));

    model = glm::translate(model, position);
    
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.05f));

    glm::mat4 view = cvPoseToView(lastPose.rvec, lastPose.tvec);

    renderer.SetViewProjection(view, projection);
    renderer.SetModelMatrix(model);
    renderer.Draw();
}

std::string GameController::getStatusText() const {
    char buf[128];
    snprintf(buf, sizeof(buf), "Gesto: %s | Pos x=%.2f y=%.2f z=%.2f",
             accion.c_str(), position.x, position.y, position.z);
    return std::string(buf);
}

glm::vec3 GameController::getPosition() const { return position; }

void GameController::resetPosition() {
    position   = glm::vec3(0.0f);
    accion     = "Reset";
}
