#!/bin/bash
#
# File:			ucreate_docu.sh
# Created on:		2019 Feb 15
# Created by:		Davit Kalantaryan
#
# Purpose:	
#					This script calls the doxygen to generate docu file based on the configuration
#					file 'doxy1.txt'. The doxygen configuration file assumed to be in the same 
#					directory with the script
#
# Argumet list:
#					1. directory name for generating documentation 
#					  (optional, if not provided, then '..\..\docs\doxy' relative dir is used)
#


lsbCodeName=`lsb_release -c | cut -f 2`
scriptDirectory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
currentDirectory=`pwd`


outputDir=$scriptDirectory/../docs/doxy

if [ $# -gt 0 ]; then
	outputDir="$1"
fi
echo "outputDir=${outputDir}"

mkdir -p ${outputDir}

export DOXYGEN_NAME="laserbeamline-API"
export DOXYGEN_ROOT_DIR=${scriptDirectory}/..
export DOXYGEN_OUT_DIR=${outputDir}

echo "AGENT_ROOT_DIR=${AGENT_ROOT_DIR}"
doxygen ${scriptDirectory}/doxy1.txt

cd ${currentDirectory}
