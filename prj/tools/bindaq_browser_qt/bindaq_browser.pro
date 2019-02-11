# File bindaq_browser.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ


include(../../common/common_qt/root_no_gui_common.pri)
include(../../common/common_qt/sys_common.pri)
message("bindaq_browser.pro")

CONFIG += debug
QT -= core
QT -= gui

DEFINES += only_socket_macroses_are_used

#INCLUDEPATH += /afs/ifh.de/@sys/products/root64/5.20.00/include
INCLUDEPATH += ../../../include
INCLUDEPATH += ../../../contrib/cpp-raft/include

SOURCES += \
    ../../../src/tools/main_bindaq_browser.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/thread_cpp11.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/mutex_cpp11.cpp \
    ../../../src/common/common_argument_parser.cpp \
    ../../../src/tools/pitz_daq_data_entryinfo.cpp \
    ../../../src/tools/pitz_daq_data_memory_base.cpp \
    ../../../src/tools/pitz_daq_data_engine_local.cpp \
    ../../../src/tools/pitz_daq_data_engine_base.cpp \
    ../../../src/tools/pitz_daq_data_getter_base.cpp \
    ../../../src/tools/pitz_daq_data_getter_topipe.cpp \
    ../../../src/pitz_daq_base.cpp \
    ../../../src/tools/pitz_daq_data_indexer.cpp \
    ../../../src/tools/pitz_daq_data_getter_withindexer.cpp \
    ../../../src/tools/pitz_daq_data_bypipe_base.cpp \
    ../../../src/tools/pitz_daq_data_getter_tosocketpipe.cpp \
    ../../../src/common/common_createnewprocess_unix.cpp

HEADERS += \
    ../../../contrib/cpp-raft/include/common/impl.common_fifofast.hpp \
    ../../../contrib/cpp-raft/include/common/common_fifofast.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/thread_cpp11.impl.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/thread_cpp11.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/mutex_cpp11.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/common_defination.h \
    ../../../contrib/cpp-raft/include/common/common_unnamedsemaphorelite.hpp \
    ../../../include/common/common_argument_parser.impl.hpp \
    ../../../include/common/common_argument_parser.hpp \
    ../../../src/tools/bin_for_mexdaq_browser_common.h \
    ../../../contrib/cpp-raft/include/common/impl.common_hashtbl.hpp \
    ../../../contrib/cpp-raft/include/common/common_hashtbl.hpp \
    ../../../include/pitz/daq/base.hpp \
    ../../../include/pitz/daq/data/engine/base.hpp \
    ../../../include/pitz/daq/data/engine/local.hpp \
    ../../../include/pitz/daq/data/memory/base.hpp \
    ../../../include/pitz/daq/data/entryinfo.hpp \
    ../../../include/pitz/daq/data/getter/topipe.hpp \
    ../../../include/pitz/daq/data/getter/base.hpp \
    ../../../src/tools/pitz_daq_data_engine_branchitemprivate.hpp \
    ../../../include/pitz/daq/data/indexer.hpp \
    ../../../include/pitz/daq/callbackn.hpp \
    ../../../contrib/cpp-raft/include/common/impl.listspecialandhashtbl.hpp \
    ../../../contrib/cpp-raft/include/common/listspecialandhashtbl.hpp \
    ../../../contrib/cpp-raft/include/common/impl.lists.hpp \
    ../../../contrib/cpp-raft/include/common/lists.hpp \
    ../../../include/pitz/daq/data/getter/withindexer.hpp \
    ../../../include/pitz/daq/data/bypipe/base.hpp \
    ../../../include/pitz/daq/data/getter/tosocketpipe.hpp
