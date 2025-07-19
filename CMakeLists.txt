cmake_minimum_required(VERSION 3.10)

project(PistaCarrerasRA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Busca OpenCV
set(OpenCV_DIR "C:/opencv/build")
find_package(OpenCV REQUIRED)

# Archivos fuente
set(SOURCES
    src/main.cpp
    src/vision/gesture_recognition.cpp
    src/model_renderer.cpp
    src/utils.cpp
)

# Incluye headers
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/vision
)

# Crea el ejecutable
add_executable(PistaCarrerasRA ${SOURCES})

# Vincula OpenCV
target_link_libraries(PistaCarrerasRA ${OpenCV_LIBS})