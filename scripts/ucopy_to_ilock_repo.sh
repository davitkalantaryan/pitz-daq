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

mkdir -p $DEST_DIR/scripts
mkdir -p $sourceOutBase
mkdir -p $includeOutBase
mkdir -p $includeOutBase/desy
mkdir -p $includeOutBase/desy/interlocknotifier
mkdir -p $includeOutBase/common
mkdir -p $includeOutBase/cpp11+

cp $scriptDir/Makefile                                                       $DEST_DIR/.
cp $scriptDir/ucopy_on_ilock_repo.sh                                         $DEST_DIR/scripts/.

cp $sourceInpBase/common/common_argument_parser.cpp                          $sourceOutBase/.
cp $repoBase/contrib/cpp-raft/common/common_iodevice.cpp                     $sourceOutBase/.
cp $repoBase/contrib/cpp-raft/common/common_servertcp.cpp                    $sourceOutBase/.
cp $repoBase/contrib/cpp-raft/common/common_socketbase.cpp                   $sourceOutBase/.
cp $repoBase/contrib/cpp-raft/common/common_sockettcp.cpp                    $sourceOutBase/.
cp $sourceInpBase/server/desy_interlocknotifier_server.cpp                   $sourceOutBase/.
cp $sourceInpBase/server/main_interlock_notifier_server.cpp                  $sourceOutBase/.
cp $repoBase/contrib/cpp-raft/common/cpp11+/mutex_cpp11.cpp                  $sourceOutBase/.
cp $repoBase/contrib/cpp-raft/common/cpp11+/shared_mutex_cpp14.cpp           $sourceOutBase/.
cp $repoBase/contrib/cpp-raft/common/cpp11+/thread_cpp11.cpp                 $sourceOutBase/.

cp $includeInpBase/desy/interlocknotifier/server.hpp                         $includeOutBase/desy/interlocknotifier/.
cp $includeInpBase/desy/interlocknotifier/server_client_common.h             $includeOutBase/desy/interlocknotifier/.

cp $includeInpBase/common/common_argument_parser.hpp                         $includeOutBase/common/.
cp $includeInpBase/common/common_argument_parser.impl.hpp                    $includeOutBase/common/.

cp $repoBase/contrib/cpp-raft/include/common/common_iodevice.hpp             $includeOutBase/common/.
cp $repoBase/contrib/cpp-raft/include/common/common_servertcp.hpp            $includeOutBase/common/.
cp $repoBase/contrib/cpp-raft/include/common/common_socketbase.hpp           $includeOutBase/common/.
cp $repoBase/contrib/cpp-raft/include/common/common_sockettcp.hpp            $includeOutBase/common/.
cp $repoBase/contrib/cpp-raft/include/common/impl.common_socketbase.hpp      $includeOutBase/common/.
cp $repoBase/contrib/cpp-raft/include/common/impl.common_servertcp.hpp       $includeOutBase/common/.
cp $repoBase/contrib/cpp-raft/include/common/listspecialandhashtbl.hpp       $includeOutBase/common/.
cp $repoBase/contrib/cpp-raft/include/common/impl.listspecialandhashtbl.hpp  $includeOutBase/common/.
cp $repoBase/contrib/cpp-raft/include/common/common_hashtbl.hpp              $includeOutBase/common/.
cp $repoBase/contrib/cpp-raft/include/common/impl.common_hashtbl.hpp         $includeOutBase/common/.
cp $repoBase/contrib/cpp-raft/include/common/lists.hpp                       $includeOutBase/common/.
cp $repoBase/contrib/cpp-raft/include/common/impl.lists.hpp                  $includeOutBase/common/.
cp $repoBase/contrib/cpp-raft/include/common/common_unnamedsemaphorelite.hpp $includeOutBase/common/.
cp $repoBase/contrib/cpp-raft/include/common/newlockguards.hpp               $includeOutBase/common/.
cp $repoBase/contrib/cpp-raft/include/common/impl.newlockguards.hpp          $includeOutBase/common/.

cp $repoBase/contrib/cpp-raft/include/cpp11+/common_defination.h             $includeOutBase/cpp11+/.
cp $repoBase/contrib/cpp-raft/include/cpp11+/shared_mutex_cpp14.hpp          $includeOutBase/cpp11+/.
cp $repoBase/contrib/cpp-raft/include/cpp11+/thread_cpp11.hpp                $includeOutBase/cpp11+/.
cp $repoBase/contrib/cpp-raft/include/cpp11+/thread_cpp11.impl.hpp           $includeOutBase/cpp11+/.
cp $repoBase/contrib/cpp-raft/include/cpp11+/mutex_cpp11.hpp                 $includeOutBase/cpp11+/.

