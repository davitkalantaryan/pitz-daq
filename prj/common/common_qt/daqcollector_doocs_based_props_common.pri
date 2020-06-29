# File daqcollector_common.pri
# File created : 02 Feb 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ

message("!!! daqcollector_event_based_common.pri:")

#include($${PWD}/../../common/common_qt/daqcollector_common.pri)

SOURCES +=	\
	$${PWD}/../../../src/server/pitz_daq_singleentrydoocs_base.cpp

HEADERS +=	\
	$${PWD}/../../../src/server/pitz_daq_singleentrydoocs_base.hpp
