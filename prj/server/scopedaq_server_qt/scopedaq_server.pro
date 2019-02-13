#MYDOOCS         =  /afs/ifh.de/group/pitz/doocs
#MYKALANTARW     =  $$MYDOOCS/develop/kalantar/programs/cpp/works/from_levon/levonh/@sys

#MAINLIBPATH     =  $$MYDOOCS/lib
#MAINLIBPATH     =  $$MYKALANTARW/lib

message("!!! scopedaq_server.pro:")
include(../../common/common_qt/doocs_server_common.pri)

# g++ -Wall -I/doocs/lib/include -fPIC -c *.cpp
# g++ -shared -W1 -o libDOOCS_Funcs.so *.o
#CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++0x
CONFIG += debug
# CONFIG += release
# TEMPLATE = lib
DEFINES += LINUX
DEFINES += __linux__
QT -= core
QT -= gui
DEFINES += ___QT___

#INCLUDEPATH += $$MAINLIBPATH/include
#INCLUDEPATH += $$MYDOOCS/lib/include
#INCLUDEPATH += $$MYKALANTARW/haditioninc
#INCLUDEPATH += /doocs/lib/include


#LIBS +=    -L$$MAINLIBPATH
#LIBS +=    -L$$MYKALANTARW/laditionlib
#rm LIBS +=    -L/usr/lib64
#LIBS +=    -ludpmulticast
#LIBS += -lmcastclass
#LIBS += -L/doocs/lib

LIBS +=  -lMCclass
LIBS +=    -lADCShm
LIBS +=    -lADCDma
LIBS +=    -lADCscope
HEADERS += \
    ../../../src/server/eq_scopedaq.h \
    ../../../src/server/adc_map_helper.h
SOURCES += \
    ../../../src/server/scopedaq_rpc_server.cc
