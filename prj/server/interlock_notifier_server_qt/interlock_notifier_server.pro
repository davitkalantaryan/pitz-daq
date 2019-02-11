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
INCLUDEPATH += ../../../contrib/cpp-raft/include

# LIBS += -lMCclass
SOURCES += \
    ../../../src/server/main_interlock_notifier_server.cpp \
    ../../../src/server/desy_interlocknotifier_server.cpp \
    ../../../contrib/cpp-raft/common/common_sockettcp.cpp \
    ../../../contrib/cpp-raft/common/common_socketbase.cpp \
    ../../../contrib/cpp-raft/common/common_servertcp.cpp \
    ../../../contrib/cpp-raft/common/common_iodevice.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/shared_mutex_cpp14.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/thread_cpp11.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/mutex_cpp11.cpp \
    ../../../src/common/common_argument_parser.cpp
HEADERS += \
    ../../../include/desy/interlocknotifier/server.hpp \
    ../../../include/desy/interlocknotifier/server_client_common.h \
    ../../../contrib/cpp-raft/include/common/newlockguards.impl.hpp \
    ../../../contrib/cpp-raft/include/common/newlockguards.hpp \
    ../../../contrib/cpp-raft/include/common/listspecialandhashtbl.hpp \
    ../../../contrib/cpp-raft/include/common/lists.hpp \
    ../../../contrib/cpp-raft/include/common/impl.listspecialandhashtbl.hpp \
    ../../../contrib/cpp-raft/include/common/impl.lists.hpp \
    ../../../contrib/cpp-raft/include/common/common_hashtbl.impl.hpp \
    ../../../contrib/cpp-raft/include/common/common_hashtbl.hpp \
    ../../../contrib/cpp-raft/include/common_sockettcp.hpp \
    ../../../contrib/cpp-raft/include/common_socketbase.impl.hpp \
    ../../../contrib/cpp-raft/include/common_servertcp.hpp \
    ../../../contrib/cpp-raft/include/common_servertcp.impl.hpp \
    ../../../contrib/cpp-raft/include/common_socketbase.hpp \
    ../../../contrib/cpp-raft/include/common_iodevice.hpp \
    ../../../contrib/cpp-raft/include/common_defination.h \
    ../../../contrib/cpp-raft/include/cpp11+/thread_cpp11.impl.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/thread_cpp11.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/shared_mutex_cpp14.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/mutex_cpp11.hpp \
    ../../../include/common/common_argument_parser.impl.hpp \
    ../../../include/common/common_argument_parser.hpp
