rem compiling as 32-bit has problems with this version of Qt and msvc post 2017u6 (QTBUG-67259)
rem so we compile as 64-bit seeing how the xp build is 32 by definition

call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
mkdir build
cd build
powershell -Command "Invoke-WebRequest http://download.qt.io/official_releases/jom/jom.zip -O jom.zip"
7z x jom.zip
qmake ..\czateria.pro -spec win32-msvc
jom all
