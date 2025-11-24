@echo off
echo ==========================================
echo      Luna - Build Script
echo ==========================================

REM Check if rc.exe is available
where rc.exe >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] rc.exe not found. 
    echo Please run this script from the "Developer Command Prompt for VS".
    pause
    exit /b 1
)

echo Creating bin folder...
if not exist bin (
    mkdir bin
)

echo [1/2] Compiling Resources...
rc.exe Luna.rc
if %errorlevel% neq 0 (
    echo [ERROR] Resource compilation failed.
    pause
    exit /b 1
)

echo [2/2] Compiling C++ Source...
cl.exe /std:c++17 /EHsc /W4 main.cpp Luna.res user32.lib shell32.lib kernel32.lib advapi32.lib ^
/link /SUBSYSTEM:WINDOWS /OUT:bin\Luna.exe
if %errorlevel% neq 0 (
    echo [ERROR] Compilation failed.
    pause
    exit /b 1
)

echo.
echo [SUCCESS] Build complete!
echo You can now run bin\Luna.exe
pause
