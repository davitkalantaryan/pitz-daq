# File laserbeamline_server.pro
# File created : 08 Feb 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
# CONFIG += TEST
# For making test: '$qmake "CONFIG+=TEST" daqadcreceiver.pro' , then '$make'

include(../../common/common_qt/doocs_client_common.pri)

greaterThan(QT_MAJOR_VERSION, 4):QT += widgets
CONFIG += c++11
INCLUDEPATH += ../../../include
INCLUDEPATH += ../../../contrib/cpp-raft/include

win32{
    CODENAME = win64
    SYSTEM_PATH = sys/win64
}else {
    QMAKE_CXXFLAGS += -std=c++0x
    QMAKE_CXXFLAGS_WARN_ON += -Wno-literal-suffix
}

SOURCES += \
    ../../../src/ui/pitz_daq_daqinterface_mainwindow.cpp \
    ../../../src/ui/pitz_daq_daqinterface_centralwidget.cpp \
    ../../../src/ui/pitz_daq_daqinterface_application.cpp \
    ../../../src/ui/main_daqinterface.cpp \
    ../../../src/ui/pitz_daq_daqinterface_collector.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/mutex_cpp11.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/shared_mutex_cpp14.cpp \
    ../../../contrib/cpp-raft/common/cpp11+/thread_cpp11.cpp \
    ../../../src/ui/pitz_daq_daqinterface_detailsbase.cpp

HEADERS += \
    ../../../src/ui/pitz_daq_daqinterface_mainwindow.hpp \
    ../../../src/ui/pitz_daq_daqinterface_centralwidget.hpp \
    ../../../src/ui/pitz_daq_daqinterface_application.hpp \
    ../../../contrib/cpp-raft/include/cpp11+/common_defination.h \
    ../../../src/ui/pitz_daq_daqinterface_collector.hpp \
    ../../../contrib/cpp-raft/include/common/impl.lists.hpp \
    ../../../contrib/cpp-raft/include/common/impl.listspecialandhashtbl.hpp \
    ../../../contrib/cpp-raft/include/common/listspecialandhashtbl.hpp \
    ../../../contrib/cpp-raft/include/common/lists.hpp \
    ../../../src/ui/impl.pitz_daq_daqinterface_application.hpp \
    ../../../src/ui/pitz_daq_daqinterface_detailsbase.hpp \
    ../../../include/common/mapandhash.hpp \
    ../../../include/common/impl.mapandhash.hpp \
    ../../../contrib/cpp-raft/include/common/common_hashtbl.hpp \
    ../../../contrib/cpp-raft/include/common/impl.common_hashtbl.hpp

RESOURCES += \
    ../../../src/resources/toolbar1.qrc

