TEMPLATE = app

include(../czateria.pri)

QT       += core gui network widgets websockets

unix:qtHaveModule(dbus): QT += dbus

INCLUDEPATH += ..

SOURCES += \
  appsettings.cpp \
    autologindatadialog.cpp \
        main.cpp \
        mainwindow.cpp \
    captchadialog.cpp \
    mainchatwindow.cpp \
    chatwindowtabwidget.cpp \
    notificationsupport.cpp \
    notificationsupport_dbus.cpp \
    notificationsupport_msgbox.cpp \
    notificationsupport_win10.cpp \
    userlistview.cpp

HEADERS += \
  appsettings.h \
    autologindatadialog.h \
        mainwindow.h \
    captchadialog.h \
    mainchatwindow.h \
    chatwindowtabwidget.h \
    notificationsupport.h \
    notificationsupport_dbus.h \
    notificationsupport_msgbox.h \
    notificationsupport_win10.h \
    userlistview.h \
    util.h

FORMS += \
    autologindatadialog.ui \
  chatwidget.ui \
        mainwindow.ui \
    captchadialog.ui

win32:CONFIG (release, debug|release): LIBS += -L../czatlib/release -lczatlib
else:win32:CONFIG (debug, debug|release): LIBS += -L../czatlib/debug -lczatlib
else:unix: LIBS += -L../czatlib -lczatlib

unix: PRE_TARGETDEPS += ../czatlib/libczatlib.a

RESOURCES += \
    rsrc.qrc
