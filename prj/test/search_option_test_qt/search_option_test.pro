#
# File search_option_test.pro
# File created : 02 Feb 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
# CONFIG += TEST
# For making test: '$qmake "CONFIG+=TEST" daqadcreceiver.pro' , then '$make'
#

options = $$find(CONFIG, "TEST")
count(options, 1) { 
    DEFINES += TEST_SIMPLE_APP

    # SOURCES += ../../../src/dgui/main_gui_wallet.cpp
    message("!!!!!! Generating test setup")
}

# 'count(options, 1)'
else { 
    # DECENT_ROOT_DEV = $$(DECENT_ROOT)
    # equals(DECENT_ROOT_DEV, ""): DECENT_ROOT_DEV = $$DECENT_ROOT_DEFAULT
    # DECENT_LIB = $$DECENT_ROOT_DEV/libraries

    options2 = $$find(CONFIG, "BAGRAT_LIBS_TST")
    count(options2, 1) {

        message("!!!!!! Generating new DOOCS lib test setup")
        LIBS += -L/doocs/develop/bagrat/doocs.git/amd64_rhel60/lib
        LIBS += -L/doocs/develop/kalantar/programs/cpp/works/sysafs/amd64_rhel60/lib
        INCLUDEPATH += /doocs/develop/bagrat/doocs.git/include
        INCLUDEPATH += /doocs/develop/kalantar/programs/cpp/works/sysafs/amd64_rhel60/lib/include

    }else{

        LIBS += -L/doocs/lib
        INCLUDEPATH += /doocs/lib/include

    }

    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-variable
    QMAKE_CXXFLAGS_WARN_ON += -Wno-sign-compare
    QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-function
    QMAKE_CXXFLAGS_WARN_ON -= -Wunused-function
    win32:SYSTEM_PATH = ../../../sys/win64
    else { 
        macx:SYSTEM_PATH = ../../../sys/mac
        else { 
            CODENAME = $$system(lsb_release -c | cut -f 2)
            SYSTEM_PATH = ../../../sys/$$CODENAME
        }
    }
    
    # Debug:DESTDIR = debug1
    DESTDIR = $$SYSTEM_PATH/bin
    OBJECTS_DIR = $$SYSTEM_PATH/.objects
    CONFIG += debug
    
    CONFIG += c++11
    # greaterThan(QT_MAJOR_VERSION, 4):QT += widgets
    QT -= core
    QT -= gui
    
    # QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
    # QMAKE_CXXFLAGS += -msse4.2
    # QMAKE_CFLAGS += -msse4.2
    # exists( $$BOOST_ROOT_QT/lib/libboost_thread-mt* ): bla bla
    MYROOTSYS = /afs/ifh.de/@sys/products/root64/5.20.00
    MYROOTCFLAGS = `$$MYROOTSYS/bin/root-config \
        --cflags`
    QMAKE_CXXFLAGS += $$MYROOTCFLAGS
    QMAKE_CFLAGS += $$MYROOTCFLAGS
    CONFIG += debug
    DEFINES += LINUX
    QT -= core
    QT -= gui
    
    # INCLUDEPATH += /doocs/develop/common/include
    # INCLUDEPATH += $$LIBBASE/lib/include
    #INCLUDEPATH += $$DOOCS_LIB_INCLUDE
    
    # INCLUDEPATH += /doocs/develop/kalantar/doocs.git/doocs/include-client
    # INCLUDEPATH += /doocs/develop/kalantar/doocs.git/doocs/include-server
    # INCLUDEPATH += $$MYDOOCS/develop/common/include
    # INCLUDEPATH += $$MYKALANTARW/haditioninc
    # Sa ka, vorpeszi includner@ hesht gtnvi
    INCLUDEPATH += /afs/ifh.de/@sys/products/root64/5.20.00/include
    INCLUDEPATH += ../../../include
    INCLUDEPATH += ../../../src/tools
    DEFINES += ___QT___
    
    # LIBS += -L$$LIBBASE/lib
    #LIBS += -L$$DOOCS_LIB
    
    # LIBS += -lsmall_plotter_lib
    LIBS += -lMCclass
    LIBS += -lEqServer
    LIBS += -lDOOCSapi
    LIBS += -lldap
    LIBS += -lrt
    LIBS += `$$MYROOTSYS/bin/root-config \
        --libs`
    LIBS += -L/doocs/lib
    LIBS += -lMCclass
    LIBS += -lEqServer
    LIBS += -lDOOCSapi
    LIBS += -lldap
    LIBS += -lrt
    LIBS += -L/afs/ifh.de/@sys/products/root64/5.20.00/lib64
    LIBS += -lCore
    LIBS += -lCint
    LIBS += -lRIO
    LIBS += -lNet
    LIBS += -lHist
    LIBS += -lGraf
    LIBS += -lGraf3d
    LIBS += -lGpad
    LIBS += -lTree
    LIBS += -lRint
    LIBS += -lPostscript
    LIBS += -lMatrix
    LIBS += -lPhysics
    LIBS += -lMathCore
    LIBS += -lThread
    LIBS += -pthread
    LIBS += -lm
    LIBS += -ldl
    LIBS += -rdynamic
    HEADERS += ../../../src/tools/smallmutex.h \
        ../../../src/tools/lifostack.h \
        ../../../src/tools/fifofast.h \
        ../../../src/server/daqadcreceiver.hpp \
        ../../../include/alog.h \
        ../../../src/server/adcdaqcommonheader.h
    SOURCES += ../../../src/server/daqadcreceiver.cpp \
        ../../../src/utils/unnamedsemaphore.cpp \
        ../../../src/utils/alog.cpp
}
SOURCES += ../../../src/server/pitz_daq_collectorbase.cpp \
    ../../../src/server/pitz_daq_collectorinfo.cpp \
    ../../../src/server/pitz_daq_globalfunctionsiniter.cpp
HEADERS += ../../../src/server/pitz_daq_collectorbase.hpp \
    ../../../src/server/pitz_daq_collectorinfo.hpp \
    ../../../src/server/pitz_daq_globalfunctionsiniter.hpp \
    ../../../include/common_daq_definations.h
