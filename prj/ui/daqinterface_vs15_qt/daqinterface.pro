# File daqinterface.pro
#
# File created : 25 Jan 2017
# Created by   : Davit Kalantaryan (davit.kalantaryan@desy.de)
#
# This file can be used to produce Makefile for daqinterface GUI application
# for PITZ
#
# daqinterface GUI is QT based application for controling PITZ DAQ servers
# daqinterface QT application will be replaced by JAVA based application
#

#
#CONFIG += TEST
# For making test: '$qmake "CONFIG+=TEST" daqadcreceiver.pro'  , then '$make'

include(../../lib/common_qt/doocs_client_common.pri)
include(../../lib/common_qt/gui_common.pri)


HEADERS += \
    ../../../src/tools/daqinterface.hpp \
    ../../../src/tools/daqinterface_config.h

SOURCES += \
    ../../../src/tools/daqinterface.cpp \
    ../../../src/tools/main_daqinterface.cpp
