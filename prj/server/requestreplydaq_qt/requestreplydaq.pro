# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
# CONFIG += 2test

#QMAKE_CXXFLAGS += "-include types.h"
#DEFINES += u_int=unsigned

win32{
    SOURCES += \

} else {
    GCCPATH = $$system(which gcc)
    message("!!!!!!!!!!! GCCPATH=$$GCCPATH")
    #QMAKE_CXXFLAGS += -std=c++0x
    QMAKE_CXXFLAGS += -std=c++11
    SOURCES += \

}

#DEFINES += DEBUG_APP
include( $${PWD}/../../common/common_qt/daqcollector_common.pri )

INCLUDEPATH += $${PWD}/../../../contrib/matlab/include

SOURCES += \
    $${PWD}/../../../src/server/pitz_daq_eqfctrr.cpp                \
    $${PWD}/../../../src/server/pitz_daq_singleentrydoocs.cpp

HEADERS += \
    $${PWD}/../../../src/server/pitz_daq_eqfctrr.hpp

