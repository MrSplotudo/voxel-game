@echo off
setlocal

set DIST=Rounder-dist

:: Clean previous dist
if exist %DIST% rmdir /s /q %DIST%

:: Build Release
if not exist build mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
cd ..

:: Create distribution folder structure
:: exe goes in bin/ so that "../shaders", "../assets", "../levels" resolve correctly
mkdir %DIST%\bin
mkdir %DIST%\shaders
mkdir %DIST%\assets
mkdir %DIST%\levels

:: Copy executable
copy build\Release\Rounder.exe %DIST%\bin\
:: If your build output is in build\Rounder.exe instead (depends on generator):
if not exist %DIST%\bin\Rounder.exe copy build\Rounder.exe %DIST%\bin\

:: Copy runtime files
copy shaders\vert.spv %DIST%\shaders\
copy shaders\frag.spv %DIST%\shaders\
xcopy assets %DIST%\assets /E /I /Q
xcopy levels %DIST%\levels /E /I /Q

:: Create launcher in dist root
(
echo @echo off
echo cd /d "%%~dp0bin"
echo start Rounder.exe
) > %DIST%\Play.bat

echo.
echo === Done! Send the %DIST% folder to your friend. ===
echo === They just double-click Play.bat ===