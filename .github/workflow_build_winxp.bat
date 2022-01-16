mkdir C:\Qt\
powershell -Command "Invoke-WebRequest https://github.com/xavery/qt563xp/releases/download/1.2/Qt-5.6.3-Static-XP.7z -O qt-xp-static.7z"
7z x -oC:\Qt\ qt-xp-static.7z

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
set INCLUDE=%ProgramFiles(x86)%\Microsoft SDKs\Windows\7.1A\Include;%INCLUDE%
set PATH=%ProgramFiles(x86)%\Microsoft SDKs\Windows\7.1A\Bin;%PATH%
set LIB=%ProgramFiles(x86)%\Microsoft SDKs\Windows\7.1A\Lib;%LIB%

mkdir build
cd build
rem unfortunately the cmake module files shipped with qt 5.6.3 don't include
rem external dependencies, or I don't know how to enable them. this is the only
rem reason why UI_EXTRA_QT_LIBS exist : to pass missing library dependencies to
rem the executable's target_link_libraries. back when qmake was used, this was
rem hidden inside .prl files for Qt5Xyz, so everything worked out of the box.
cmake -DCMAKE_PREFIX_PATH=C:\Qt\5.6.3-Static-XP\lib\cmake^
  -GNinja -DCMAKE_BUILD_TYPE=Release -DTARGET_WINDOWS_XP=TRUE^
  -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded^
  -DUI_EXTRA_QT_LIBS=C:\Qt\5.6.3-Static-XP\lib\libeay32.lib;C:\Qt\5.6.3-Static-XP\lib\qtpcre.lib;C:\Qt\5.6.3-Static-XP\lib\ssleay32.lib;ws2_32;crypt32;dnsapi;iphlpapi^
  ..
ninja
