# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

optionsCpp11 = $$find(CONFIG, "cpp11")
count(optionsCpp11, 1):QMAKE_CXXFLAGS += -std=c++0x

# message("!!! root_no_gui_common.pri: ROOT_FLAGS=$$MYROOTCFLAGS")
#LIBS += $$system($$MYROOTSYS/bin/root-config --libs)
CONFIG += debug
QT -= core
QT -= gui

include($${PWD}/../../common/common_qt/sys_common.pri)

INCLUDEPATH += $${PWD}/../../../include
INCLUDEPATH += $${PWD}/../../../contrib/data_handling/include


SOURCES += \
    $${PWD}/../../../src/test/main_udp_mcast_sender_test.cpp    \
    $${PWD}/../../../src/tmp/MCclass.cc

HEADERS += \
