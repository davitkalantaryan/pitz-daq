# File pitzrf1mtca_server.pro
# File created : 27 Jun 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
MYROOTSYS = /afs/ifh.de/@sys/products/root64/5.20.00
MYROOTCFLAGS = `$$MYROOTSYS/bin/root-config \
    --cflags`
QMAKE_CXXFLAGS += $$MYROOTCFLAGS
QMAKE_CFLAGS += $$MYROOTCFLAGS
message("!!! daqcollector_common.pri: ROOT_FLAGS=$$MYROOTCFLAGS")
INCLUDEPATH += /afs/ifh.de/@sys/products/root64/5.20.00/include

# LIBS += `$$MYROOTSYS/bin/root-config --libs`
LIBS += $$system($$MYROOTSYS/bin/root-config --libs)
include(../../common/common_qt/doocs_server_common.pri)
QT -= core
QT -= gui
INCLUDEPATH += ../../../include
INCLUDEPATH += ../../../src/server
INCLUDEPATH += ../../../src/tools
SOURCES += ../../../src/tmp/mailsender.cpp \
    ../../../src/tmp/pitzrf1fpga_rpc_server.cc \
    ../../../src/server/pitz_daq_collectorproperties.cpp \
    ../../../src/tmp/stringparser1.cpp
HEADERS += ../../../src/tmp/eq_pitzrf1fpga.h \
    ../../../src/server/pitz_daq_collectorproperties.hpp
OTHER_FILES += ../../../doc/daqcollector.udoc
