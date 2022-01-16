cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(SET CMP0091 NEW)
project(czateria CXX)

if(TARGET_WINDOWS_XP)
    # this is an internal cmake variable set by the MSVC support module,
    # but there is no other way to change the subsystem passed to the linker.
    set(CMAKE_CREATE_WIN32_EXE "/subsystem:windows,5.01")
    # used by newer cmake
    set(CMAKE_CXX_CREATE_WIN32_EXE "/subsystem:windows,5.01")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_compile_definitions(QT_DEPRECATED_WARNINGS QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

if(MSVC)
  add_compile_options(-WX)
elseif(${CMAKE_CXX_COMPILER} STREQUAL "GNU")
  add_compile_options(-Wextra -Werror -Wno-deprecated-copy)
elseif(${CMAKE_CXX_COMPILER} MATCHES "Clang")
  add_compile_options(-Wextra -Werror -Wmismatched-tags)
endif()

set(QT_VERSION 5 CACHE STRING "Qt version to use")
set_property(CACHE QT_VERSION PROPERTY STRINGS 5 6)

set(qt_ver_pfx "Qt${QT_VERSION}")

find_package(${qt_ver_pfx}
  REQUIRED COMPONENTS Core Gui Network WebSockets
  OPTIONAL_COMPONENTS Widgets DBus)

add_subdirectory(czatlib)
if(${${qt_ver_pfx}Widgets_FOUND})
  add_subdirectory(ui)
endif()
