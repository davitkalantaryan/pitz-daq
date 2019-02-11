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

INCLUDEPATH += ../../../include

SOURCES += ../../../src/tools/browsing_funcs.cpp \
    ../../../src/tools/main_rootreader.cpp \
    ../../../src/utils/shared_memory_clt.cpp \
    ../../../src/utils/shared_memory_base.cpp \
    ../../../src/utils/alog.cpp
HEADERS += ../../../include/commonheader_root.h \
    ../../../include/browsing_funcs.h \
    ../../../include/shared_memory_clt.h \
    ../../../include/shared_memory_base.h \
    ../../../include/alog.h
