# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
# CONFIG += 2test

#QMAKE_CXXFLAGS += "-include types.h"
#DEFINES += u_int=unsigned

QMAKE_CXXFLAGS += -std=c++0x

#DEFINES += DEBUG_APP
include(../../common/common_qt/daqcollector_common.pri)
SOURCES += ../../../src/server/pitz_daq_eqfctrr.cpp
HEADERS += ../../../src/server/pitz_daq_eqfctrr.hpp
