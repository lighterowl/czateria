TEMPLATE = app

include(../czateria.pri)
include(../czatlib.pri)

QT       += network widgets websockets

SOURCES += \
  appsettings.cpp \
    autologindatadialog.cpp \
        main.cpp \
        mainwindow.cpp \
    captchadialog.cpp \
    mainchatwindow.cpp \
    chatwindowtabwidget.cpp \
  qthttpsocket.cpp \
  qtwebsocket.cpp \
    userlistview.cpp

HEADERS += \
  appsettings.h \
    autologindatadialog.h \
        mainwindow.h \
    captchadialog.h \
    mainchatwindow.h \
    chatwindowtabwidget.h \
  qthttpsocket.h \
  qtwebsocket.h \
    userlistview.h

FORMS += \
    autologindatadialog.ui \
  chatwidget.ui \
        mainwindow.ui \
    captchadialog.ui

RESOURCES += \
    rsrc.qrc
