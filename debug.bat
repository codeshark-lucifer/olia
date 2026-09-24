@echo off
setlocal

cd /d "%~dp0"

:: Use the validation layer bundled with this project.
set "VULKAN_BIN=%~dp0vendor\vulkan\bin"
set "VK_LAYER_PATH=%VULKAN_BIN%"
set "PATH=%VULKAN_BIN%;%PATH%"

echo [INFO] Vulkan validation layer path: %VK_LAYER_PATH%

echo ========================================
echo   Configuring Vulkan validation build
echo ========================================
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DOLIA_ENABLE_VALIDATION=ON
if errorlevel 1 (
    echo [ERROR] CMake configuration failed.
    exit /b 1
)

echo.
echo ========================================
echo   Compiling Vulkan shaders
echo ========================================
call "%~dp0compile.bat"
if errorlevel 1 (
    echo [ERROR] Shader compilation failed.
    exit /b 1
)

echo.
echo ========================================
echo   Building application
echo ========================================
cmake --build build
if errorlevel 1 (
    echo [ERROR] Application build failed.
    exit /b 1
)

echo.
echo ========================================
echo   Running with Vulkan validation enabled
echo   Validation messages will appear below.
echo ========================================
build\application.exe

endlocal