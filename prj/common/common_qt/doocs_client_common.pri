#
# File doocs_client_common.pri
# File created : 12 Feb 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
#


LIBS += -L/afs/ifh.de/group/pitz/doocs/data/ers/sys/$$CODENAME/lib

MYDOOCS = /afs/ifh.de/group/pitz/doocs

INCLUDEPATH += $$MYDOOCS/include

message("!!! doocs_client_common.pri:")

QMAKE_CXXFLAGS_WARN_ON += -Wno-attributes

include(../../common/common_qt/sys_common.pri)

SYSTEM_LIB = $$MYDOOCS/system_arch/$$CODENAME/lib

# message ("!!!!! No cpp 11 used") # todo: calculate in the sys_common.pri
# DOOCS always requires cpp 14
#QMAKE_CXXFLAGS += -std=c++0x
CONFIG += c++14

equals(CODENAME,"Boron") {
    INCLUDEPATH += $$SYSTEM_LIB/include

}
else {

    equals(CODENAME,"trusty") {
        INCLUDEPATH += $$SYSTEM_LIB/include
    }
    else{
        INCLUDEPATH += $$MYDOOCS/include/doocs
        #INCLUDEPATH += /doocs/system_arch/trusty/lib/include
    }
}


message("!!! SYSTEM_LIB: $$SYSTEM_LIB")

DEFINES += LINUX

#LIBS += -L/doocs/lib
LIBS += -L$$SYSTEM_LIB
#LIBS += -L/doocs/develop/kalantar/programs/cpp/works/sys/$$CODENAME/lib
#LIBS += -L/doocs/develop/bagrat/doocs.git/amd64_rhel60/lib
LIBS += -lDOOCSapi
LIBS += -lgul14
LIBS += -lldap
LIBS += -lrt

include(../../common/common_qt/sys_common.pri)

#INCLUDEPATH += /doocs/include/doocs
#INCLUDEPATH += $$SYSTEM_LIB/include/doocs
#INCLUDEPATH += /doocs/develop/bagrat/doocs.git/include

#INCLUDEPATH += /afs/ifh.de/group/pitz/doocs/lib/include
INCLUDEPATH += /afs/ifh.de/group/pitz/doocs/include/doocs
