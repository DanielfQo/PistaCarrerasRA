#include "game_controller.h"

using namespace cv;
using namespace std;

GameController::GameController() {
    state = GameState::STOPPED;
}

void GameController::updateFromGesture(bool handOpen, float angle) {
    if (!handOpen) {
        state = GameState::STOPPED;
    } else {
        if (angle < -15) state = GameState::TURNING_LEFT;
        else if (angle > 15) state = GameState::TURNING_RIGHT;
        else state = GameState::MOVING;
    }
}

string GameController::getStateAsString() const {
    switch (state) {
        case GameState::STOPPED: return "STOPPED";
        case GameState::MOVING: return "MOVING";
        case GameState::TURNING_LEFT: return "TURNING LEFT";
        case GameState::TURNING_RIGHT: return "TURNING RIGHT";
        default: return "UNKNOWN";
    }
}

void GameController::renderHUD(Mat& frame) const {
    string text = "Estado: " + getStateAsString();
    putText(frame, text, Point(20, 90),
            FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255,255,255), 2);
}
