# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ


#DEFINES += DEBUG_APP
include($${PWD}/../../common/common_qt/daqcollector_event_based_common.pri)

# LIBS += -lMCclass
SOURCES += \
    $${PWD}/../../../src/server/pitz_daq_eqfctudpmcast.cpp              \
    $${PWD}/../../../src/common/mclistener.cpp

HEADERS += \
    $${PWD}/../../../src/server/pitz_daq_eqfctudpmcast.hpp              \
    $${PWD}/../../../include/udpmcastdaq_commonheader.h                 \
    $${PWD}/../../../include/mclistener.hpp

