# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

GCCPATH = $$system(which gcc)
message("!!!!!!!!!!! GCCPATH=$$GCCPATH")

#MYROOTSYS = /afs/ifh.de/@sys/products/root64/5.20.00
#MYROOTSYS = /opt/root/6.16.00
include($${PWD}/../../common/common_qt/root_no_gui_common.pri)

# just for intelisence
INCLUDEPATH += $$MYROOTSYS/include

QMAKE_CXXFLAGS += -std=c++0x

# message("!!! root_no_gui_common.pri: ROOT_FLAGS=$$MYROOTCFLAGS")
#LIBS += $$system($$MYROOTSYS/bin/root-config --libs)
QT -= core
QT -= gui
CONFIG -= qt

#INCLUDEPATH += /afs/ifh.de/@sys/products/root64/5.20.00/include

SOURCES += ../../../src/test/main_simple_root_reader_test.cpp


HEADERS += ../../../src/test/test_collector_test_reader_common.h
