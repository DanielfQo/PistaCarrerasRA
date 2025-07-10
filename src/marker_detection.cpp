#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include "marker_detection.h"

void arUco(){ // para comparar

    cv::VideoCapture cap(0); 
    if (!cap.isOpened()) {
        std::cerr << "No se pudo abrir la cámara." << std::endl;
        return;
    }

    //cv::Ptr<cv::aruco::Dictionary> diccionario = 
        //cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

    cv::Ptr<cv::aruco::Dictionary> diccionario = cv::makePtr<cv::aruco::Dictionary>(
    cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50));

    while (true) {
        cv::Mat frame;
        cap >> frame; 
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> esquinas;

        cv::aruco::detectMarkers(frame, diccionario, esquinas, ids);

        if (!ids.empty()) {
            cv::aruco::drawDetectedMarkers(frame, esquinas, ids);
        }

        cv::imshow("Marcadores ArUco", frame);

        if (cv::waitKey(10) == 'q') break;
    }

    return;

}

const int warpSize = 200;
const int totalGrid = 6;
const int internalGrid = 4;

std::vector<std::vector<int>> matriz0 = {
    {0, 1, 0, 0},
    {1, 0, 1, 0},
    {1, 1, 0, 0},
    {1, 1, 0, 1}
};

std::vector<std::vector<int>> rotar90(const std::vector<std::vector<int>>& mat) {
    int n = mat.size();
    std::vector<std::vector<int>> rot(n, std::vector<int>(n));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            rot[j][n - i - 1] = mat[i][j];
    return rot;
}

int detectarOrientacion(const std::vector<std::vector<int>>& bits, const std::vector<std::vector<int>>& ref) {
    std::vector<std::vector<int>> rot = ref;
    for (int i = 0; i < 4; ++i) {
        if (bits == rot) return i * 90;
        rot = rotar90(rot);
    }
    return -1;
}

void rotarOrderedPts(std::vector<cv::Point2f>& pts, int angulo) {
    int rotaciones = (angulo / 90) % 4;
    for (int i = 0; i < rotaciones; ++i) {
        std::rotate(pts.begin(), pts.begin() + 1, pts.end());
    }
}

bool validarBordeNegro(const cv::Mat& warpBin) {
    int cellSize = warpSize / totalGrid;
    for (int i = 0; i < totalGrid; i++) {
        for (int j = 0; j < totalGrid; j++) {
            if (i == 0 || i == totalGrid - 1 || j == 0 || j == totalGrid - 1) {
                int x = j * cellSize;
                int y = i * cellSize;
                cv::Rect cell(x, y, cellSize, cellSize);
                cv::Mat subImg = warpBin(cell);
                int blancos = cv::countNonZero(subImg);
                float ratio = (float)blancos / (cellSize * cellSize);
                if (ratio > 0.2) return false;
            }
        }
    }
    return true;
}

void ordenarPuntos(const std::vector<cv::Point2f>& srcPts, std::vector<cv::Point2f>& orderedPts) {
    std::vector<std::pair<float, int>> sumPts;
    for (int i = 0; i < 4; i++)
        sumPts.push_back({srcPts[i].x + srcPts[i].y, i});
    std::sort(sumPts.begin(), sumPts.end(), [](auto& a, auto& b) { return a.first < b.first; });
    orderedPts[0] = srcPts[sumPts[0].second];
    orderedPts[2] = srcPts[sumPts[3].second];

    std::vector<std::pair<float, int>> diffPts;
    for (int i = 0; i < 4; i++) {
        if (i != sumPts[0].second && i != sumPts[3].second)
            diffPts.push_back({srcPts[i].y - srcPts[i].x, i});
    }
    if (diffPts[0].first < diffPts[1].first) {
        orderedPts[1] = srcPts[diffPts[0].second];
        orderedPts[3] = srcPts[diffPts[1].second];
    } else {
        orderedPts[1] = srcPts[diffPts[1].second];
        orderedPts[3] = srcPts[diffPts[0].second];
    }
}

std::vector<std::vector<int>> extraerBits(const cv::Mat& warpBin) {
    int cellSize = warpSize / totalGrid;
    std::vector<std::vector<int>> bits(internalGrid, std::vector<int>(internalGrid));

    for (int i = 0; i < internalGrid; i++) {
        for (int j = 0; j < internalGrid; j++) {
            int y = (i + 1) * cellSize;
            int x = (j + 1) * cellSize;
            cv::Rect cell(x, y, cellSize, cellSize);
            cv::Mat subImg = warpBin(cell);
            int blancos = cv::countNonZero(subImg);
            float ratio = (float)blancos / (cellSize * cellSize);
            bits[i][j] = (ratio < 0.5) ? 1 : 0;
        }
    }
    return bits;
}

void procesarFrame(cv::Mat& frame, const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs) {
    cv::Mat gray, blurred, bin;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.5);
    cv::threshold(blurred, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(bin, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours) {
        std::vector<cv::Point> approx;
        double epsilon = 0.05 * cv::arcLength(contour, true);
        cv::approxPolyDP(contour, approx, epsilon, true);

        if (approx.size() == 4 && cv::isContourConvex(approx) && cv::contourArea(approx) > 1000) {
            std::vector<cv::Point2f> srcPts;
            for (const auto& p : approx) srcPts.push_back(p);
            std::vector<cv::Point2f> orderedPts(4);
            ordenarPuntos(srcPts, orderedPts);

            std::vector<cv::Point2f> dstPts = {
                {0, 0}, {warpSize - 1, 0}, {warpSize - 1, warpSize - 1}, {0, warpSize - 1}
            };

            cv::Mat H = cv::findHomography(orderedPts, dstPts);
            cv::Mat warped;
            cv::warpPerspective(frame, warped, H, cv::Size(warpSize, warpSize));

            cv::Mat warpGray, warpBin;
            cv::cvtColor(warped, warpGray, cv::COLOR_BGR2GRAY);
            cv::threshold(warpGray, warpBin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

            if (!validarBordeNegro(warpBin)) continue;

            std::vector<std::vector<int>> bits = extraerBits(warpBin);
            int angulo = detectarOrientacion(bits, matriz0);
            if (angulo == -1) continue;
            rotarOrderedPts(orderedPts, angulo);

            std::vector<cv::Point3f> objPoints = {
                {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}
            };

            cv::Mat rvec, tvec;
            cv::solvePnP(objPoints, orderedPts, cameraMatrix, distCoeffs, rvec, tvec);

            std::cout << "Pose del marcador:\n" << "Rotacion: " << rvec.t() << "\n" << "Traslacion: " << tvec.t() << "\n";

            
            std::vector<cv::Point3f> axis = {
                {0, 0, 0}, {0.5f, 0, 0}, {0, 0.5f, 0}, {0, 0, 0.5f}
            };
            std::vector<cv::Point2f> imgpts;
            cv::projectPoints(axis, rvec, tvec, cameraMatrix, distCoeffs, imgpts);

            cv::line(frame, imgpts[0], imgpts[1], cv::Scalar(0,0,255), 2); // x rojo
            cv::line(frame, imgpts[0], imgpts[2], cv::Scalar(0,255,0), 2); // y verde
            cv::line(frame, imgpts[0], imgpts[3], cv::Scalar(255,0,0), 2); // z azul
        }
    }

    cv::imshow("Posibles marcadores", frame);
    cv::imshow("Imagen binarizada (Otsu)", bin);
}

void procesarMarcadores(cv::Mat& frame, const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs) {
    procesarFrame(frame, cameraMatrix, distCoeffs);
}
/*
int main() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "No se pudo abrir la cámara." << std::endl;
        return -1;
    }

    // falta calibrar
    cv::Mat cameraMatrix = (cv::Mat_<double>(3, 3) << 800, 0, 320, 0, 800, 240, 0, 0, 1);
    cv::Mat distCoeffs = cv::Mat::zeros(5, 1, CV_64F);

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        procesarFrame(frame, cameraMatrix, distCoeffs);
        if (cv::waitKey(10) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}*/