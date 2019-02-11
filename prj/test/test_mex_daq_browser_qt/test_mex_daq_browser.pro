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

QMAKE_CXXFLAGS += -std=c++0x

include(../../common/common_qt/sys_common.pri)
#include(../../common/common_qt/matlab_matrix_common.pri)


# message("!!! root_no_gui_common.pri: ROOT_FLAGS=$$MYROOTCFLAGS")
#LIBS += $$system($$MYROOTSYS/bin/root-config --libs)
LIBS += -L../../../../sys/$$CODENAME/lib
LIBS += -L../../../contrib/matlab/sys/glnxa64
#LIBS += -lmat
#LIBS += -lformexdaq_browser
CONFIG += debug
QT -= core
QT -= gui

#INCLUDEPATH += /afs/ifh.de/@sys/products/root64/5.20.00/include
INCLUDEPATH += ../../../include
INCLUDEPATH += ../../../contrib/cpp-raft/include

SOURCES += \
    ../../../src/test/main_test_mex_daq_browser.cpp \
    ../../../src/tools/pitz_daq_data_memory_base.cpp \
    ../../../src/tools/pitz_daq_data_getter_base.cpp \
    ../../../src/tools/pitz_daq_data_engine_base.cpp \
    ../../../src/tools/pitz_daq_data_getter_tobuffer.cpp \
    ../../../src/tools/pitz_daq_data_entryinfo.cpp \
    ../../../src/pitz_daq_base.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/thread_cpp11.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/mutex_cpp11.cpp \
    ../../../src/tools/pitz_daq_data_getter_noindexing.cpp \
    ../../../src/tools/pitz_daq_data_getter_frompipe.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/shared_mutex_cpp14.cpp \
    ../../../src/tools/pitz_daq_data_bypipe_base.cpp \
    ../../../src/tools/pitz_daq_data_engine_bypipe.cpp

HEADERS += \
    ../../../include/pitz/daq/callbackn.hpp \
    ../../../include/pitz/daq/base.hpp \
    ../../../include/pitz/daq/data/indexer.hpp \
    ../../../include/pitz/daq/data/entryinfo.hpp \
    ../../../include/pitz/daq/data/engine/base.hpp \
    ../../../include/pitz/daq/data/getter/base.hpp \
    ../../../include/pitz/daq/data/memory/base.hpp \
    ../../../src/tools/pitz_daq_data_engine_branchitemprivate.hpp \
    ../../../include/pitz/daq/data/engine/frompipe.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/thread_cpp11.impl.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/thread_cpp11.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/mutex_cpp11.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/common_defination.h \
    ../../../src/tools/bin_for_mexdaq_browser_common.h \
    ../../../include/pitz/daq/data/getter/impl.tobuffer.hpp \
    ../../../include/pitz/daq/data/getter/tobuffer.hpp \
    ../../../include/pitz/daq/data/getter/noindexing.hpp

