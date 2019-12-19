
QMAKE_LFLAGS = -nostdlib
DEFINES += DO_NOT_USE_LIBC

CONFIG -= qt

SOURCES += \
    $${PWD}/../../../src/test/main_nolibc_h_test.c


HEADERS += \
    $${PWD}/../../../src/test/nolibc.h
