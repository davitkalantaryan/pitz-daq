# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
INCLUDEPATH += ../../../src/tmp
#DEFINES += DEBUG_APP
include(../../common/common_qt/daqcollector_common.pri)

# LIBS += -lMCclass
SOURCES += \
    $${PWD}/../../../src/server/pitz_daq_eqfctudpmcast.cpp \
    $${PWD}/../../../src/tmp/mclistener.cpp
HEADERS += \
    $${PWD}/../../../src/server/pitz_daq_eqfctudpmcast.hpp \
    $${PWD}/../../../src/tmp/MCclass.h \
    $${PWD}/../../../include/udpmcastdaq_commonheader.h
