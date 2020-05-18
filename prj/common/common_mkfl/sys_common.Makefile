#
# file:		Makefile (Makefile to create event_based_collector)
#
ROOTSYS			= /afs/ifh.de/group/pitz/doocs/amd64_rhel60/root/6.02.00
PROJECT_NAME		= event_based_collector
OUTPUT_NAME		= event_based_collector
TARGET_NAME		= $(PROJECT_NAME)
LSB_RELEASE		:= $(shell lsb_release -c | cut -f 2)
mkfile_path		:= $(abspath $(lastword $(MAKEFILE_LIST)))
#mkfile_dir		:= $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))
mkfile_dir		:= $(shell dirname $(mkfile_path))
TARGET_DIR		= $(mkfile_dir)/../../../sys/$(LSB_RELEASE)/bin
SOURCES_BASE_DIR	= $(mkfile_dir)/../../../src
