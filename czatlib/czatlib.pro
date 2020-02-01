TEMPLATE = lib
CONFIG += staticlib

include(../czateria.pri)

# the gui dependency kind of sucks, but CzatSession needs to be able to put the
# image width and height into the JSON image sent to the server, and optionally
# scale it if larger than the maximum allowed size. doing it via QImage (which
# is located in the gui module) is simply the best way to achieve that with a
# minimum of fuss.
QT = core gui

SOURCES += room.cpp \
  roomlistmodel.cpp \
    captcha.cpp \
    loginsession.cpp \
    chatsession.cpp \
    message.cpp \
    user.cpp \
    userlistmodel.cpp \
    icons.cpp \
  util.cpp

HEADERS += room.h \
  httpsocket.h \
  roomlistmodel.h \
    loginfailreason.h \
    captcha.h \
    loginsession.h \
    chatsession.h \
    message.h \
    user.h \
    userlistmodel.h \
    icons.h \
    conversationstate.h \
    util.h \
    avatarhandler.h \
  websocket.h
