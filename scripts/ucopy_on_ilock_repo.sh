#!/bin/bash
#
### BEGIN INIT INFO
# Author:		Davit Kalantaryan
# Script-name:		prepare.sh
# Required-Stop:   $network $remote_fs $syslog
# Default-Start:   2 3 4 5
# Default-Stop:    1
# Short-Description: Start watchdog_server 
### END INIT INFO


DEST_DIR=`pwd`

scriptDir="$( cd "$(dirname "$0")" ; pwd -P )"
currentDir=`pwd`

repoBase=$scriptDir/..
sourceInpBase=${repoBase}/src
includeInpBase=${repoBase}/include
sourceOutBase=$DEST_DIR/src
includeOutBase=$DEST_DIR/include

cp /var/oe/devenv/user/kalantar/i4repository/i4software/common/libs/i4driver/driver/i4/hal/daq.h ${repoBase}/.

