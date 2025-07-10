cmake_minimum_required(VERSION 3.10)

project(PistaCarrerasRA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(APPLE)
    set(OpenCV_DIR "/opt/homebrew/opt/opencv/share/opencv4")
    find_package(OpenCV REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(GLUT REQUIRED)
endif()

set(SOURCES
    src/main.cpp
    src/vision/gesture_recognition.cpp
    src/marker_detection.cpp
    src/model_renderer.cpp
    src/utils.cpp
    src/glad.c
)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${OpenGL_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
)

add_executable(PistaCarrerasRA ${SOURCES})

target_link_libraries(PistaCarrerasRA 
    ${OpenCV_LIBS}
    ${OpenGL_LIBRARIES}
    ${GLUT_LIBRARY}
)