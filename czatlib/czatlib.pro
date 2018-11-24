TEMPLATE = lib
CONFIG += staticlib

include(../czateria.pri)

QT += core network websockets

SOURCES += room.cpp \
  roomlistmodel.cpp \
    captcha.cpp \
    loginsession.cpp \
    chatsession.cpp \
    message.cpp \
    user.cpp \
    userlistmodel.cpp \
    icons.cpp

HEADERS += room.h \
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
    avatarhandler.h
