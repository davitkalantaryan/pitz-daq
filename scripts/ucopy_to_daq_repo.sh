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


cp $sourceOutBase/common_argument_parser.cpp                     $sourceInpBase/common/.


cp $sourceOutBase/common_iodevice.cpp                            $repoBase/contrib/cpp-raft/common/.              
cp $sourceOutBase/common_servertcp.cpp                           $repoBase/contrib/cpp-raft/common/.             
cp $sourceOutBase/common_socketbase.cpp                          $repoBase/contrib/cpp-raft/common/.            
cp $sourceOutBase/common_sockettcp.cpp                           $repoBase/contrib/cpp-raft/common/.             
cp $sourceOutBase/desy_interlocknotifier_server.cpp              $sourceInpBase/server/.            
cp $sourceOutBase/main_interlock_notifier_server.cpp             $sourceInpBase/server/.           
cp $sourceOutBase/mutex_cpp11.cpp                                $repoBase/contrib/cpp-raft/common/cpp11+/.            
cp $sourceOutBase/shared_mutex_cpp14.cpp                         $repoBase/contrib/cpp-raft/common/cpp11+/.     
cp $sourceOutBase/thread_cpp11.cpp                               $repoBase/contrib/cpp-raft/common/cpp11+/.           

cp $includeOutBase/desy/interlocknotifier/server.hpp             $includeInpBase/desy/interlocknotifier/.                         
cp $includeOutBase/desy/interlocknotifier/server_client_common.h $includeInpBase/desy/interlocknotifier/.             

cp $includeOutBase/common/common_argument_parser.hpp             $includeInpBase/common/.                         
cp $includeOutBase/common/common_argument_parser.impl.hpp        $includeInpBase/common/.                    

cp $includeOutBase/common/common_iodevice.hpp                    $repoBase/contrib/cpp-raft/include/common/.             
cp $includeOutBase/common/common_servertcp.hpp                   $repoBase/contrib/cpp-raft/include/common/.            
cp $includeOutBase/common/common_socketbase.hpp                  $repoBase/contrib/cpp-raft/include/common/.           
cp $includeOutBase/common/common_sockettcp.hpp                   $repoBase/contrib/cpp-raft/include/common/.            
cp $includeOutBase/common/impl.common_socketbase.hpp             $repoBase/contrib/cpp-raft/include/common/.      
cp $includeOutBase/common/impl.common_servertcp.hpp              $repoBase/contrib/cpp-raft/include/common/.       
cp $includeOutBase/common/listspecialandhashtbl.hpp              $repoBase/contrib/cpp-raft/include/common/.       
cp $includeOutBase/common/impl.listspecialandhashtbl.hpp         $repoBase/contrib/cpp-raft/include/common/.  
cp $includeOutBase/common/common_hashtbl.hpp                     $repoBase/contrib/cpp-raft/include/common/.              
cp $includeOutBase/common/impl.common_hashtbl.hpp                $repoBase/contrib/cpp-raft/include/common/.         
cp $includeOutBase/common/lists.hpp                              $repoBase/contrib/cpp-raft/include/common/.                       
cp $includeOutBase/common/impl.lists.hpp                         $repoBase/contrib/cpp-raft/include/common/.                  
cp $includeOutBase/common/common_unnamedsemaphorelite.hpp        $repoBase/contrib/cpp-raft/include/common/. 
cp $includeOutBase/common/newlockguards.hpp                      $repoBase/contrib/cpp-raft/include/common/.               
cp $includeOutBase/common/impl.newlockguards.hpp                 $repoBase/contrib/cpp-raft/include/common/.          
                                                                 
cp $includeOutBase/cpp11+/common_defination.h                    $repoBase/contrib/cpp-raft/include/cpp11+/.             
cp $includeOutBase/cpp11+/shared_mutex_cpp14.hpp                 $repoBase/contrib/cpp-raft/include/cpp11+/.          
cp $includeOutBase/cpp11+/thread_cpp11.hpp                       $repoBase/contrib/cpp-raft/include/cpp11+/.                
cp $includeOutBase/cpp11+/thread_cpp11.impl.hpp                  $repoBase/contrib/cpp-raft/include/cpp11+/.           
cp $includeOutBase/cpp11+/mutex_cpp11.hpp                        $repoBase/contrib/cpp-raft/include/cpp11+/.                 

