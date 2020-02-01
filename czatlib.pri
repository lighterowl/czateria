QT *= core gui

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../czatlib/release/ -lczatlib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../czatlib/debug/ -lczatlib
else:unix: LIBS += -L$$OUT_PWD/../czatlib/ -lczatlib

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD/czatlib

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../czatlib/release/libczatlib.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../czatlib/debug/libczatlib.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../czatlib/release/czatlib.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../czatlib/debug/czatlib.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../czatlib/libczatlib.a
