MYZMQ     =  /afs/ifh.de/group/pitz/doocs/zmq

LIBS += -L$$MYZMQ/lib -lzmq
INCLUDEPATH += $$MYZMQ/include
INCLUDEPATH += ../../../include

HEADERS += \
    ../../../include/common/zmqsubscriber.hpp
SOURCES += ../../../src/tools/zmqsubscriber.cpp \
    ../../../src/tools/main_zmqdaqreceiver.cpp
