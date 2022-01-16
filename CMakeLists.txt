cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(czateria CXX)

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
