# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

DEFINES += DMSG
#DEFINES += DEBUG_APP


include(../../common/common_qt/daqcollector_event_based_common.pri)

INCLUDEPATH += $$MYDOOCS/include/zmq

# LIBS += -lMCclass
SOURCES += \
    $${PWD}/../../../src/server/pitz_daq_eqfcteventbased.cpp \
    $${PWD}/../../../src/tmp/mclistener.cpp
HEADERS += \
    $${PWD}/../../../src/server/pitz_daq_eqfcteventbased.hpp

OTHER_FILES += \
    $${PWD}/../../../src/server/pitz_daq_eqfctudpmcast.cpp \
    $${PWD}/../../../src/examples/zmqget.cc
