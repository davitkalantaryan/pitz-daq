
QMAKE_CXXFLAGS += -std=c++0x

INCLUDEPATH += /doocs/include
LIBS += -L/doocs/lib -lDOOCSapi -lldap -lrt

SOURCES += \
    ../../../src/test/main_all_test.cpp
