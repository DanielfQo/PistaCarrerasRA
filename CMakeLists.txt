cmake_minimum_required(VERSION 3.10)
project(PistaCarrerasRA)

set(CMAKE_CXX_STANDARD 17)

# Ruta local de OpenCV (para MinGW)
if(NOT OpenCV_DIR)
    set(OpenCV_DIR "C:/opencv/x64/mingw/lib")
endif()

# Directorios de cabecera
include_directories(
    include
    include/glad
    include/GLFW
    include/KHR
    include/glm
    include/assimp          # <-- Asegúrate de tener los headers aquí
)

# Directorios de librerías
link_directories(
    lib                     # <-- Aquí debe estar libassimp.a
)

# Archivos fuente
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.c
    main.cpp
)

# Crear el ejecutable
add_executable(PistaCarrerasRA ${SOURCES})

# Buscar e incluir OpenCV
find_package(OpenCV REQUIRED)
target_include_directories(PistaCarrerasRA PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(PistaCarrerasRA ${OpenCV_LIBS})

# Enlazar OpenGL, GLFW y Assimp
target_link_libraries(PistaCarrerasRA
    assimp                 # Enlace explícito con Assimp
    glfw3
    opengl32
    ${CMAKE_DL_LIBS}
)
