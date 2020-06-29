# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
# CONFIG += 2test

#QMAKE_CXXFLAGS += "-include types.h"
#DEFINES += u_int=unsigned

#DEFINES += DEBUG_APP

include($${PWD}/../../common/common_qt/daqcollector_doocs_based_props_common.pri)
include( $${PWD}/../../common/common_qt/daqcollector_common.pri )

INCLUDEPATH += $${PWD}/../../../contrib/matlab/include

SOURCES += \
	$${PWD}/../../../src/server/pitz_daq_eqfctrr.cpp

HEADERS += \
    $${PWD}/../../../src/server/pitz_daq_eqfctrr.hpp

