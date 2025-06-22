@echo off
setlocal EnableDelayedExpansion

echo === Eclipse Logger Build and Test Script ===
echo.

rem Create build directory
echo [STATUS] Creating build directory...
if not exist build mkdir build
cd build

rem Configure with CMake
echo [STATUS] Configuring with CMake...
cmake .. -DBUILD_TESTS=ON -DRUN_TESTS_BEFORE_BUILD=ON
if !errorlevel! neq 0 (
    echo [ERROR] CMake configuration failed!
    exit /b 1
)

rem Build the project (this will run tests first due to our CMake configuration)
echo [STATUS] Building project (tests will run automatically before library build)...
cmake --build . --config Release
if !errorlevel! neq 0 (
    echo [ERROR] Build failed!
    exit /b 1
)

rem Run tests explicitly for verification
echo [STATUS] Running tests explicitly for verification...
ctest --output-on-failure --verbose -C Release
if !errorlevel! neq 0 (
    echo [ERROR] Some tests failed!
    exit /b 1
)

echo [STATUS] All tests passed! Build completed successfully.
echo.
echo === Build Artifacts ===
echo Library: %cd%\EclipseCore.lib (or .a on Unix-like systems)
echo Test executables:
dir tests\test_*.exe 2>nul || echo   No test executables found

echo.
echo [STATUS] Build and test process completed successfully!

pause
