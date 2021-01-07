TEMPLATE = app

include(../czateria.pri)

QT       += core gui network widgets websockets

unix:qtHaveModule(dbus) {
  QT += dbus
  SOURCES += notificationsupport_dbus.cpp
  HEADERS += notificationsupport_dbus.h
}
win32 {
  SOURCES += notificationsupport_win10.cpp ../WinToast/src/wintoastlib.cpp
  HEADERS += notificationsupport_win10.h ../WinToast/src/wintoastlib.h
  DEFINES += UNICODE PSAPI_VERSION=1
  RC_ICONS += icons/czateria.ico
}

INCLUDEPATH += ..

SOURCES += \
    appsettings.cpp \
    autologindatadialog.cpp \
    filebasedlogger.cpp \
    main.cpp \
    mainwindow.cpp \
    captchadialog.cpp \
    mainchatwindow.cpp \
    chatwindowtabwidget.cpp \
    notificationsupport.cpp \
    notificationsupport_msgbox.cpp \
    settingsbasedblocker.cpp \
    settingsdialog.cpp \
    userlistview.cpp

HEADERS += \
    appsettings.h \
    autologindatadialog.h \
    filebasedlogger.h \
    mainwindow.h \
    captchadialog.h \
    mainchatwindow.h \
    chatwindowtabwidget.h \
    notificationsupport.h \
    notificationsupport_msgbox.h \
    notificationsupport_native.h \
    settingsbasedblocker.h \
    settingsdialog.h \
    userlistview.h \
    util.h

FORMS += \
    autologindatadialog.ui \
    chatsettingsform.ui \
    chatwidget.ui \
    mainwindow.ui \
    captchadialog.ui \
    settingsdialog.ui

win32:CONFIG (release, debug|release): LIBS += -L../czatlib/release -lczatlib
else:win32:CONFIG (debug, debug|release): LIBS += -L../czatlib/debug -lczatlib
else:unix: LIBS += -L../czatlib -lczatlib

unix: PRE_TARGETDEPS += ../czatlib/libczatlib.a

RESOURCES += rsrc.qrc
