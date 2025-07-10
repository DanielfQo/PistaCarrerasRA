#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>

int g_cameraIndex = 0;

void arUco(){ // para comparar

    cv::VideoCapture cap(g_cameraIndex); 
    if (!cap.isOpened()) {
        std::cerr << "No se pudo abrir la cámara." << std::endl;
        return;
    }

    cv::Ptr<cv::aruco::Dictionary> diccionario = 
        cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

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
                {0, 0, 0}, {0.5f, 0, 0}, {0, 0.5f, 0}, {0, 0, -0.5f}  
            };
            std::vector<cv::Point2f> imgpts;
            cv::projectPoints(axis, rvec, tvec, cameraMatrix, distCoeffs, imgpts);

            cv::line(frame, imgpts[0], imgpts[1], cv::Scalar(0,0,255), 2); // x rojo
            cv::line(frame, imgpts[0], imgpts[2], cv::Scalar(0,255,0), 2); // y verde
            cv::line(frame, imgpts[0], imgpts[3], cv::Scalar(255,0,0), 2); // z azul
        }
    }

    //cv::imshow("Posibles marcadores", frame);
}

void ejecutarDeteccion(const cv::Mat& cameraMatrix = cv::Mat(), 
                      const cv::Mat& distCoeffs = cv::Mat()) {
    cv::VideoCapture cap(g_cameraIndex);
    if (!cap.isOpened()) {
        std::cerr << "Error: No se pudo abrir la cámara" << std::endl;
        return;
    }

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
            std::cerr << "Error: Frame vacio" << std::endl;
            break;
        }

        cv::putText(frame, estadoCalibracion, cv::Point(10, 30),
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, colorCalibracion, 2);

        procesarFrame(frame, camMatrix, distCoeff);

        cv::putText(frame, "Presiona 'q' para salir", cv::Point(10, frame.rows - 10),
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 1);

        cv::imshow("Deteccion de Marcadores", frame);

        if (cv::waitKey(10) == 'q') break;
    }

    cap.release();
    cv::destroyAllWindows();
}

bool captureCalibrationImages(int num_images = 20, const std::string& filename_prefix = "calibrate/calib_") {
    cv::VideoCapture cap(g_cameraIndex);
    if (!cap.isOpened()) {
        std::cerr << "Error" << std::endl;
        return false;
    }

    int imgCount = 0;
    std::cout << "Instrucciones:\n"
              << "1. Muestra el tablero de ajedrez en diferentes posiciones\n"
              << "2. Presiona 's' para guardar cada imagen\n"
              << "3. Presiona 'q' para terminar antes de tiempo\n\n";

    while (imgCount < num_images) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Frame vacio" << std::endl;
            break;
        }

        cv::putText(frame, "Presiona 's' para guardar (" + std::to_string(imgCount) + "/" + 
                    std::to_string(num_images) + ")", cv::Point(10, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 0), 2);
        
        cv::imshow("Captura para calibracion", frame);
        char key = cv::waitKey(30);

        if (key == 's') {
            std::string filename = filename_prefix + std::to_string(imgCount) + ".jpg";
            if (cv::imwrite(filename, frame)) {
                std::cout << "Imagen guardada: " << filename << std::endl;
                imgCount++;
            } else {
                std::cerr << "Error al guardar: " << filename << std::endl;
            }
        } else if (key == 'q') {
            std::cout << "Captura interrumpida" << std::endl;
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    
    if (imgCount < 5) {
        std::cerr << "Advertencia: Se recomienda capturar al menos 15 imagenes" << std::endl;
        return false;
    }

    std::cout << "Captura completada. " << imgCount << " imagenes guardadas." << std::endl;
    return true;
}

double calibrateCameraFromImages(
    const std::string& images_path,
    const cv::Size& boardSize,
    float squareSize,
    cv::Mat& cameraMatrix,
    cv::Mat& distCoeffs,
    bool showCorners = true)
{
    std::vector<std::vector<cv::Point3f>> objectPoints;
    std::vector<cv::Point3f> objCorners;
    
    for (int y = 0; y < boardSize.height; y++) {
        for (int x = 0; x < boardSize.width; x++) {
            objCorners.push_back(cv::Point3f(x * squareSize, y * squareSize, 0));
        }
    }

    std::vector<cv::String> images;
    cv::glob(images_path, images);
    
    if (images.empty()) {
        std::cerr << "Error: No se encontraron imgs" 
                  << images_path << std::endl;
        return -1;
    }

    std::vector<std::vector<cv::Point2f>> imagePoints;
    cv::Size imageSize;
    
    for (const auto& filename : images) {
        cv::Mat img = cv::imread(filename);
        if (img.empty()) {
            std::cerr << "Advertencia: No se pudo leer " << filename << std::endl;
            continue;
        }
        
        imageSize = img.size();
        cv::Mat gray;
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

        std::vector<cv::Point2f> corners;
        bool found = cv::findChessboardCorners(gray, boardSize, corners,
            cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE);

        if (found) {
            cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
                cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.1));

            imagePoints.push_back(corners);
            objectPoints.push_back(objCorners);
            if (showCorners) {
                cv::drawChessboardCorners(img, boardSize, corners, found);
                //cv::imshow("Esquinas detectadas", img);
                //cv::waitKey(300);
            }
        } else {
            std::cerr << "Advertencia: No se encontraron esquinas en " << filename << std::endl;
        }
    }

    if (imagePoints.size() < 5) {
        std::cerr << "Error: Insuficientes imágenes válidas (" << imagePoints.size() 
                  << "). Se requieren al menos 5." << std::endl;
        return -1;
    }

    std::vector<cv::Mat> rvecs, tvecs;
    double rms = cv::calibrateCamera(
        objectPoints, imagePoints, imageSize,
        cameraMatrix, distCoeffs, rvecs, tvecs,
        cv::CALIB_FIX_PRINCIPAL_POINT | cv::CALIB_FIX_ASPECT_RATIO
    );

    return rms;
}

void mostrarMenu() {
    std::cout << "\n=== MENU PRINCIPAL ===\n";
    std::cout << "1. Calibrar camara (capturar imagenes y calcular parametros)\n";
    std::cout << "2. Ejecutar deteccion de marcadores\n";
    std::cout << "3. Salir\n";
    std::cout << "Seleccione una opcion: ";
}

int main() {
    /*
    for (int i = 0; i < 5; ++i) {
        cv::VideoCapture cap(i);
        if (cap.isOpened()) {
            std::cout << "Cámara encontrada en índice: " << i << std::endl;
            cv::Mat frame;
            cap >> frame;
            if (!frame.empty()) {
                cv::imshow("DroidCam", frame);
                cv::waitKey(0);
                
            }
        }
    }*/

    cv::Mat cameraMatrix, distCoeffs;
    bool calibrado = false;
    double errorCalibracion = -1.0;

    while (true) {
        mostrarMenu();
        int opcion;
        std::cin >> opcion;

        switch (opcion) {
            case 1: {
                if (!captureCalibrationImages()) {
                    std::cout << "Fallo en la captura de imagenes.\n";
                    break;
                }

                errorCalibracion = calibrateCameraFromImages(
                    "calibrate/calib_*.jpg", cv::Size(6, 9), 2.5f, 
                    cameraMatrix, distCoeffs
                );

                if (errorCalibracion >= 0) {
                    calibrado = true;
                    std::cout << "\nCalibracion exitosa! Error RMS: " 
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
                    ejecutarDeteccion(cameraMatrix, distCoeffs);
                } else {
                    std::cout << "Advertencia: La camara no esta calibrada. "
                              << "Se ejecutara con parametros por defecto.\n";
                    ejecutarDeteccion();
                }
                break;
            }

            case 3:
                std::cout << "Saliendo del programa...\n";
                return 0;

            default:
                std::cout << "Opcion no valida. Intente nuevamente.\n";
        }
    }

    return 0;
}
