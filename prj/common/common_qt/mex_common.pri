#
# File mex_common.pri
# File created : 19 Apr 2017
# Created by : Davit Kalantaryan (davit.kalantaryan@desy.de)
# This file can be used to produce Makefile for daqadcreceiver application
# for PITZ
#


TARGET_EXT = mexa64
#QMAKE_EXTENSION_SHLIB = mexa64
TEMPLATE = lib

message("mex_common.pro")
include(../../common/common_qt/sys_common.pri)

INCLUDEPATH += /opt/matlab/current/extern/include


equals(CODENAME,"Santiago"){
    INCLUDEPATH += /products/matlab/R2010a/extern/include
}else{
equals(CODENAME,"trusty"){
    INCLUDEPATH += /usr/local/MATLAB/R2016a/extern/include
}else{
    INCLUDEPATH += /opt/matlab/current/extern/include
}
} # equals(CODENAME,"Santiago"){

INCLUDEPATH += /afs/ifh.de/SL/6/x86_64/opt/matlab/R2016b/extern/include

#QMAKE_EXTRA_TARGETS += copy_mex_file
#copy_mex_file.commands = "cp "
#POST_TARGETDEPS += copy_mex_file

QT -= core
QT -= gui
