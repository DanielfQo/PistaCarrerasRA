#pragma once
#include <string>
#include <opencv2/opencv.hpp>

enum class GameState {
    STOPPED,
    MOVING,
    TURNING_LEFT,
    TURNING_RIGHT
};

class GameController {
public:
    GameState state;

    GameController();

    void updateFromGesture(bool handOpen, float angle);
    std::string getStateAsString() const;

    void renderHUD(cv::Mat& frame) const;
};
