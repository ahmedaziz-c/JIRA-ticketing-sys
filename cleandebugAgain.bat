@echo off
echo Setting up Retro-SCRUM TUI for Windows 11...

:: Clean previous builds COMPLETELY
rmdir /s /q build 2>nul
rmdir /s /q out 2>nul
rmdir /s /q _deps 2>nul
rmdir /s /q .vs 2>nul
rmdir /s /q projects 2>nul

:: Create external directory structure for json
mkdir external\nlohmann\json 2>nul

:: Download nlohmann/json single header
echo Downloading nlohmann/json header...
powershell -Command "Invoke-WebRequest -Uri 'https://github.com/nlohmann/json/releases/download/v3.11.2/json.hpp' -OutFile 'external\nlohmann\json\json.hpp'"

:: Create build directory
mkdir build
cd build

echo Configuring project with CMake...
cmake -G "Visual Studio 17 2022" -A x64 ..

if %errorlevel% equ 0 (
    echo Building project...
    cmake --build . --config Debug
    
    if %errorlevel% equ 0 (
        echo.
        echo ========================================
        echo BUILD SUCCESSFUL! 
        echo ========================================
        echo.
        echo Running Retro-SCRUM TUI...
        echo.
        Debug\retro-scrum.exe
    ) else (
        echo.
        echo BUILD FAILED!
    )
) else (
    echo.
    echo CMAKE CONFIGURATION FAILED!
)

pause