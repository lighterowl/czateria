DEFINES += QT_DEPRECATED_WARNINGS QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII
CONFIG += c++14
*-msvc {
  QMAKE_CXXFLAGS += -WX
}
*-g++ {
  QMAKE_CXXFLAGS += -Wextra -Werror
}
*-clang {
  QMAKE_CXXFLAGS += -Wextra -Wmismatched-tags -Werror
}
