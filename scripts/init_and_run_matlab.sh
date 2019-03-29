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

rootBaseDir=/opt/root/6.16.00

scriptDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "scriptDirectory=""${scriptDirectory}"

lsbReleaseCode=`lsb_release -c | cut -f 2`
echo "lsbReleaseCode="$lsbReleaseCode

#currentDirectory=`pwd`
#echo "currentDirectory=""${currentDirectory}"


. ${rootBaseDir}/bin/thisroot.sh
export MATLABPATH=${scriptDirectory}/../src/matlab:${scriptDirectory}/../../sys/$lsbReleaseCode/mbin:${MATLABPATH}
export PATH=${scriptDirectory}../../sys/$lsbReleaseCode/bin:${PATH}
export LD_LIBRARY_PAT=${scriptDirectory}/../../sys/$lsbReleaseCode/lib:${scriptDirectory}/../../sys/$lsbReleaseCode/dll:${LD_LIBRARY_PAT}

matlab_r2018b
