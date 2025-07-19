cmake_minimum_required(VERSION 3.10)
project(PistaCarrerasRA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(OpenCV_DIR "/opt/homebrew/opt/opencv/share/opencv4")
endif()

find_package(OpenCV REQUIRED)

file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.c
)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/vision
    ${PROJECT_SOURCE_DIR}/include/glad
    ${PROJECT_SOURCE_DIR}/include/KHR
    ${PROJECT_SOURCE_DIR}/include/glm
    ${PROJECT_SOURCE_DIR}/include/assimp
)

add_executable(PistaCarrerasRA ${SOURCES})

if(APPLE)
    target_link_libraries(PistaCarrerasRA
        ${OpenCV_LIBS}
        "-framework OpenGL"
        "-framework GLUT"
    )
else()
    link_directories(${PROJECT_SOURCE_DIR}/lib)
    target_link_libraries(PistaCarrerasRA
        ${OpenCV_LIBS}
        assimp
        glfw3
        opengl32
        ${CMAKE_DL_LIBS}
    )
endif()
