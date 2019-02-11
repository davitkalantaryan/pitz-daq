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

BASEDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "BASEDIR=""$BASEDIR"

CUR_DIR=`pwd`
echo "CUR_DIR=""$CUR_DIR"


if [ "$#" == 0 ]; then
	TODO_FNC=BUILD
	OPTION_IN=all
elif [ "$#" == 1 ]; then
	if [[ "$1" == "clean" ]]; then
		TODO_FNC=CLEAN
		OPTION_IN=all
	else
		TODO_FNC=BUILD
		OPTION_IN=$1
	fi
else
	if [[ "$2" == "clean" ]]; then
		TODO_FNC=CLEAN
	fi
	OPTION_IN=$1
fi

echo "TODO_FNC="$TODO_FNC
echo "OPTION_IN="$OPTION_IN

function daqinterface {
	echo daqinterface
	cd $BASEDIR/prj/daqinterface/daqinterface_qt
	qmake daqinterface.pro
	if [[ "$TODO_FNC" == "BUILD" ]]; then
		make
	else
		make clean
	fi
	cd $CUR_DIR
	echo "daqinterface done!"
}


function build_daqadcreceiver {
	echo daqadcreceiver
	cd $BASEDIR/prj/daqadcreceiver/daqadcreceiver_qt
	qmake daqadcreceiver.pro
	if [[ "$TODO_FNC" == "BUILD" ]]; then
		make
	else
		make clean
	fi
	cd $CUR_DIR
	echo "daqadcreceiver done!"
}
function daqadcreceiver {
	FUNCTION_NAME=daqadcreceiver	
	if [[ "$OSTYPE" == "linux-gnu" ]]; then
		LINUX_CODE=`lsb_release -c | cut -f 2`
		if [[ "$LINUX_CODE" == "Santiago" ]]; then
			build_$FUNCTION_NAME
		elif [[ "$LINUX_CODE" == "trusty" ]]; then
			echo "!!!!!! "$FUNCTION_NAME" will not be compiled on ubuntu14"
		else
			echo "!!!!!! Unknown LINUX!"
		fi
	elif [[ "$OSTYPE" == "darwin"* ]]; then
		echo "!!!!!! "$FUNCTION_NAME" will not be compiled on mac"
	elif [[ "$OSTYPE" == "cygwin" ]]; then
		echo "!!!!!! "$FUNCTION_NAME" will not be compiled for cygwin (POSIX compatibility layer and Linux environment emulation for Windows)"
	elif [[ "$OSTYPE" == "msys" ]]; then
		echo "!!!!!! "$FUNCTION_NAME" will not be compiled for msys (Lightweight shell and GNU utilities compiled for Windows (part of MinGW))"
        elif [[ "$OSTYPE" == "win32" ]]; then
		echo "!!!!!! "$FUNCTION_NAME" will not be compiled on 32 bit WINDOWS"
	elif [[ "$OSTYPE" == "freebsd"* ]]; then
		echo "!!!!!! "$FUNCTION_NAME" will not be compiled on freebsd"
	else
        	echo "!!!!!!  ERROR: Unknown OS"
	fi
}


function build_librootreader {
	echo librootreader
	cd $BASEDIR/prj/rootreader/librootreader_qt
	qmake librootreader.pro
	if [[ "$TODO_FNC" == "BUILD" ]]; then
		make
	else
		make clean
	fi
	cd $CUR_DIR
	echo "rootreader done!"
}
function librootreader {
	FUNCTION_NAME=librootreader	
	if [[ "$OSTYPE" == "linux-gnu" ]]; then
		LINUX_CODE=`lsb_release -c | cut -f 2`
		if [[ "$LINUX_CODE" == "Santiago" ]]; then
			build_$FUNCTION_NAME
		elif [[ "$LINUX_CODE" == "trusty" ]]; then
			echo "!!!!!! "$FUNCTION_NAME" will not be compiled on ubuntu14"
		else
			echo "!!!!!! Unknown LINUX!"
		fi
	elif [[ "$OSTYPE" == "darwin"* ]]; then
		echo "!!!!!! "$FUNCTION_NAME" will not be compiled on mac"
	elif [[ "$OSTYPE" == "cygwin" ]]; then
		echo "!!!!!! "$FUNCTION_NAME" will not be compiled for cygwin (POSIX compatibility layer and Linux environment emulation for Windows)"
	elif [[ "$OSTYPE" == "msys" ]]; then
		echo "!!!!!! "$FUNCTION_NAME" will not be compiled for msys (Lightweight shell and GNU utilities compiled for Windows (part of MinGW))"
        elif [[ "$OSTYPE" == "win32" ]]; then
		echo "!!!!!! "$FUNCTION_NAME" will not be compiled on 32 bit WINDOWS"
	elif [[ "$OSTYPE" == "freebsd"* ]]; then
		echo "!!!!!! "$FUNCTION_NAME" will not be compiled on freebsd"
	else
        	echo "!!!!!!  ERROR: Unknown OS"
	fi
}


function build_rootreader {
	echo rootreader
	cd $BASEDIR/prj/rootreader/rootreader_qt
	qmake rootreader.pro
	if [[ "$TODO_FNC" == "BUILD" ]]; then
		make
	else
		make clean
	fi
	cd $CUR_DIR
	echo "rootreader done!"
}
function rootreader {
	FUNCTION_NAME=rootreader	
	if [[ "$OSTYPE" == "linux-gnu" ]]; then
		LINUX_CODE=`lsb_release -c | cut -f 2`
		if [[ "$LINUX_CODE" == "Santiago" ]]; then
			build_$FUNCTION_NAME
		elif [[ "$LINUX_CODE" == "trusty" ]]; then
			echo "!!!!!! "$FUNCTION_NAME" will not be compiled on ubuntu14"
		else
			echo "!!!!!! Unknown LINUX!"
		fi
	elif [[ "$OSTYPE" == "darwin"* ]]; then
		echo "!!!!!! "$FUNCTION_NAME" will not be compiled on mac"
	elif [[ "$OSTYPE" == "cygwin" ]]; then
		echo "!!!!!! "$FUNCTION_NAME" will not be compiled for cygwin (POSIX compatibility layer and Linux environment emulation for Windows)"
	elif [[ "$OSTYPE" == "msys" ]]; then
		echo "!!!!!! "$FUNCTION_NAME" will not be compiled for msys (Lightweight shell and GNU utilities compiled for Windows (part of MinGW))"
        elif [[ "$OSTYPE" == "win32" ]]; then
		echo "!!!!!! "$FUNCTION_NAME" will not be compiled on 32 bit WINDOWS"
	elif [[ "$OSTYPE" == "freebsd"* ]]; then
		echo "!!!!!! "$FUNCTION_NAME" will not be compiled on freebsd"
	else
        	echo "!!!!!!  ERROR: Unknown OS"
	fi
}


function daq_browser {
	echo daq_browser
	cd $BASEDIR/prj/tools/daq_browser_qt
	qmake qmake daq_browser.pro
	if [[ "$TODO_FNC" == "BUILD" ]]; then
		make
	else
		make clean
	fi
	cd $CUR_DIR
	echo "daq_browser done!"
}


case $OPTION_IN in
	daqinterface)
		daqinterface
		;;
	daqadcreceiver)
		daqadcreceiver
		;;
	librootreader)
		librootreader
		;;
	rootreader)
		rootreader
		;;
	daq_browser)
		daq_browser
		;;
	all|*)
		echo "all|*"
		daqinterface
		daqadcreceiver
		librootreader
		daq_browser
		rootreader
		;;
esac
