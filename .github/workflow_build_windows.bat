call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
for %%g in (Debug,Release) DO (
  mkdir build%%g
  cd build%%g
  cmake -DCMAKE_BUILD_TYPE=%%g -G "NMake Makefiles" ..
  nmake
  cd ..
)
