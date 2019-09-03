#!/bin/bash

# some defination
RootExtWithVersion=so.5.28
buildDirectoryBaseName=build-mexdaq_browser2-Desktop_Qt_5_12_2_GCC_64bit-Debug

currentDirectory=`pwd`

#scriptDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )" # instead of this
scriptDirectoryBase=`dirname ${0}`
cd ${scriptDirectoryBase}
fileOrigin=`readlink ${0}`
if [ ! -z "$fileOrigin" ]; then
	relativeSourceDir=`dirname ${fileOrigin}`
	cd ${relativeSourceDir}
fi
scriptDirectory=`pwd`
cd ${currentDirectory}

lsbCode=`lsb_release -c | cut -f 2`

echo "scriptDirectory="${scriptDirectory}

# Another defination (assumption)
buildDirectoryPath=${scriptDirectory}/../../prj/tools2/${buildDirectoryBaseName}

cd ${buildDirectoryPath}

patchelf --remove-needed libroot_for_matlab_false.so.1		${buildDirectoryPath}/mexdaq_browser2.mexa64

cd ${currentDirectory}
