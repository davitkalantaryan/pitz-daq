# File sys_common.pri
# File created : 12 Feb 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
#

#QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter
#QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-variable
#QMAKE_CXXFLAGS_WARN_ON += -Wno-sign-compare
#QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-function
#QMAKE_CXXFLAGS_WARN_ON -= -Wunused-function

DEEPNESS = ../../..

optionsLib = $$find(TEMPLATE, "lib")

count(optionsLib, 1){
    TARGET_PATH=lib
    DEEPNESS = ../../../../
    message("Shared library creation")
}else{
    TARGET_PATH=bin
    DEEPNESS = ../../../../
    message("Binary file creation")
}


win32 {
    contains(QMAKE_TARGET.arch, x86_64) {
        ## Windows x64 (64bit) specific build here
        CODENAME = win64
        SYSTEM_PATH = sys/win64

    } else {
        ## Windows x86 (32bit) specific build here
        CODENAME = win32
        SYSTEM_PATH = sys/win32

    }

} else:mac {
    CODENAME = mac
    SYSTEM_PATH = sys/mac
} else:android {
    CODENAME = android
    SYSTEM_PATH = sys/android
} else {
    DEFINES += LINUX
    CODENAME = $$system(lsb_release -c | cut -f 2)
    SYSTEM_PATH = sys/$$CODENAME
}


message("!!! sys_common.pri: SYSTEM_PATH=$$SYSTEM_PATH")

# Debug:DESTDIR = debug1
DESTDIR = $$DEEPNESS/$$SYSTEM_PATH/$$TARGET_PATH
OBJECTS_DIR = ../../../$$SYSTEM_PATH/.objects

#CONFIG += debug
#CONFIG += c++11
#QMAKE_CXXFLAGS += -std=c++0x
# greaterThan(QT_MAJOR_VERSION, 4):QT += widgets
#QT -= core
#QT -= gui
