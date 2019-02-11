//
// file:        formexdaq_browser_dynuser.ch
//
#ifndef FORMEXDAQ_BROWSER_DYNUSER_H
#define FORMEXDAQ_BROWSER_DYNUSER_H

//
// file:        mex_simple_root_reader.cpp
//

#ifdef _WIN32
#define FILE_DELIMER    '\\'
#else
#include <dlfcn.h>
//#define LoadLibrary
#define FILE_DELIMER    '/'
#endif
#include "tool/simple_root_reader.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <vector>
#include <string>

int ReadOneRootFileToVectorDl(const char* a_cpcRootFileName, const char* a_cpcTreeAndBranchName, std::vector<structCommon>* outVector);
int GetAllEntriesInTheRootFileToVectorDl(const char* a_cpcRootFileName, std::vector<std::string>* a_outVector);
int MultyReadOneRootFileToVectorDl(const char* a_cpcRootFileName,std::vector<std::vector<structCommon> >* a_outVector,
                                   const std::vector<std::string>& a_daqNames, int a_start, int a_end);
int MultyReadEntriesFromIndexToVectorDl(std::vector<std::vector<structCommon> >* a_outVector,
                                   const std::vector<std::string>& a_daqNames, int a_start, int a_end);


#ifdef __cplusplus
extern "C"{
#endif

extern const size_t g_unErrorStrLen;
extern int g_nLogLevel;
extern int (*g_fpReport)(const char* format, ...) ;
void* InitFunctionGlb(void** a_ppHandle,const char* a_fncName);

int ReadOneRootFileToClbkDl(ReadOneFileArgs);
int GetAllEntriesInTheRootFileDl(GetAllEntriesArgs);


#define FILE_FROM_PATH(__path) ( strrchr((__path),FILE_DELIMER) ? (strrchr((__path),FILE_DELIMER)+1) : (__path) )

#ifdef __cplusplus
}
#endif

int MultyReadOneRootFileToClbkCDl(const char* a_cpcRootFileName, const std::vector<std::string>& a_branchNames,
                          structCommon* a_pStrForRoot,void* a_pOwner,TypeMultyRtReader a_fpReader, TypeReport a_fpReport);

#define DEBUG_ROOT_APP(__logLevel,...)  \
    do{ \
        if((__logLevel)<=g_nLogLevel) { \
            (*g_fpReport)("fl:%s,ln:%d -> ",FILE_FROM_PATH(__FILE__),__LINE__);(*g_fpReport)(__VA_ARGS__);(*g_fpReport)("\n"); \
        } \
    }while(0)

#endif // #ifndef FORMEXDAQ_BROWSER_DYNUSER_H
