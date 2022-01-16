powershell -Command "Invoke-WebRequest http://download.qt.io/official_releases/jom/jom.zip -O jom.zip"
7z x jom.zip

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
for %%g in (Debug,Release) DO (
  mkdir build%%g
  cd build%%g
  cmake -DCMAKE_BUILD_TYPE=%%g -G "NMake Makefiles" ..
  ..\jom.exe all
  cd ..
)
