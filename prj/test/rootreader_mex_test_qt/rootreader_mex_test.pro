# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

# temporary will be deleted to root_reader_base
MYROOTSYS = /opt/root/6.16.00
INCLUDEPATH += $$MYROOTSYS/include
MYROOTCFLAGS = `$$MYROOTSYS/bin/root-config --cflags`
QMAKE_CXXFLAGS += $$MYROOTCFLAGS
QMAKE_CFLAGS += $$MYROOTCFLAGS
LIBS += $$system($$MYROOTSYS/bin/root-config --libs)


#QMAKE_CXXFLAGS += $$MYROOTCFLAGS
#QMAKE_CFLAGS += $$MYROOTCFLAGS
# direct including root is not possible

optionsCpp11 = $$find(CONFIG, "cpp11")
count(optionsCpp11, 1):QMAKE_CXXFLAGS += -std=c++0x

include(../../common/common_qt/mex_common.pri)

# message("!!! root_no_gui_common.pri: ROOT_FLAGS=$$MYROOTCFLAGS")
#LIBS += $$system($$MYROOTSYS/bin/root-config --libs)
LIBS += -L../../../../sys/$$CODENAME/lib
#LIBS += -lformexdaq_browser
CONFIG += debug
QT -= core
QT -= gui

#INCLUDEPATH += /afs/ifh.de/@sys/products/root64/5.20.00/include
INCLUDEPATH += ../../../include
INCLUDEPATH += ../../../contrib/cpp-raft/include

SOURCES += \
    ../../../src/test/mex_rootreader_mex_test.cpp

HEADERS += \
    ../../../include/pitz/daq/callbackn.hpp \
    ../../../include/pitz/daq/base.hpp \
    ../../../include/pitz/daq/data/indexer.hpp \
    ../../../include/pitz/daq/data/entryinfo.hpp \
    ../../../include/pitz/daq/data/engine/frompipebase.hpp \
    ../../../include/pitz/daq/data/engine/base.hpp \
    ../../../include/pitz/daq/data/getter/base.hpp \
    ../../../include/pitz/daq/data/getter/noindexing.hpp \
    ../../../include/pitz/daq/data/getter/tobuffer.hpp \
    ../../../include/pitz/daq/data/getter/impl.tobuffer.hpp \
    ../../../include/pitz/daq/data/memory/base.hpp \
    ../../../src/tools/pitz_daq_data_engine_branchitemprivate.hpp \
    ../../../include/pitz/daq/data/getter/tobuffer.hpp \
    ../../../include/pitz/daq/data/engine/frompipe.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/thread_cpp11.impl.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/thread_cpp11.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/mutex_cpp11.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/common_defination.h \
    ../../../include/pitz/daq/data/getter/impl.tobuffer.hpp \
    ../../../include/pitz/daq/data/getter/frompipe.hpp \
    ../../../include/pitz/daq/data/getter/frompipe.hpp \
    ../../../include/pitz/daq/data/engine/bypipe.hpp \
    ../../../include/pitz/daq/data/engine/tmp.frompipebase.hpp \
    ../../../include/pitz/daq/data/engine/tmp.frompipe.hpp \
    ../../../src/tools/bin_for_mexdaq_browser_common.h \
    ../../../contrib/cpp-raft/include/common/impl.lists.hpp \
    ../../../contrib/cpp-raft/include/common/lists.hpp

