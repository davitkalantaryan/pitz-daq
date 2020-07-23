#
# file:			daq.pro
# created on:	2020 Jul 23
# created by:	D. Kalantaryan (davit.kalantaryan@desy.de)
#

TEMPLATE = subdirs
#CONFIG += ordered

SUBDIRS		+=	$${PWD}/../../prj/server/event_based_collector_qt/event_based_collector.pro
SUBDIRS		+=	$${PWD}/../../prj/server/udpmcastdaq_qt/udpmcastdaq.pro
SUBDIRS		+=	$${PWD}/../../prj/server/requestreplydaq_qt/requestreplydaq.pro
SUBDIRS		+=	$${PWD}/../../contrib/data_handling/prj/tools/daq_browser_bin_for_nonroot_qt/daq_browser_bin_for_nonroot.pro
SUBDIRS		+=	$${PWD}/../../contrib/data_handling/prj/tools/mexdaq_browser4_qt/mexdaq_browser4.pro



OTHER_FILES	+=	\
	$${PWD}/../../README.md									\
	$${PWD}/../../.gitignore
