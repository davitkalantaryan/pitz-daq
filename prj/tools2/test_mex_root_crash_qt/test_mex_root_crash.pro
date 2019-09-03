# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

win32{
    contains(QMAKE_TARGET.arch, x86_64): CODENAME = win64
    else:   CODENAME = win32
} else:macx {
    CODENAME = mac
} else:unix {
    CODENAME = $$system(lsb_release -c | cut -f 2)
    GCCPATH = $$system(which gcc)
    message("!!!!!!!!!!! GCCPATH=$$GCCPATH")
}

# Root configuration
MYROOT_SYS_DIR = $$system(env | grep ROOT_SYS_DIR)
equals($$MYROOT_SYS_DIR,"") {
    MYROOT_SYS_DIR = /afs/ifh.de/group/pitz/data/ers/sys/$${CODENAME}/opt/root/current
    message("!!! MYROOT_SYS_DIR set in the project file: $$MYROOT_SYS_DIR")
} else {
    message("!!! MYROOT_SYS_DIR comes from environment: $$MYROOT_SYS_DIR")
}
QMAKE_CXXFLAGS += $$system($$MYROOT_SYS_DIR/bin/root-config --cflags)
LIBS += $$system($$MYROOT_SYS_DIR/bin/root-config --libs)
# message("ROOTCFLAGS=$$ROOTCFLAGS")
# INCLUDEPATH += $$MYROOT_SYS_DIR/include

# C++ 11 story
optionsCpp11 = $$find(CONFIG, "cpp11")
count(optionsCpp11, 1):QMAKE_CXXFLAGS += -std=c++0x
#DEFINES += nullptr=NULL
QMAKE_CXXFLAGS += -std=c++0x

include(../../common/common_qt/mex_common.pri)

TARGET_EXT = mexa64
#TARGET = mexdaq_browser2.mexa64
TEMPLATE = lib

CONFIG += debug
QT -= core
QT -= gui

SOURCES += \
    $${PWD}/../../../src/tools2/entry_test_mex_root_crash.cpp

HEADERS += \
    $${PWD}/../../../src/tools2/daq_root_reader.hpp

OTHER_FILES += \
    $${PWD}/../../../src/tools/mex_daq_browser.cpp  \
    $${PWD}/../../../src/tools2/daq_root_reader.cpp \
    $${PWD}/../../../src/tools2/entry_mex_daq_browser2.cpp
