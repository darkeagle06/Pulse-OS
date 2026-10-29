@echo off
echo === Compilers ===
where cl 2>nul && cl 2>&1 | findstr /i "version" || echo cl.exe: NOT FOUND
where g++ 2>nul && g++ --version 2>&1 | findstr /i "g++" || echo g++: NOT FOUND
where clang++ 2>nul && clang++ --version 2>&1 | findstr /i "clang" || echo clang++: NOT FOUND

echo.
echo === Build Tools ===
where cmake 2>nul && cmake --version 2>&1 | findstr /i "cmake" || echo cmake: NOT FOUND
where ninja 2>nul && ninja --version || echo ninja: NOT FOUND
where make 2>nul || echo make: NOT FOUND
where mingw32-make 2>nul || echo mingw32-make: NOT FOUND

echo.
echo === Git ===
where git 2>nul && git --version || echo git: NOT FOUND

echo.
echo === Qt Check ===
where qmake 2>nul && qmake --version || echo qmake: NOT FOUND
where qmake6 2>nul || echo qmake6: NOT FOUND
if exist "C:\Qt" (echo Found C:\Qt & dir /b "C:\Qt") else (echo C:\Qt: NOT FOUND)
if exist "%USERPROFILE%\Qt" (echo Found %USERPROFILE%\Qt & dir /b "%USERPROFILE%\Qt") else (echo %USERPROFILE%\Qt: NOT FOUND)
if exist "D:\Qt" (echo Found D:\Qt) else (echo D:\Qt: NOT FOUND)

echo.
echo === Visual Studio ===
set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist %VSWHERE% (
    %VSWHERE% -all -property displayName
    %VSWHERE% -all -property installationPath
    %VSWHERE% -all -property installationVersion
) else (
    echo vswhere.exe: NOT FOUND
)

echo.
echo === System ===
systeminfo | findstr /C:"OS Name" /C:"OS Version" /C:"System Type"

echo.
echo === Environment Variables ===
echo Qt6_DIR=%Qt6_DIR%
echo CMAKE_PREFIX_PATH=%CMAKE_PREFIX_PATH%
echo QTDIR=%QTDIR%
echo PATH truncated:
echo %PATH:;=&echo.%

echo.
echo === DONE ===

