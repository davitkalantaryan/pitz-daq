# File daqadcreceiver.pro
# File created : 01 Jan 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ


#DEFINES += DEBUG_APP

DEFINES += DMSG
DEFINES += NEW_GETTER_THREAD
#DEFINES += CPP11_THREADS_IMPLEMENT_HERE
DEFINES += USE_DOOCS


include($${PWD}/../../common/common_qt/daqcollector_doocs_based_props_common.pri)
include($${PWD}/../../common/common_qt/daqcollector_event_based_common.pri)

INCLUDEPATH += $$MYDOOCS/include/zmq
LIBS += -lzmq
#LIBS += -L/afs/ifh.de/group/pitz/doocs/data/ers/sys/$$CODENAME/lib
# LIBS += -lMCclass

SOURCES +=	\
	$${PWD}/../../../src/server/pitz_daq_eqfcteventbased.cpp


HEADERS +=	\
	$${PWD}/../../../src/server/pitz_daq_eqfcteventbased.cpp.hpp		\
	$${PWD}/../../../src/server/pitz_daq_eqfcteventbased.hpp

OTHER_FILES +=	\
	$${PWD}/../../../src/server/pitz_daq_eqfctudpmcast.cpp			\
	$${PWD}/../../../src/server/pitz_daq_eqfctrr.cpp			\
	$${PWD}/../../../src/examples/zmqget.cc					\
	$${PWD}/../event_based_collector_mkfl/Makefile		\
	$${PWD}/../../../README.md
