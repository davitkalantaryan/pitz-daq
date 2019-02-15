::
:: File:			wcreate_docu.bat
:: Created on:		2019 Feb 15
:: Created by:		Davit Kalantaryan
::
:: Purpose:	
::					This script calls the doxygen to generate docu file based on the configuration
::					file 'doxy1.txt'. The doxygen configuration file assumed to be in the same 
::					directory with the script
::
:: Argumet list:
::					1. directory name for generating documentation 
::					  (optional, if not provided, then '..\..\docs\doxy' relative dir is used)
::

@ECHO off

SETLOCAL enableextensions


:: calculating script and current directory
SET  scriptDirectory=%~dp0
set  currentDirectory=%cd%

::cd %scriptDirectory%

set argC=0
for %%x in (%*) do Set /A argC+=1
echo %argC%

set outputDir=%scriptDirectory%..\docs\doxy

IF /I "%argC%" NEQ "0" ( set outputDir=%1 )
echo "outputDir=%outputDir%"

if not exist %outputDir% ( mkdir %outputDir% )

set DOXYGEN_NAME=PITZ DAQ API
set DOXYGEN_ROOT_DIR=%scriptDirectory%..
set DOXYGEN_OUT_DIR=%outputDir%

echo "AGENT_ROOT_DIR=%AGENT_ROOT_DIR%"
call doxygen %scriptDirectory%doxy1.txt


cd %currentDirectory%
ENDLOCAL
