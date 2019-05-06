# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

#MYROOTSYS = /afs/ifh.de/@sys/products/root64/5.20.00
MYROOTSYS = /opt/root/6.16.00

# just for intelisence
INCLUDEPATH += $$MYROOTSYS/include

MYROOTCFLAGS = `$$MYROOTSYS/bin/root-config --cflags`
QMAKE_CXXFLAGS += $$MYROOTCFLAGS
QMAKE_CFLAGS += $$MYROOTCFLAGS
#optionsCpp11 = $$find(CONFIG, "cpp11")
#count(optionsCpp11, 1):QMAKE_CXXFLAGS += -std=c++0x

# message("!!! root_no_gui_common.pri: ROOT_FLAGS=$$MYROOTCFLAGS")
LIBS += $$system($$MYROOTSYS/bin/root-config --libs)
CONFIG += debug
QT -= core
QT -= gui

#INCLUDEPATH += /afs/ifh.de/@sys/products/root64/5.20.00/include
INCLUDEPATH = $${MYROOTSYS}/include

SOURCES += ../../../src/test/main_simple_root_reader_test.cpp
