TEMPLATE = app

include(../czateria.pri)
include(../czatlib.pri)

QT       += network widgets websockets
unix:qtHaveModule(dbus) {
  QT += dbus
  SOURCES += notificationsupport_dbus.cpp
  HEADERS += notificationsupport_dbus.h
}
win32 {
  SOURCES += notificationsupport_win10.cpp ../WinToast/src/wintoastlib.cpp
  HEADERS += notificationsupport_win10.h ../WinToast/src/wintoastlib.h
  DEFINES += UNICODE PSAPI_VERSION=1
}


SOURCES += \
    appsettings.cpp \
    autologindatadialog.cpp \
    main.cpp \
    mainwindow.cpp \
    captchadialog.cpp \
    mainchatwindow.cpp \
    chatwindowtabwidget.cpp \
    notificationsupport.cpp \
    notificationsupport_msgbox.cpp \
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
    notificationsupport.h \
    notificationsupport_msgbox.h \
    notificationsupport_native.h \
    userlistview.h \
    util.h \
  qthttpsocket.h \
  qtwebsocket.h \
    userlistview.h

FORMS += \
    autologindatadialog.ui \
    chatwidget.ui \
    mainwindow.ui \
    captchadialog.ui

RESOURCES += rsrc.qrc
