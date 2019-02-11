# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

TEMPLATE = lib
VERSION = 5.0

MYROOTSYS = /afs/ifh.de/group/pitz/doocs/@sys/root/5.30.00
MYROOTCFLAGS = `$$MYROOTSYS/bin/root-config \
    --cflags`
#QMAKE_CXXFLAGS += $$MYROOTCFLAGS
#QMAKE_CFLAGS += $$MYROOTCFLAGS
optionsCpp11 = $$find(CONFIG, "cpp11")
count(optionsCpp11, 1):QMAKE_CXXFLAGS += -std=c++0x

include(../../common/common_qt/sys_common.pri)

# message("!!! root_no_gui_common.pri: ROOT_FLAGS=$$MYROOTCFLAGS")
LIBS += $$system($$MYROOTSYS/bin/root-config --libs)
LIBS += -ldcap
CONFIG += debug
QT -= core
QT -= gui

#INCLUDEPATH += /afs/ifh.de/@sys/products/root64/5.20.00/include
INCLUDEPATH += $$MYROOTSYS/include
INCLUDEPATH += ../../../include

SOURCES += \
    ../../../src/tools/libsimple_root_reader.cpp

HEADERS += \
    ../../../include/tool/simple_root_reader.h
