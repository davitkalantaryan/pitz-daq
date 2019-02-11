#ifndef __stringparser1_h__
#define __stringparser1_h__

#define		TAKE_OUT_FROM	36

#include <stddef.h>

//#include "repanderrprinter.h"

#ifndef LINKAGE_HDR
	#define LINKAGE_HDR
#endif

typedef char* TypeCharPtr;


class StringParser1
{
public:
	/*
	 * Thios routine finds the buffer, for coresponding file
	 * Default reading mode is  "r"
	 * but it can be any mode	"rb" - not translated bynary mode for example
	 *
	 */
	LINKAGE_HDR static int FileBuffer2(const char* a_FileName, TypeCharPtr*const& a_pBuffer, const char* a_Mode = "r" );

	/*
	 * Thios routine finds the buffer, for coresponding file
	 * Default reading mode is  "w"
	 * but it can be any mode	"wb" - not translated bynary mode for example
	 *
	 */
	LINKAGE_HDR static int WriteIntoFile(const char* a_FileName, const int& a_nSize, const char*const& a_pBuffer, const char* a_Mode = "w" );

	/*
	 * This routine finds folder name for corresponding file
	 *
	 */
	LINKAGE_HDR static int FindFolder(const char* a_FileName, TypeCharPtr*const& a_FolderName );

	/*
	 * This routine ...
	 *
	 */
	LINKAGE_HDR static int FindFile(const char* a_RealFileName, TypeCharPtr*const& a_FolderName );

	/*
	 *
	 */
	LINKAGE_HDR static bool ConcatenatePaths( const char* a_FolderName, const char* a_FileName, TypeCharPtr*const& a_Final );

	/*
	 * This routine takes out First ML (Multi Line) coment from string
	 */
	LINKAGE_HDR static bool TakeFirstMLComment( char* a_String, const char* a_BgnStr, const char* a_EndStr );

	/*
	 * This routine takes out Line coments from string
	 */
	LINKAGE_HDR static bool TakeFirstLineComment( char* a_String, const char* a_CmtStr );

	/*
	 * This routine takes out all ML (Multi Line) coments from string
	 */
	LINKAGE_HDR static void TakeAllMLComments( char* a_String, const char* a_BgnStr, const char* a_EndStr );


	/*
	 * This routine takes out all Line coments from string
	 */
	LINKAGE_HDR static void TakeAllLineComments( char* a_String, const char* a_CmtStr );

	LINKAGE_HDR static int IsNextDigit( const char* a_String, char* a_pLast, size_t* a_unEndIndex );

	LINKAGE_HDR static char	NextSymbol( char* a_String );

	LINKAGE_HDR static bool FindFirstVariable( char* a_String, float*const& a_fpVarbl );

	LINKAGE_HDR static bool FindFirstVariable( char* a_String, double*const& a_plfVarbl );

	LINKAGE_HDR static bool FindFirstVariable( char* a_String, long*const& a_lpVarbl );

	LINKAGE_HDR static bool FindFirstVariable( char* a_String, int*const& a_pnVarbl );

	/*
	 * This routine finds the name of variable, takes out name and sighn of =
	 * then return the beginng of variable in    a_unOffset
	 *
	 */
	LINKAGE_HDR static bool FindVariableByName( char* a_String, const char* a_VaribleName,
												size_t*const& a_unpOffset, size_t*const& a_unpStrLen );

	/*
	 *
	 */
	LINKAGE_HDR static bool FindVariable( char* a_String, const char* a_VaribleName, float*const& a_fpValue );

	/*
	 *
	 */
	LINKAGE_HDR static bool FindVariable( char* a_String, const char* a_VarblName, double*const& a_plfVarbl );

	/*
	 *
	 */
	LINKAGE_HDR static bool FindVariable( char* a_String, const char* a_VarblName, long*const& a_lpVarbl );

	/*
	 *
	 */
	LINKAGE_HDR static bool FindVariable( char* a_String, const char* a_VarblName, int*const& a_pnVarbl );

	/*
	 *
	 */
	LINKAGE_HDR static bool FindVariable( char* a_String, const char* a_VaribleName, TypeCharPtr*const& a_fpValue );

	/*
	 *
	 */
	LINKAGE_HDR static bool FindDigits( char* a_String, const char* a_VarblName, int a_nNumber, ... );

	/*
	 * This method 
	 *
	 */
	LINKAGE_HDR static bool FindFirstString(char* a_String, TypeCharPtr*const& a_spValue);

	LINKAGE_HDR static bool FindFirstStringSmpl(char* a_String, char* a_pcBuffer, int a_nBufLen );

	/*
	 * This methot realloc needed memory then copies the string
	 */
	LINKAGE_HDR static bool AllocAndCopyStr(const char* a_pcSource, TypeCharPtr*const& a_ppcDest, size_t aditionalBytes=0 );


};


#endif
