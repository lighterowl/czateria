cmake_minimum_required(VERSION 3.12)
project(czatlib CXX)

add_library(czatlib STATIC room.cpp chatsessionlistener.cpp roomlistmodel.cpp
  captcha.cpp loginsession.cpp chatsession.cpp message.cpp user.cpp 
  userlistmodel.cpp icons.cpp)
