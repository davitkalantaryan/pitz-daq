# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

#QMAKE_CXXFLAGS += $$MYROOTCFLAGS
#QMAKE_CFLAGS += $$MYROOTCFLAGS
# direct including root is not possible

optionsCpp11 = $$find(CONFIG, "cpp11")
count(optionsCpp11, 1):QMAKE_CXXFLAGS += -std=c++0x

#include(../../common/common_qt/mex_common.pri)

# message("!!! root_no_gui_common.pri: ROOT_FLAGS=$$MYROOTCFLAGS")
#LIBS += $$system($$MYROOTSYS/bin/root-config --libs)
LIBS += -L../../../../sys/$$CODENAME/lib
LIBS += -ldl
#LIBS += -lformexdaq_browser
CONFIG += debug
QT -= core
QT -= gui

#INCLUDEPATH += /afs/ifh.de/@sys/products/root64/5.20.00/include
INCLUDEPATH += ../../../include

SOURCES += \
    ../../../src/test/main_formexdaq_browser_test.cpp \
    ../../../src/tools/formexdaq_browser_dynuser.cpp

HEADERS += \
    ../../../include/tool/simple_root_reader.h \
    ../../../include/tool/formexdaq_browser_dynuser.h
