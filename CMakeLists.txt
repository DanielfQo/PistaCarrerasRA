cmake_minimum_required(VERSION 3.10)

project(PistaCarrerasRA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(APPLE)
    set(OpenCV_DIR "/opt/homebrew/opt/opencv/share/opencv4")
    find_package(OpenCV REQUIRED)
endif()

set(SOURCES
    src/main.cpp
    src/vision/gesture_recognition.cpp
    src/model_renderer.cpp
    src/utils.cpp
)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/vision
)

add_executable(PistaCarrerasRA ${SOURCES})

if(APPLE)
    target_link_libraries(PistaCarrerasRA
        ${OpenCV_LIBS}
        "-framework OpenGL"
        "-framework GLUT")
else()
    target_link_libraries(PistaCarrerasRA ${OpenCV_LIBS})
endif()