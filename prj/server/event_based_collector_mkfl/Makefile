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

CPP = g++
LINK = g++
#CPP = arm-linux-gnueabihf-g++
#LINK = arm-linux-gnueabihf-g++

#LIBS = -ldl -lpcre 
#LIBS += -L/afs/ifh.de/amd64_rhel50/products/root64/5.28.00/lib64 -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -pthread -lm -ldl -rdynamic
LIBS += -L$(ROOTSYS)/lib -lCore -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -pthread -lm -ldl -rdynamic
LFLAGS  = -O2

OBJECT_FILES_DIR = $(mkfile_dir)/../../../sys/$(LSB_RELEASE)/.objects/$(PROJECT_NAME)
TARGET_FILE_PATH = $(TARGET_DIR)/$(OUTPUT_NAME)


#CPPPARAMS = -m32 -I../../../include -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings -DIs32Bit
CPPPARAMS = -DUSE_NEW_MATLB_NAMES -std=c++11 -O2 -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_BSD -D__USE_GNU -D__USE_UNIX98 -I$(mkfile_dir)/../../../include -I$(mkfile_dir)/../../../src/include_p
CPPPARAMS += -I$(mkfile_dir)/../../../contrib/matlab/include
CPPPARAMS += -I$(ROOTSYS)/include -I/afs/ifh.de/group/pitz/doocs/lib/include


OBJECTS = \
	$(OBJECT_FILES_DIR)/tools/main_daq_browser_bin_for_nonroot.o                 \
	$(OBJECT_FILES_DIR)/common/common_system_rw.o                                \
	$(OBJECT_FILES_DIR)/common/common_memorypool.o                               \
	$(OBJECT_FILES_DIR)/common/common_system_exe_child_windows.o                 \
	$(OBJECT_FILES_DIR)/common/common_argument_parser.o                          \
	$(OBJECT_FILES_DIR)/libs/pitz_daq_data_handling.o                            \
	$(OBJECT_FILES_DIR)/libs/pitz_daq_data_handling_types.o                      \
	$(OBJECT_FILES_DIR)/libs/pitz_daq_data_getter.o                              \
	$(OBJECT_FILES_DIR)/libs/common_libs_matlab_independent_functions.o          \
	$(OBJECT_FILES_DIR)/libs/pitz_daq_data_indexing.o

default: $(TARGET_NAME)

	
testVars:
	@echo "mkfile_dir="$(mkfile_dir)


$(OBJECT_FILES_DIR)/%.o: $(SOURCES_BASE_DIR)/%.cpp
	mkdir -p $(@D)
	$(CPP) $(CPPPARAMS) -o $@ -c $<


$(TARGET_NAME): $(OBJECTS)
	mkdir -p $(TARGET_DIR)
	#ld $(LDPARAMS) -T $< -o $(TARGET_FILE_PATH) $(OBJECTS)
	$(LINK) $(OBJECTS) $(LIBS) $(LFLAGS) -o $(TARGET_FILE_PATH)
	rm -rf $(SRC_TMP)

# .PHONY means always out of date
.PHONY: clean

clean:
	#rm -f $(OBJECTS) $(TARGET_FILE_PATH) $(SRC_TMP)/*.cpp $(SRC_TMP)/*.s $(ISO_DIR2)/mykernel.iso
	rm -f $(OBJECTS) $(TARGET_FILE_PATH) 

