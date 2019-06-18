TEMPLATE = app

include(../czateria.pri)

QT       += core gui network widgets websockets

INCLUDEPATH += ..

SOURCES += \
  appsettings.cpp \
        main.cpp \
        mainwindow.cpp \
    captchadialog.cpp \
    mainchatwindow.cpp \
    chatwindowtabwidget.cpp \
    userlistview.cpp

HEADERS += \
  appsettings.h \
        mainwindow.h \
    captchadialog.h \
    mainchatwindow.h \
    chatwindowtabwidget.h \
    userlistview.h

FORMS += \
  chatwidget.ui \
        mainwindow.ui \
    captchadialog.ui

win32:CONFIG (release, debug|release): LIBS += -L../czatlib/release -lczatlib
else:win32:CONFIG (debug, debug|release): LIBS += -L../czatlib/debug -lczatlib
else:unix: LIBS += -L../czatlib -lczatlib

unix: PRE_TARGETDEPS += ../czatlib/libczatlib.a

RESOURCES += \
    rsrc.qrc
