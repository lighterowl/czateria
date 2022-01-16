cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(czatlib CXX)

add_library(czatlib STATIC
    room.cpp chatsessionlistener.cpp roomlistmodel.cpp
    captcha.cpp loginsession.cpp chatsession.cpp message.cpp user.cpp
    userlistmodel.cpp icons.cpp chatblocker.h)
target_link_libraries(czatlib ${qt_ver_pfx}::Core ${qt_ver_pfx}::Network
    ${qt_ver_pfx}::WebSockets ${qt_ver_pfx}::Gui)
