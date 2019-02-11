# File daqinterface.pro
# File created : 24 Apr 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
include(../../common/common_qt/doocs_client_common.pri)
DEFINES += use_mat_matrix
TEMPLATE = app
INCLUDEPATH += /doocs/lib/matlab/R2010a/include
INCLUDEPATH += /doocs/lib/matlab/R2016a/include
INCLUDEPATH += ../../../include
INCLUDEPATH += ../../../src/client

greaterThan(QT_MAJOR_VERSION, 4):QT += widgets
DEFINES += __toLatin1_SHOULD_BE_DEFINED__

HEADERS += \
    ../../../src/tools/daqinterface.hpp \
    ../../../src/tools/daqinterface_config.h

SOURCES += \
    ../../../src/tools/daqinterface.cpp \
    ../../../src/tools/main_daqinterface.cpp
