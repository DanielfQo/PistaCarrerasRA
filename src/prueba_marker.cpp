/*
#include <iostream>
#include "marker_detection.h"

void ejecucionCamara(int cameraIndex, const cv::Mat& cameraMatrix, const cv::Mat& distCoeffs) {
    cv::VideoCapture cap(cameraIndex);
    if (!cap.isOpened()) {
        std::cerr << "Error camara" << std::endl;
        return;
    }

    PoseData poseData; // importante para opengl

    cv::Mat camMatrix = cameraMatrix;
    cv::Mat distCoeff = distCoeffs;
    bool calibracionCargada = false;

    if (camMatrix.empty() || distCoeff.empty()) {
        cv::FileStorage fs("calibracion.yml", cv::FileStorage::READ);
        if (fs.isOpened()) {
            fs["cameraMatrix"] >> camMatrix;
            fs["distCoeffs"] >> distCoeff;
            fs.release();
            calibracionCargada = true;
            std::cout << "Parametros de calibracion cargados correctamente" << std::endl;
        } else {
            camMatrix = (cv::Mat_<double>(3, 3) << 800, 0, 320, 0, 800, 240, 0, 0, 1);
            distCoeff = cv::Mat::zeros(5, 1, CV_64F);
            std::cout << "Usando parametros de camara por defecto" << std::endl;
        }
    } else {
        calibracionCargada = true;
    }

    std::string estadoCalibracion = calibracionCargada ? "CALIBRACION ACTIVA" : "SIN CALIBRACION (default)";
    cv::Scalar colorCalibracion = calibracionCargada ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error vacio" << std::endl;
            break;
        }

        cv::putText(frame, estadoCalibracion, cv::Point(10, 30),
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, colorCalibracion, 2);

        procesarFrame(frame, camMatrix, distCoeff, poseData);

        cv::putText(frame, "Presiona 'q' para salir", cv::Point(10, frame.rows - 10),
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);

        cv::imshow("Deteccion de Marcadores", frame);

        if (cv::waitKey(10) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
}


int main(){
    int cameraIndex = 0; // Cambiar

    cv::Mat cameraMatrix, distCoeffs;
    bool calibrado = false;
    double errorCalibracion = -1.0;

    while (true) {
        mostrarMenu();
        int opcion;
        std::cin >> opcion;

        switch (opcion) {
            case 1: {
                if (!captureCalibrationImages(20, "calibrate/calib_", cameraIndex)) {
                    std::cout << "Fallo en la captura de imagenes.\n";
                    break;
                }

                errorCalibracion = calibrateCameraFromImages(
                    "calibrate/calib_*.jpg", cv::Size(6, 9), 2.5f, 
                    cameraMatrix, distCoeffs
                );

                if (errorCalibracion >= 0) {
                    calibrado = true;
                    std::cout << "\nCalibracion exitosa Error RMS: " 
                              << errorCalibracion << "\n";
                    
                    cv::FileStorage fs("calibracion.yml", cv::FileStorage::WRITE);
                    fs << "cameraMatrix" << cameraMatrix;
                    fs << "distCoeffs" << distCoeffs;
                    fs.release();
                } else {
                    std::cout << "Fallo en la calibracion.\n";
                }
                break;
            }

            case 2: {
                if (!calibrado) {
                    cv::FileStorage fs("calibracion.yml", cv::FileStorage::READ);
                    if (fs.isOpened()) {
                        fs["cameraMatrix"] >> cameraMatrix;
                        fs["distCoeffs"] >> distCoeffs;
                        fs.release();
                        calibrado = true;
                        std::cout << "Parametros de calibracion cargados.\n";
                    }
                }

                if (calibrado) {
                    ejecucionCamara(cameraIndex, cameraMatrix, distCoeffs);
                } else {
                    std::cout << "La camara no esta calibrada. "
                              << "Parametros por defecto.\n";
                    ejecucionCamara(cameraIndex, cv::Mat(), cv::Mat());
                }
                break;
            }

            case 3:
                std::cout << "Saliendo\n";
                return 0;

            default:
                std::cout << "Opcion no valida\n";
        }
    }


    
}*/