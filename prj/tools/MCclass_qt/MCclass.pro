


VERSION = 2.0.10

CONFIG += debug
DEFINES += LINUX
DEFINES += DO_NEW_TEST
QT -= core
QT -= gui
DEFINES += ___QT___
TEMPLATE = lib
INCLUDEPATH += /doocs/develop/common/include

include (../../common/common_qt/sys_common.pri)

HEADERS += ../../../src/tmp/MCclass.h
SOURCES += ../../../src/tmp/MCclass.cc \
    ../../../src/tmp/mclistener.cpp
