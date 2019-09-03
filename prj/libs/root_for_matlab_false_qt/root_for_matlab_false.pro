# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ


#DEFINES += DO_TEST_WITH_MAIN
TEMPLATE = lib

win32{

} else {
    GCCPATH = $$system(which gcc)
    message("!!!!!!!!!!! GCCPATH=$$GCCPATH")
}

include ($${PWD}/../../../contrib/matlab/prj/common/common_qt/matlab_matrix_common.pri)

message("!!!!!!!!!!! CODENAME=$$CODENAME")

# Root configuration
MYROOT_SYS_DIR = $$system(env | grep ROOT_SYS_DIR)
equals($$MYROOT_SYS_DIR,"") {
    MYROOT_SYS_DIR = /afs/ifh.de/group/pitz/data/ers/sys/$${CODENAME}/opt/root/current
    message("!!! MYROOT_SYS_DIR set in the project file: $$MYROOT_SYS_DIR")
} else {
    message("!!! MYROOT_SYS_DIR comes from environment: $$MYROOT_SYS_DIR")
}
QMAKE_CXXFLAGS += $$system($$MYROOT_SYS_DIR/bin/root-config --cflags)
DEFINES += R__NULLPTR


LIBS += $$system($$MYROOT_SYS_DIR/bin/root-config --libs)


# message("ROOTCFLAGS=$$ROOTCFLAGS")
# INCLUDEPATH += $$MYROOT_SYS_DIR/include

# C++ 11 story
#optionsCpp11 = $$find(CONFIG, "cpp11")
#count(optionsCpp11, 1):QMAKE_CXXFLAGS += -std=c++0x
##DEFINES += nullptr=NULL
QMAKE_CXXFLAGS += -std=c++0x

#include(../../common/common_qt/mex_common.pri)

#TARGET_EXT = mexa64
TARGET = mexdaq_browser2.mexa64

# message("!!! root_no_gui_common.pri: ROOT_FLAGS=$$MYROOTCFLAGS")
#LIBS += $$system($$MYROOTSYS/bin/root-config --libs)
LIBS += -L$${PWD}/../../../sys/$$CODENAME/lib
CONFIG += debug
QT -= core
QT -= gui


#INCLUDEPATH += /afs/ifh.de/@sys/products/root64/5.20.00/include
#INCLUDEPATH += ../../../include
#INCLUDEPATH += ../../../contrib/cpp-raft/include

INCLUDEPATH += $${PWD}/../../../src/tools2

SOURCES += \
    $${PWD}/../../../src/tools2/pitz_daq_root_for_matlab_false.cpp          \
    $${PWD}/../../../src/tools2/daq_root_reader.cpp                         \
    $${PWD}/../../../src/tools2/pitz_daq_data_memory_base.cpp               \
    $${PWD}/../../../src/tools2/pitz_daq_data_indexing.cpp                  \
    $${PWD}/../../../src/tools2/pitz_daq_data_memory_forclient.cpp          \
    $${PWD}/../../../src/tools2/pitz_daq_data_entryinfo.cpp

HEADERS += \
    $${PWD}/../../../src/tools2/pitz_daq_root_for_matlab_false.hpp

OTHER_FILES += \
