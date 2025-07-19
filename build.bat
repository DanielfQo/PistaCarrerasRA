@echo off
setlocal

set OpenCV_DIR=C:/opencv/build

if not exist build (
    mkdir build
)

cd build

if not exist CMakeCache.txt (
    cmake .. -DOpenCV_DIR=%OpenCV_DIR%
)

cmake --build . --config Release

REM Ejecutar desde carpeta build\Release
.\Release\PistaCarrerasRA.exe

endlocal
pause