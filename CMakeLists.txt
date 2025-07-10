cmake_minimum_required(VERSION 3.10)

project(PistaCarrerasRA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Configuración pa mi MAC
if(APPLE)
    set(OpenCV_DIR "/opt/homebrew/opt/opencv/share/opencv4")
endif()

# Busca OpenCV
set(OpenCV_DIR "C:/opencv/build")
find_package(OpenCV REQUIRED)

# Archivos fuente
set(SOURCES
    src/main.cpp
    src/vision/gesture_recognition.cpp
    src/marker_detection.cpp
    src/model_renderer.cpp
    src/utils.cpp
)

# Incluye headers
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
)

# Crea el ejecutable
add_executable(PistaCarrerasRA ${SOURCES})

# Vincula OpenCV
target_link_libraries(PistaCarrerasRA ${OpenCV_LIBS})
