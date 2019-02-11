#
# File pitznoadc2_server.pro
# File created : 02 Feb 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
#

#LIBS += -lMCclass
INCLUDEPATH += ../../../include
INCLUDEPATH += ../../../src/tools

HEADERS += ../../../src/tmp/eq_pitznoadc2.h \
    ../../../include/stringparser1.h
SOURCES += ../../../src/tmp/pitznoadc2_rpc_server.cc \
    ../../../src/tools/stringparser1.cpp \
    ../../../src/tools/mailsender.cpp

QT -= core
QT -= gui

#QMAKE_CXXFLAGS += -H
DEFINES += FIX_UNRESOLVED

include(../../common/daqcollector_common_qt/daqcollector_common.pri)
