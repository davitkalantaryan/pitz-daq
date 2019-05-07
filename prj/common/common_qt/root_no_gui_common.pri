#
# File root_no_gui_common.pri
# File created : 02 Feb 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
#

message("!!! root_no_gui_common.pri")

MYROOT_SYS_DIR = $$system(env | grep ROOT_SYS_DIR)

#message("!!! env:MYROOT_SYS_DIR : $$MYROOT_SYS_DIR")

#/opt/root/6.16.00

equals(MYROOT_SYS_DIR,"") {
    #MYROOT_SYS_DIR = /afs/ifh.de/amd64_rhel50/products/root64/5.28.00
    MYROOT_SYS_DIR = /afs/ifh.de/amd64_rhel50/products/root64/5.20.00
    message("!!! MYROOT_SYS_DIR set in the project file: $$MYROOT_SYS_DIR")
} else {
    MYROOT_SYS_DIR = $$(ROOT_SYS_DIR)
    message("!!! MYROOT_SYS_DIR comes from environment: $$MYROOT_SYS_DIR")
}


ROOTCFLAGS = $$system($$MYROOT_SYS_DIR/bin/root-config --cflags)

QMAKE_CXXFLAGS += $$ROOTCFLAGS
QMAKE_CFLAGS += $$ROOTCFLAGS
optionsCpp11 = $$find(CONFIG, "cpp11")
count(optionsCpp11, 1):QMAKE_CXXFLAGS += -std=c++0x
message("ROOTCFLAGS=$$ROOTCFLAGS")

LIBS += -L/doocs/develop/kalantar/programs/cpp/works/pitz-daq/sys/$$CODENAME/lib
LIBS += $$system($$MYROOT_SYS_DIR/bin/root-config --libs)
#LIBS += -ltbb

# this line is not needed for compilation but some
# IDE does not shows ROOT headers properly if this line is not there
INCLUDEPATH += $$MYROOT_SYS_DIR/include

# temporar
#INCLUDEPATH += /afs/ifh.de/amd64_rhel50/products/root64/5.20.00/include
