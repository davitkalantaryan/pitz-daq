
include(../../common/common_qt/sys_common.pri)

MYZMQ = /afs/ifh.de/group/pitz/doocs/zmq

# TEMPLATE = lib
CONFIG += debug

# DEFINES += LINUX
DEFINES += ___QT___
INCLUDEPATH += $$MYZMQ/include
INCLUDEPATH += ../../../src/tmp
LIBS += -L$$MYZMQ/lib \
    -lzmq
QT -= core
QT -= gui
HEADERS += ../../../src/tools/daqtimeZMQ_server.h \
    ../../../src/tmp/stringparser1.h
SOURCES += ../../../src/tools/daqtimeZMQ_server.cpp \
    ../../../src/tmp/stringparser1.cpp
