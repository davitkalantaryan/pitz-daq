# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

win32{
    SOURCES += \

} else {
    GCCPATH = $$system(which gcc)
    message("!!!!!!!!!!! GCCPATH=$$GCCPATH")
    QMAKE_CXXFLAGS += -std=c++11
    SOURCES += \

}

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
