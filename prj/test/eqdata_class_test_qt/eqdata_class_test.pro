# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

GCCPATH = $$system(which gcc)
message("!!!!!!!!!!! GCCPATH=$$GCCPATH")

include( $${PWD}/../../common/common_qt/doocs_client_common.pri )

CONFIG -= qt

SOURCES += \
    $${PWD}/../../../src/test/main_eqdata_class_test.cpp

