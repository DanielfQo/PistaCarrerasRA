cmake_minimum_required(VERSION 3.10)
project(PistaCarrerasRA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(OpenCV_DIR "/opt/homebrew/opt/opencv/share/opencv4")
    set(GLFW_INCLUDE_DIR "/opt/homebrew/include")
    set(GLFW_LIBRARY "/opt/homebrew/lib/libglfw.3.dylib")
endif()

find_package(OpenCV REQUIRED)

file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.c
)

find_package(assimp REQUIRED)

include_directories(${ASSIMP_INCLUDE_DIRS})

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
        ${ASSIMP_LIBRARIES}
        ${GLFW_LIBRARY}
        "-framework OpenGL"
        "-framework GLUT"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
else()
    target_link_libraries(PistaCarrerasRA
        ${OpenCV_LIBS}
        ${ASSIMP_LIBRARIES}
        glfw3
        opengl32
        ${CMAKE_DL_LIBS}
    )
endif()