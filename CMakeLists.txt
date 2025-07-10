cmake_minimum_required(VERSION 3.10)
project(PistaCarrerasRA)

set(CMAKE_CXX_STANDARD 17)

# Configuración específica para macOS
if(APPLE)
    # Buscar paquetes necesarios
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
    
    # Configuración especial para Assimp en Homebrew
    set(ASSIMP_ROOT_DIR "/opt/homebrew/Cellar/assimp/6.0.2")
    find_package(assimp CONFIG REQUIRED
        PATHS ${ASSIMP_ROOT_DIR}/lib/cmake/assimp-6.0
        NO_DEFAULT_PATH
    )
    
    # Configurar frameworks de macOS
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    
    # Directorios de inclusión
    include_directories(
        include
        ${OPENGL_INCLUDE_DIR}
        ${GLFW3_INCLUDE_DIR}
        ${ASSIMP_INCLUDE_DIRS}
        /opt/homebrew/include
    )
    
    # Archivos fuente
    set(SOURCES
        src/main.cpp
        src/model_renderer.cpp
        src/glad.c
    )
    
    add_executable(PistaCarrerasRA ${SOURCES})
    
    # Enlazar bibliotecas
    target_link_libraries(PistaCarrerasRA
        ${OPENGL_LIBRARIES}
        glfw
        assimp::assimp  # Esta es la forma correcta para Assimp 5.0+
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
endif()