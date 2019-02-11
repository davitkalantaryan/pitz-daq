# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

CONFIG += c++11

QT -= core
QT -= gui

#DEFINES += DEBUG_APP
include(../../common/common_qt/sys_common.pri)

INCLUDEPATH += ../../../src/tmp
INCLUDEPATH += ../../../include
INCLUDEPATH += ../../../include/ilock4
INCLUDEPATH += ../../../contrib/cpp-raft/include

# LIBS += -lMCclass
SOURCES += \
    ../../../src/client/main_interlock_notifier_client.cpp \
    ../../../contrib/cpp-raft/common/common_sockettcp.cpp \
    ../../../contrib/cpp-raft/common/common_socketbase.cpp \
    ../../../contrib/cpp-raft/common/common_iodevice.cpp \
    ../../../src/client/desy_interlocknotifier_client.cpp \
    ../../../src/common/common_argument_parser.cpp
HEADERS += \
    ../../../include/desy/interlocknotifier/client.hpp \
    ../../../include/desy/interlocknotifier/server_client_common.h \
    ../../../contrib/cpp-raft/include/common/newlockguards.impl.hpp \
    ../../../contrib/cpp-raft/include/common/newlockguards.hpp \
    ../../../contrib/cpp-raft/include/common_sockettcp.hpp \
    ../../../contrib/cpp-raft/include/common_socketbase.impl.hpp \
    ../../../contrib/cpp-raft/include/common_socketbase.hpp \
    ../../../contrib/cpp-raft/include/common_iodevice.hpp \
    ../../../contrib/cpp-raft/include/common_defination.h \
    ../../../include/common/common_argument_parser.impl.hpp \
    ../../../include/common/common_argument_parser.hpp
