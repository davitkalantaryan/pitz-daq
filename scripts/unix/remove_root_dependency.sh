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

patchelf --remove-needed libCore.${RootExtWithVersion}		${buildDirectoryPath}/mexdaq_browser2.mexa64
patchelf --remove-needed libCint.${RootExtWithVersion}		${buildDirectoryPath}/mexdaq_browser2.mexa64
patchelf --remove-needed libRIO.${RootExtWithVersion}		${buildDirectoryPath}/mexdaq_browser2.mexa64
patchelf --remove-needed libNet.${RootExtWithVersion}		${buildDirectoryPath}/mexdaq_browser2.mexa64
patchelf --remove-needed libHist.${RootExtWithVersion}		${buildDirectoryPath}/mexdaq_browser2.mexa64
patchelf --remove-needed libGraf.${RootExtWithVersion}		${buildDirectoryPath}/mexdaq_browser2.mexa64
patchelf --remove-needed libGraf3d.${RootExtWithVersion}	${buildDirectoryPath}/mexdaq_browser2.mexa64
patchelf --remove-needed libGpad.${RootExtWithVersion}		${buildDirectoryPath}/mexdaq_browser2.mexa64
patchelf --remove-needed libTree.${RootExtWithVersion}		${buildDirectoryPath}/mexdaq_browser2.mexa64
patchelf --remove-needed libRint.${RootExtWithVersion}		${buildDirectoryPath}/mexdaq_browser2.mexa64
patchelf --remove-needed libPostscript.${RootExtWithVersion}	${buildDirectoryPath}/mexdaq_browser2.mexa64
patchelf --remove-needed libMatrix.${RootExtWithVersion}	${buildDirectoryPath}/mexdaq_browser2.mexa64
patchelf --remove-needed libPhysics.${RootExtWithVersion}	${buildDirectoryPath}/mexdaq_browser2.mexa64
patchelf --remove-needed libMathCore.${RootExtWithVersion}	${buildDirectoryPath}/mexdaq_browser2.mexa64
patchelf --remove-needed libThread.${RootExtWithVersion}	${buildDirectoryPath}/mexdaq_browser2.mexa64

cd ${currentDirectory}
