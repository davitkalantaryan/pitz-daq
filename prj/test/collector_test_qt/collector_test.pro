# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

DEFINES += DMSG
#DEFINES += DEBUG_APP
DEFINES += NEW_GETTER_THREAD
#DEFINES += CPP11_THREADS_IMPLEMENT_HERE
DEFINES += USE_DOOCS

CONFIG -= qt

win32{
    SOURCES += \

} else {
    GCCPATH = $$system(which gcc)
    message("!!!!!!!!!!! GCCPATH=$$GCCPATH")
    QMAKE_CXXFLAGS += -std=c++11
    SOURCES += \

}

include($${PWD}/../../common/common_qt/root_no_gui_common.pri)

#LIBS += -L/afs/ifh.de/group/pitz/doocs/data/ers/sys/$$CODENAME/lib

# LIBS += -lMCclass
SOURCES += \
    $${PWD}/../../../src/test/main_collector_test.cpp

HEADERS += ../../../src/test/test_collector_test_reader_common.h


HEADERS += \
    $${PWD}/../../../src/server/pitz_daq_singleentrydoocs.hpp \
    $${PWD}/../../../src/server/pitz_daq_eqfcteventbased.cpp.hpp \
    $${PWD}/../../../src/server/pitz_daq_eqfcteventbased.hpp

OTHER_FILES += \
    $${PWD}/../../../src/server/pitz_daq_eqfctudpmcast.cpp \
    $${PWD}/../../../src/server/pitz_daq_eqfctrr.cpp \
    $${PWD}/../../../src/examples/zmqget.cc
