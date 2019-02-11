#
# File daq_browser.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
#

message("daq_browser.pro")
include(../../common/common_qt/mex_common.pri)


INCLUDEPATH += ../../../include


SOURCES += ../../../src/tools/daq_browser_mex.cpp \
    ../../../src/utils/createnewprocess1.cpp \
    ../../../src/utils/shared_memory_svr.cpp \
    ../../../src/utils/shared_memory_base.cpp
HEADERS += ../../../include/browsing_funcs.h \
    ../../../include/createnewprocess1.h \
    ../../../include/shared_memory_svr.h \
    ../../../include/shared_memory_base.h \
    ../../../include/commonheader_root.h
