#include <opencv2/opencv.hpp>

class VisionProcessor {
public:
    // Procesa el frame y dibuja convex hull + l�nea de ajuste
    static void processHand(const cv::Mat& inputFrame, cv::Mat& outputFrame, cv::Mat& outputHsv);
};