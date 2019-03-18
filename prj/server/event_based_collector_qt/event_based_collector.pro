# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
INCLUDEPATH += ../../../src/tmp
#DEFINES += DEBUG_APP
include(../../common/common_qt/daqcollector_event_based_common.pri)

# LIBS += -lMCclass
SOURCES += ../../../src/server/pitz_daq_eqfctudpmcast.cpp \
    ../../../src/tmp/mclistener.cpp
HEADERS += ../../../src/server/pitz_daq_eqfctudpmcast.hpp \
    ../../../src/tmp/MCclass.h \
    ../../../include/udpmcastdaq_commonheader.h
