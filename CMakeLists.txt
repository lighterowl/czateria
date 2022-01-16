cmake_minimum_required(VERSION 3.12)
project(czateria CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

add_compile_definitions(QT_DEPRECATED_WARNINGS QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII)

if(MSVC)
  add_compile_options(-WX)
elseif(${CMAKE_CXX_COMPILER} STREQUAL "GNU")
  add_compile_options(-Wextra -Werror -Wno-deprecated-copy)
elseif(${CMAKE_CXX_COMPILER} MATCHES "Clang")
  add_compile_options(-Wextra -Werror -Wmismatched-tags)
endif()

add_subdirectory(czatlib)
#add_subdirectory(ui)
