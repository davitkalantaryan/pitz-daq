# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

optionsCpp11 = $$find(CONFIG, "cpp11")
count(optionsCpp11, 1):QMAKE_CXXFLAGS += -std=c++0x

# message("!!! root_no_gui_common.pri: ROOT_FLAGS=$$MYROOTCFLAGS")
LIBS += $$system($$MYROOTSYS/bin/root-config --libs)
CONFIG += debug
QT -= core
QT -= gui

include(../../common/common_qt/sys_common.pri)


SOURCES += ../../../src/test/main_test_udp_receiver.cpp
