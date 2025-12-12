@echo off
REM === Build and run the fluid simulation ===

REM Go to the build directory (create it if missing)
if not exist build (
    mkdir build
)
cd build

REM Run CMake configuration
cmake ..

REM Build the project (Release mode by default)
cmake --build . --config Release

REM Go back to root
cd ..

REM Run the executable
if exist terrain.exe (
    terrain.exe
) else (
    echo Executable not found!
)
