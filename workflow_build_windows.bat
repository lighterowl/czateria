call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
mkdir build
cd build
powershell -Command "Invoke-WebRequest http://download.qt.io/official_releases/jom/jom.zip -O jom.zip"
7z x jom.zip
qmake ..\czateria.pro -spec win32-msvc
jom all
