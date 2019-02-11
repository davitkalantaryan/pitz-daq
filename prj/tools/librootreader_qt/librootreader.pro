#
# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
#

include(../../common/common_qt/root_no_gui_common.pri)
include(../../common/common_qt/sys_common.pri)

QT -= core
QT -= gui
TEMPLATE = lib

INCLUDEPATH += ../../../include

SOURCES += ../../../src/tools/browsing_funcs.cpp
HEADERS += ../../../include/browsing_funcs.h
