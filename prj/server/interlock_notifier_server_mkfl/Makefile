
PROJECT_NAME = interlock_notifier_server
OUTPUT_NAME = interlock_notifier_server.bin
TARGET_NAME = $(PROJECT_NAME)
TARGET_DIR  = .
ISO_DIR2 = $(TARGET_DIR)/iso
#SOURCES_BASE_DIR = ../../../src
SOURCES_BASE_DIR = ../../..

CPP = g++
LINK = g++
#CPP = arm-linux-gnueabihf-g++
#LINK = arm-linux-gnueabihf-g++


LIBS = -lnsl -lpthread -lrt
LFLAGS  = -O2

OBJECT_FILES_DIR = ../../../sys/sos/.objects/$(PROJECT_NAME)
ISO_TMP = tmp.iso
ISO_BOOT = $(ISO_TMP)/boot
ISO_GRUB = $(ISO_TMP)/boot/grub
TARGET_FILE_PATH = $(TARGET_DIR)/$(OUTPUT_NAME)


#CPPPARAMS = -m32 -I../../../include -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings -DIs32Bit
CPPPARAMS = -std=c++11 -O2 -DLINUX -D_REENTRANT -D_GNU_SOURCE -D_BSD -D__USE_GNU -D__USE_UNIX98 -I../../../include -I../../../contrib/cpp-raft/include
ASPARAMS = --32
LDPARAMS = -melf_i386

OBJECTS = \
	$(OBJECT_FILES_DIR)/src/server/main_interlock_notifier_server.o \
	$(OBJECT_FILES_DIR)/src/server/desy_interlocknotifier_server.o \
	$(OBJECT_FILES_DIR)/contrib/cpp-raft/common/common_sockettcp.o \
	$(OBJECT_FILES_DIR)/contrib/cpp-raft/common/common_socketbase.o \
	$(OBJECT_FILES_DIR)/contrib/cpp-raft/common/common_servertcp.o \
	$(OBJECT_FILES_DIR)/contrib/cpp-raft/common/common_iodevice.o \
	$(OBJECT_FILES_DIR)/contrib/cpp-raft/common/cpp11+/shared_mutex_cpp14.o \
	$(OBJECT_FILES_DIR)/contrib/cpp-raft/common/cpp11+/thread_cpp11.o \
	$(OBJECT_FILES_DIR)/contrib/cpp-raft/common/cpp11+/mutex_cpp11.o \
	$(OBJECT_FILES_DIR)/src/common/common_argument_parser.o

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

install: $(TARGET_NAME)
	sudo cp $(TARGET_FILE_PATH) /boot/.

mykernel.iso: $(TARGET_NAME)
	mkdir -p $(ISO_DIR2)
	mkdir -p $(ISO_GRUB)
	cp $(TARGET_FILE_PATH) $(ISO_BOOT)/.
	echo '' > $(ISO_GRUB)/grub.cfg
	echo 'set timeout=0' >>  $(ISO_GRUB)/grub.cfg
	echo 'set default=0' >> $(ISO_GRUB)/grub.cfg
	echo '' >> $(ISO_GRUB)/grub.cfg
	echo "menuentry 'sosystem' {" >> $(ISO_GRUB)/grub.cfg
	echo '	multiboot /boot/armos.bin' >> $(ISO_GRUB)/grub.cfg
	echo '	boot' >> $(ISO_GRUB)/grub.cfg
	echo '}' >> $(ISO_GRUB)/grub.cfg
	echo '' >> $(ISO_GRUB)/grub.cfg
	grub-mkrescue --output=$(ISO_DIR2)/$@ $(ISO_TMP)
	rm -r $(ISO_TMP)

run: mykernel.iso
	(killall VirtualBox && sleep 1) || true
	virtualbox --startvm soskernel&

# .PHONY means always out of date
.PHONY: clean cleanAll

clean:
	#rm -f $(OBJECTS) $(TARGET_FILE_PATH) $(SRC_TMP)/*.cpp $(SRC_TMP)/*.s $(ISO_DIR2)/mykernel.iso
	rm -f $(OBJECTS) $(TARGET_FILE_PATH) 

