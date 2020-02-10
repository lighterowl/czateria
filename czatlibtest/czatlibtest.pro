GTEST_SRCDIR = googletest/googletest
GMOCK_SRCDIR = googletest/googlemock

requires(exists($$GTEST_SRCDIR):exists($$GMOCK_SRCDIR))

INCLUDEPATH *= \
    $$GTEST_SRCDIR \
    $$GTEST_SRCDIR/include \
    $$GMOCK_SRCDIR \
    $$GMOCK_SRCDIR/include

SOURCES += $$GTEST_SRCDIR/src/gtest-all.cc $$GMOCK_SRCDIR/src/gmock-all.cc

include(../czateria.pri)
include(../czatlib.pri)

TEMPLATE = app
CONFIG += console thread
CONFIG -= app_bundle

HEADERS += \
        mocks.h \
        tst_captcha.h \
        tst_sessions.h

SOURCES += \
        main.cpp
