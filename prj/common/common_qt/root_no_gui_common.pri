#
# File root_no_gui_common.pri
# File created : 02 Feb 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
#


#MYROOTSYS = /afs/ifh.de/group/pitz/doocs/amd64_rhel50/root/6.02.00
MYROOTSYS = /afs/ifh.de/amd64_rhel50/products/root64/5.20.00
MYROOTCFLAGS = `$$MYROOTSYS/bin/root-config \
    --cflags`
QMAKE_CXXFLAGS += $$MYROOTCFLAGS
QMAKE_CFLAGS += $$MYROOTCFLAGS
optionsCpp11 = $$find(CONFIG, "cpp11")
count(optionsCpp11, 1):QMAKE_CXXFLAGS += -std=c++0x
message("!!! root_no_gui_common.pri: ROOT_FLAGS=$$MYROOTCFLAGS")

LIBS += -L/doocs/develop/kalantar/programs/cpp/works/pitz-daq/sys/$$CODENAME/lib
LIBS += $$system($$MYROOTSYS/bin/root-config --libs)

# this line is not needed for compilation but some
# IDE does not shows ROOT headers properly if this line is not there
INCLUDEPATH += $$MYROOTSYS/include

# temporar
INCLUDEPATH += /afs/ifh.de/amd64_rhel50/products/root64/5.20.00/include
