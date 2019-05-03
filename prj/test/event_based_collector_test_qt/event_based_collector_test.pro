# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ




#DEFINES += DEBUG_APP
#include(../../common/common_qt/daqcollector_event_based_common.pri)
include(../../common/common_qt/doocs_client_common.pri)

INCLUDEPATH += $${PWD}/../../../include
INCLUDEPATH += $${PWD}/../../../contrib/cpp-raft/include

LIBS += -lzmq

#QMAKE_CXXFLAGS += -std=c++17
#CONFIG += c++17

# LIBS += -lMCclass
SOURCES += \
    ../../../src/test/main_event_based_collector_test.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/shared_mutex_cpp14.cpp

HEADERS += \
    ../../../src/server/pitz_daq_eqfctudpmcast.hpp \
    ../../../src/tmp/MCclass.h \
    ../../../include/udpmcastdaq_commonheader.h \
    $${PWD}/../../../include/event_based_common_header.h \
    ../../../include/mclistener.hpp \
    $${PWD}/../../../contrib/cpp-raft/include/cpp11+/shared_mutex_cpp14.hpp
