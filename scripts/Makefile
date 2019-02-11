
PROJECT_NAME = interlock_notifier_server
OUTPUT_NAME = interlock_notifier_server.bin
TARGET_NAME = $(PROJECT_NAME)
TARGET_DIR  = .
SOURCES_BASE_DIR = src

CPP = g++
LINK = g++
#CPP = arm-linux-gnueabihf-g++
#LINK = arm-linux-gnueabihf-g++


LIBS = -lnsl -lpthread -lrt
LFLAGS  = -O2

OBJECT_FILES_DIR = .objects/$(PROJECT_NAME)
TARGET_FILE_PATH = $(TARGET_DIR)/$(OUTPUT_NAME)


CPPPARAMS = -std=c++11 -O2 -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_BSD -D__USE_GNU -D__USE_UNIX98 -Iinclude
#ASPARAMS = --32
#LDPARAMS = -melf_i386

OBJECTS = \
	$(OBJECT_FILES_DIR)/main_interlock_notifier_server.o \
	$(OBJECT_FILES_DIR)/desy_interlocknotifier_server.o \
	$(OBJECT_FILES_DIR)/common_sockettcp.o \
	$(OBJECT_FILES_DIR)/common_socketbase.o \
	$(OBJECT_FILES_DIR)/common_servertcp.o \
	$(OBJECT_FILES_DIR)/common_iodevice.o \
	$(OBJECT_FILES_DIR)/shared_mutex_cpp14.o \
	$(OBJECT_FILES_DIR)/thread_cpp11.o \
	$(OBJECT_FILES_DIR)/mutex_cpp11.o \
	$(OBJECT_FILES_DIR)/common_argument_parser.o

default: $(TARGET_NAME)


$(OBJECT_FILES_DIR)/%.o: $(SOURCES_BASE_DIR)/%.cpp
	mkdir -p $(@D)
	$(CPP) $(CPPPARAMS) -o $@ -c $<

$(OBJECT_FILES_DIR)/%.o: $(SOURCES_BASE_DIR)/%.s
	mkdir -p $(@D)
	as $(ASPARAMS) -o $@ $<

$(TARGET_NAME): $(OBJECTS)
	#mkdir -p $(TARGET_DIR)
	#ld $(LDPARAMS) -T $< -o $(TARGET_FILE_PATH) $(OBJECTS)
	$(LINK) $(OBJECTS) $(LIBS) $(LFLAGS) -o $(TARGET_FILE_PATH)
	rm -rf $(SRC_TMP)


# .PHONY means always out of date
.PHONY: clean cleanAll

clean:
	#rm -f $(OBJECTS) $(TARGET_FILE_PATH) $(SRC_TMP)/*.cpp $(SRC_TMP)/*.s $(ISO_DIR2)/mykernel.iso
	rm -f $(OBJECTS) $(TARGET_FILE_PATH) 

