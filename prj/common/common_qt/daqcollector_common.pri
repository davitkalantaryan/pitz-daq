# File daqcollector_common.pri
# File created : 02 Feb 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
message("!!! daqcollector_common.pri:")
include(../../common/common_qt/root_no_gui_common.pri)
DEFINES += ROOT_APP

# call qmake CONFIG+=test
optionsTest = $$find(CONFIG, "1test")
count(optionsTest, 1):message("!!! test1 version") DEFINES += TEST_VERSION111
options = $$find(CONFIG, "2test")
count(options, 1):message("!!! test2 version") DEFINES += TEST_VERSION112
include(../../common/common_qt/doocs_server_common.pri)
equals(CODENAME,"Boron") { 
    message ("!!!!! No cpp 11 used")
    DEFINES += no_cpp11
    QMAKE_CXXFLAGS += -std=c++0x
}
else { 
    message ("!!!!! cpp 11 is used")
    QMAKE_CXXFLAGS += -std=c++0x
}
INCLUDEPATH += ../../../include
INCLUDEPATH += ../../../contrib/cpp-raft/include
INCLUDEPATH += ../../../src/tools

# these two lines are just for inteligence
#INCLUDEPATH += /afs/ifh.de/@sys/products/root64/5.20.00/include
INCLUDEPATH += /doocs/lib/include
SOURCES += \
    ../../../src/server/pitz_daq_collectorproperties.cpp \
    ../../../src/tools/mailsender.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/thread_cpp11.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/mutex_cpp11.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/shared_mutex_cpp14.cpp \
    ../../../src/tools/pitz_daq_data_memory_base.cpp \
    ../../../src/tools/pitz_daq_data_memory_forserver.cpp \
    ../../../src/server/pitz_daq_singleentry.cpp \
    ../../../src/server/pitz_daq_eqfctcollector.cpp \
    ../../../src/server/pitz_daq_collector_global.cpp \
    ../../../src/tools/pitz_daq_data_entryinfo.cpp
HEADERS += \
    ../../../src/server/pitz_daq_collectorproperties.hpp \
    ../../../src/tools/mailsender.h \
    ../../../contrib/cpp-raft/include/cpp11+/thread_cpp11.impl.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/thread_cpp11.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/common_defination.h \
    ../../../contrib/cpp-raft/include/cpp11+/mutex_cpp11.hpp \
    ../../../contrib/cpp-raft/include/common/common_unnamedsemaphorelite.hpp \
    ../../../contrib/cpp-raft/include/common/impl.common_fifofast.hpp \
    ../../../contrib/cpp-raft/include/common/common_fifofast.hpp \
    ../../../contrib/cpp-raft/include/common/lists.hpp \
    ../../../contrib/cpp-raft/include/common/impl.lists.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/shared_mutex_cpp14.hpp \
    ../../../include/pitz_daq_memory.hpp \
    ../../../src/server/pitz_daq_singleentry.hpp \
    ../../../src/server/pitz_daq_eqfctcollector.hpp \
    ../../../include/pitz/daq/data/memory/base.hpp \
    ../../../include/pitz/daq/data/memory/forserver.hpp
