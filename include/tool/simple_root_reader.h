//
// file:        simple_root_reader.h
// created on:  2018 Jun 08
// created by:  D. Kalantaryan (davit.kalantaryan@desy.de)
//
#ifndef SIMPLE_ROOT_READER_H
#define SIMPLE_ROOT_READER_H

#include <stdint.h>
#include <cpp11+/common_defination.h>
#include "pitz/daq/data_and_memory.hpp"
#include <pitz/daq/dataclientbase.hpp>

#define MARKED_ERROR       "ERROR:"
#define RtReaderLibraryName "libformexdaq_browser.so.4"
#define ReadOneRootFileToClbkFncName "ReadOneRootFileToClbk"
#define EntryGetterFncName "GetAllEntriesInTheRootFile"
#define MultyReadOneRootFileToClbkCName "MultyReadOneRootFileToClbkC"

enum callbackReturnType{callbackReturn_fatal=-1,callbackReturn_finish=0,callbackReturn_finishForCurrent=1,callbackReturn_continue};


typedef int                (*TypeReport)(void*owner,const char* format,...);
typedef callbackReturnType (*TypeRtReader)(void*owner,int index, int count,structCommonNew** ppForRoot);
typedef callbackReturnType (*TypeMultyRtReader)(void*owner,int index, int count,int branchIndex);
typedef void*              (*TypeBufferAddressGetter)(void*owner,int branchIndex);
typedef callbackReturnType (*TypeEntriesCountHandler)(void*owner,int count);
typedef callbackReturnType (*TypeBranchNameHandler)(void*owner,int index,const pitz::daq::dataClient::StrForEntries& a_info);

#define ReadOneFileArgs const char* a_cpcRootFileName, const char* a_cpcTreeAndBranchName, \
                        structCommonNew* a_pStrForRoot,void* a_pOwner,TypeRtReader a_fpReader, TypeReport a_fpReport
#define GetAllEntriesArgs   const char* a_cpcRootFileName, void* a_pOwner, TypeEntriesCountHandler a_fpEntriesCount,TypeBranchNameHandler a_brahNameHandler,TypeReport a_fpReport
#define MultyReadOneRootFileArgs    const char* a_cpcRootFileName, void* a_pBranchNames, \
                                    structCommonNew* a_pStrForRoot,void* a_pOwner,TypeMultyRtReader a_fpReader, TypeReport a_fpReport

typedef int (*TypeReadOneRootFileToClbk)(ReadOneFileArgs);
int ReadOneRootFileToClbk(ReadOneFileArgs);

typedef int (*TypeGetAllEntriesInTheRootFile)(GetAllEntriesArgs);
int GetAllEntriesInTheRootFile(GetAllEntriesArgs);

typedef int (*TypeMultyReadOneRootFile)(MultyReadOneRootFileArgs);

class TBranch;

const char* GetDataTypeAndCount(const TBranch* a_pBranch, structBranchDataInfo* pInfo);


#endif // SIMPLE_ROOT_READER_H
