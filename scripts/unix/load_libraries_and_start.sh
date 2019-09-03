#!/bin/bash

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 arguments should be at least one"
  exit 1
fi

# some defination
RootExtWithVersion=so.5.28

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


LD_PRELOAD=$LD_PRELOAD:libCore.${RootExtWithVersion}	
LD_PRELOAD=$LD_PRELOAD:libCint.${RootExtWithVersion}	
LD_PRELOAD=$LD_PRELOAD:libRIO.${RootExtWithVersion}		
LD_PRELOAD=$LD_PRELOAD:libNet.${RootExtWithVersion}		
LD_PRELOAD=$LD_PRELOAD:libHist.${RootExtWithVersion}	
LD_PRELOAD=$LD_PRELOAD:libGraf.${RootExtWithVersion}	
LD_PRELOAD=$LD_PRELOAD:libGraf3d.${RootExtWithVersion}	
LD_PRELOAD=$LD_PRELOAD:libGpad.${RootExtWithVersion}	
LD_PRELOAD=$LD_PRELOAD:libTree.${RootExtWithVersion}	
LD_PRELOAD=$LD_PRELOAD:libRint.${RootExtWithVersion}	
LD_PRELOAD=$LD_PRELOAD:libPostscript.${RootExtWithVersion}
LD_PRELOAD=$LD_PRELOAD:libMatrix.${RootExtWithVersion}	
LD_PRELOAD=$LD_PRELOAD:libPhysics.${RootExtWithVersion}	
LD_PRELOAD=$LD_PRELOAD:libMathCore.${RootExtWithVersion}
LD_PRELOAD=$LD_PRELOAD:libThread.${RootExtWithVersion}	

export LD_PRELOAD

cd ${currentDirectory}

programToRun="${1}"

shift

${programToRun} "$@"
