choco install jom
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
mkdir build
cd build
qmake ..\czateria.pro -spec win32-msvc
jom all
