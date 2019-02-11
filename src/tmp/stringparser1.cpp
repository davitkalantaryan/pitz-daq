#include "stdafx.h"
#include "linkagesdefinations.h"
#include "stringparser1.h"

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>



#ifdef _MSC_VER
#if(_MSC_VER >= 1400)
#pragma warning(disable : 4996)
#endif
#endif

/*
 * Thios routine finds the buffer, for coresponding file
 * Default reading mode is  "r"
 * but it can be any mode	"rb" - not translated bynary mode for example
 *
 */
LINKAGE_SRC int StringParser1::FileBuffer2(const char* a_FileName, TypeCharPtr*const& a_pBuffer, const char* a_Mode )
{


	FILE* pFile = fopen( a_FileName, a_Mode );

	if( !pFile )
	{
		return -1;
	}

	struct stat fStat;

	if( fstat( fileno( pFile ), &fStat ) )
	{
		
		fclose( pFile );

		return -2;
	}

	int nBytes = (int)fStat.st_size;

	char* buff = (char*)realloc( *a_pBuffer, (nBytes + 2) * sizeof(char) );

	if( !buff )
	{
		fclose( pFile );

		return -3;
	}

	const int nRead = (int)fread( buff, sizeof(char), nBytes, pFile );

	fclose( pFile );

	buff[ nRead ] = 0;
	buff[ nRead + 1 ] = 0;


	*a_pBuffer = buff;

	return nRead;

}



/*
 * Thios routine finds the buffer, for coresponding file
 * Default reading mode is  "r"
 * but it can be any mode	"rb" - not translated bynary mode for example
 *
 */
LINKAGE_SRC int StringParser1::WriteIntoFile(const char* a_FileName, const int& a_nSize, const char*const& a_pBuffer, const char* a_Mode )
{


	FILE* pFile = fopen( a_FileName, a_Mode );

	if( !pFile )
	{
		return -1;
	}

	int nRet = (int)fwrite( a_pBuffer, sizeof(char), a_nSize, pFile );

	return nRet;

}



/*
 * This routine finds folder name for corresponding file
 *
 */
LINKAGE_SRC int StringParser1::FindFolder(const char* a_FileName, TypeCharPtr*const& a_FolderName )
{

	if( !a_FileName )
	{
		return -1;
	}

	//int i, nFlNameLen( (int)strlen(a_FileName) );
	size_t i, unFlNameLen( strlen(a_FileName) );

	bool bNotFound(true);

	for( i = unFlNameLen - 1; bNotFound && i > 0; --i )
	{
		bNotFound = a_FileName[i] != '\\' && a_FileName[i] != '/' ;
	}

	if( bNotFound )
		return 0;

	*a_FolderName = (char*)realloc( *a_FolderName, i + 2 );

	if(i)
		memcpy( *a_FolderName, a_FileName, i + 1 );

	(*a_FolderName)[i + 1] = 0;

	return (int)i;
}


/*
 * This routine ...
 *
 */
LINKAGE_SRC int StringParser1::FindFile(const char* a_RealFileName, TypeCharPtr*const& a_FlName )
{
	
	if( !a_RealFileName )
	{
		return -1;
	}

	size_t i, unFlNameLen( strlen(a_RealFileName) );

	bool bNotFound(true);

	for( i = unFlNameLen - 2; bNotFound && i > 0; --i )
	{
		bNotFound = a_RealFileName[i] != '\\' && a_RealFileName[i] != '/' ;
	}

	if( bNotFound )
		return 0;

	*a_FlName = (char*)realloc( *a_FlName, unFlNameLen - i );

	if(i)
		memcpy( *a_FlName, a_RealFileName + i + 2, unFlNameLen - i );

	return (int)i;
}


LINKAGE_SRC bool StringParser1::ConcatenatePaths( const char* a_FolderName, const char* a_FileName, TypeCharPtr*const& a_Final )
{

	size_t unRealFlName, unFoldNmLen, unFlNmLen;
	char* pcTemp;
	bool bRes;

	if( !a_FolderName && !a_FileName )
	{
		return false;
	}

	if( !a_FolderName || !a_FileName )
	{

		const char* pcSourceTemp = a_FolderName ? a_FolderName : a_FileName ;

		unRealFlName = strlen( pcSourceTemp );

		pcTemp = (char*)realloc( *a_Final, unRealFlName + 1 );

		if( !pcTemp )
		{
			return false;
		}

		*a_Final = pcTemp;

		memcpy( *a_Final, a_FileName, unRealFlName + 1 );
		
		return true;
	
	}

	unFoldNmLen = strlen( a_FolderName );

	unFlNmLen = strlen( a_FileName );

	unRealFlName =	( bRes = (a_FolderName[ unFoldNmLen - 1 ] == '/' || a_FolderName[ unFoldNmLen - 1 ] == '\\') ) ?
					unFoldNmLen + unFlNmLen : unFoldNmLen + unFlNmLen + 1;


	pcTemp = (char*)realloc( *a_Final, unRealFlName + 1 );

	if( !pcTemp )
	{
		return false;
	}

	*a_Final = pcTemp;
	
	memcpy( *a_Final, a_FolderName, unFoldNmLen );


	if( !bRes )
	{
		(*a_Final)[unFoldNmLen++] = '/' ;
	}

	memcpy( *a_Final + unFoldNmLen, a_FileName, unFlNmLen + 1 );


	return true;

}


/*
 * This routine takes out First ML (Multi Line) coment from string
 */
LINKAGE_SRC bool StringParser1::TakeFirstMLComment( char* a_String, const char* a_BgnStr, const char* a_EndStr )
{

	char *pcBegSubString = strstr(a_String,a_BgnStr);
	if(!pcBegSubString)return false;

	char* pcEndSubString = strstr(pcBegSubString,a_EndStr);
	if(!pcEndSubString)return false;

	size_t unEndStrLen(strlen(a_EndStr));

	memmove( pcBegSubString, pcEndSubString+unEndStrLen, strlen(pcEndSubString+unEndStrLen)+1 );

	return true;
}


/*
 * This routine takes out Line coments from string
 */
LINKAGE_SRC bool StringParser1::TakeFirstLineComment( char* a_String, const char* a_CmtStr )
{
	return StringParser1::TakeFirstMLComment( a_String, a_CmtStr, "\n" );
}


/*
 * This routine takes out all ML (Multi Line) coments from string
 */
LINKAGE_SRC void StringParser1::TakeAllMLComments( char* a_String, const char* a_BgnStr, const char* a_EndStr )
{
	while( StringParser1::TakeFirstMLComment( a_String, a_BgnStr, a_EndStr ) );
}


/*
 * This routine takes out all Line coments from string
 */
LINKAGE_SRC void StringParser1::TakeAllLineComments( char* a_String, const char* a_CmtStr )
{
	while( StringParser1::TakeFirstMLComment( a_String, a_CmtStr, "\n" ) );
}



LINKAGE_SRC int StringParser1::IsNextDigit( const char* a_String, char* a_pLast, size_t* a_unEndIndex )
{

	size_t i(0);

	for( ; (a_String[i] < TAKE_OUT_FROM) && a_String[i] ; ++i );

	if( a_pLast )
	{
		(*a_pLast) = a_String[i];
	}

	if( a_unEndIndex )
	{
		*a_unEndIndex = i;
	}

	return isdigit( a_String[i] );
}



LINKAGE_SRC char StringParser1::NextSymbol( char* a_String )
{

	size_t unStrlen;

	if( !a_String || !(unStrlen = strlen(a_String)))
	{
		return (char)0;
	}


	size_t i(0);

	for( ; (a_String[i] < TAKE_OUT_FROM) && a_String[i] ; ++i );

	if( !a_String[i] )
		return a_String[i];


	char cRet(a_String[i]);


	memmove( a_String, a_String + i + 1, unStrlen - i );


	return cRet;
}


/*
 * This routine finds the name of variable, takes out name and sighn of =
 * then return the beginng of variable in    a_unOffset
 *
 */
LINKAGE_SRC bool StringParser1::FindVariableByName(char* a_String, const char* a_VarblName, 
												   size_t*const& a_unpOffset, size_t*const& a_unpStrLen )
{

	if( !a_String )
	{
		//throw "NULL string";
		return false;
	}


	*a_unpStrLen = strlen(a_String);
	size_t unNameLen;
	size_t i, j, unScanRate;


	bool bFounded;

	if( !a_VarblName )
	{
		i = 0;
		unNameLen = 0;
		bFounded = true;
	}
	else
	{
		unNameLen = strlen(a_VarblName);
		if( *a_unpStrLen < unNameLen )
		{
			return false;
		}

		bFounded = false;
		unScanRate = (*a_unpStrLen) - unNameLen;
		for( i = 0; i < unScanRate; ++i )
		{
			// compaering with Variable name
			if( (memcmp( a_String + i, a_VarblName, unNameLen ) == 0) && a_String[i+unNameLen] < TAKE_OUT_FROM )
			{
				// If variable name found, trying to find symbol '='
				bFounded = true;
				break;
				/*for( j = i + unNameLen; j < (*a_unpStrLen); ++j )
				{

					if( a_String[j] == '=' )
					{
						memmove( a_String + i, a_String + j + 1, (*a_unpStrLen) - j );
						*a_unpStrLen = i + (*a_unpStrLen) - j - 1;
						*a_unpOffset = i;
						return true;
					}
				}*/
			}
		}
	}



	////////////////////////////////////////////////////////////
	if( bFounded )
	{
		for( j = i + unNameLen; j < (*a_unpStrLen); ++j )
		{

			if( a_String[j] == '=' )
			{
				memmove( a_String + i, a_String + j + 1, (*a_unpStrLen) - j );
				*a_unpStrLen = i + (*a_unpStrLen) - j - 1;
				*a_unpOffset = i;
				return true;
			}
		}
	}

	return false;
}



LINKAGE_SRC bool StringParser1::FindVariable( char* a_String, const char* a_VarblName, float*const& a_fpVarbl )
{
	size_t unOffset, unStrLen;

	if( !FindVariableByName( a_String, a_VarblName, &unOffset, &unStrLen ) )
	{
		return false;
	}


	char* pDest;

	*a_fpVarbl = (float)strtod( a_String + unOffset, &pDest );

	if( (a_String + unOffset) == pDest )
	{
		return false;
	}

	size_t unDestLen( strlen(pDest) );

	memmove( a_String + unOffset, pDest, unDestLen + 1 );

	return true;
}



LINKAGE_SRC bool StringParser1::FindFirstVariable( char* a_String, float*const& a_fpVarbl )
{
	
	size_t unOffset(0);

	char* pDest;

	*a_fpVarbl = (float)strtod( a_String + unOffset, &pDest );

	if( (a_String + unOffset) == pDest )
	{
		return false;
	}

	size_t unDestLen( strlen(pDest) );

	memmove( a_String + unOffset, pDest, unDestLen + 1 );

	return true;
}



LINKAGE_SRC bool StringParser1::FindFirstVariable( char* a_String, double*const& a_plfVarbl )
{
	size_t unOffset(0);

	char* pDest;

	*a_plfVarbl = strtod( a_String + unOffset, &pDest );

	if( (a_String + unOffset) == pDest )
	{
		return false;
	}

	size_t unDestLen( strlen(pDest) );

	memmove( a_String + unOffset, pDest, unDestLen + 1 );

	return true;
}



LINKAGE_SRC bool StringParser1::FindFirstVariable( char* a_String, long*const& a_lpVarbl )
{
	size_t unOffset(0);


	char* pDest;

	*a_lpVarbl = strtol( a_String + unOffset, &pDest, 10 );

	if( (a_String + unOffset) == pDest )
	{
		return false;
	}

	size_t unDestLen( strlen(pDest) );

	memmove( a_String + unOffset, pDest, unDestLen + 1 );

	return true;
}



LINKAGE_SRC bool StringParser1::FindFirstVariable( char* a_String, int*const& a_pnVarbl )
{
	size_t unOffset(0);

	char* pDest;

	*a_pnVarbl = (int)strtol( a_String + unOffset, &pDest, 10 );

	if( (a_String + unOffset) == pDest )
	{
		return false;
	}

	size_t unDestLen( strlen(pDest) );

	memmove( a_String + unOffset, pDest, unDestLen + 1 );

	return true;
}



LINKAGE_SRC bool StringParser1::FindVariable( char* a_String, const char* a_VarblName, double*const& a_plfVarbl )
{
	size_t unOffset, unStrLen;

	if( !FindVariableByName( a_String, a_VarblName, &unOffset, &unStrLen ) )
	{
		return false;
	}


	char* pDest;

	*a_plfVarbl = strtod( a_String + unOffset, &pDest );

	if( (a_String + unOffset) == pDest )
	{
		return false;
	}

	size_t unDestLen( strlen(pDest) );

	memmove( a_String + unOffset, pDest, unDestLen + 1 );

	return true;
}



LINKAGE_SRC bool StringParser1::FindDigits( char* a_String, const char* a_VarblName, int a_nNumber, ... )
{

	size_t unOffset, unStrLen;

	if( !FindVariableByName( a_String, a_VarblName, &unOffset, &unStrLen ) )
	{
		return false;
	}


	char* pDest;
	double* plfDigit;
	size_t unDestLen;


	va_list args;

	va_start( args, a_nNumber );

	for( int i(0); i < a_nNumber; ++i )
	{
		plfDigit = va_arg(args,double*);

		if(plfDigit)
		{
			*plfDigit = strtod( a_String + unOffset, &pDest );

			unDestLen = strlen(pDest) ;

			if( unDestLen)
			{
				memmove( a_String + unOffset, pDest + 1, unDestLen );
			}
		}
	}

	va_end( args );


	return true;
}



LINKAGE_SRC bool StringParser1::FindVariable( char* a_String, const char* a_VarblName, long*const& a_lpVarbl )
{
	size_t unOffset, unStrLen;

	if( !FindVariableByName( a_String, a_VarblName, &unOffset, &unStrLen ) )
	{
		return false;
	}


	char* pDest;

	*a_lpVarbl = strtol( a_String + unOffset, &pDest, 10 );

	if( (a_String + unOffset) == pDest )
	{
		return false;
	}

	size_t unDestLen( strlen(pDest) );

	memmove( a_String + unOffset, pDest, unDestLen + 1 );

	return true;
}



LINKAGE_SRC bool StringParser1::FindVariable( char* a_String, const char* a_VarblName, int*const& a_pnVarbl )
{
	size_t unOffset, unStrLen;

	if( !FindVariableByName( a_String, a_VarblName, &unOffset, &unStrLen ) )
	{
		return false;
	}


	char* pDest;

	*a_pnVarbl = (int)strtol( a_String + unOffset, &pDest, 10 );

	if( (a_String + unOffset) == pDest )
	{
		return false;
	}

	size_t unDestLen( strlen(pDest) );

	memmove( a_String + unOffset, pDest, unDestLen + 1 );

	return true;
}



LINKAGE_SRC bool StringParser1::FindVariable( char* a_String, const char* a_VarblName, TypeCharPtr*const& a_spVarbl )
{

	size_t j, unOffset, unStrLen;

	if( !FindVariableByName( a_String, a_VarblName, &unOffset, &unStrLen ) )
	{
		return false;
	}

	for( size_t i(unOffset); i < unStrLen; ++i )
	{
		
		if( a_String[i] == '\"' )
		{
			for( j = i + 1; j < unStrLen; ++j/*++j, ++unNewStrLen*/ )
			{
				if( a_String[j] == '\\' )
				{
					if( j == unStrLen - 1 )
					{
						return false;
					}

					memmove( a_String + j, a_String + j + 1, unStrLen - j );

					--unStrLen;
				}
				else if( a_String[j] == '\"' )
				{
					*a_spVarbl = (char*)realloc( *a_spVarbl, j - i );

					if( j - i - 1 )
					{
						memcpy( *a_spVarbl, a_String + i + 1, j - i - 1 );
					}

					(*a_spVarbl)[j-i-1] = 0;

					memmove( a_String + i, a_String + j + 1, unStrLen + 1 - j );
					return true;
				}
			}
		}
	}


	return false;

}



LINKAGE_SRC bool StringParser1::FindFirstStringSmpl(char* a_String, char* a_pcBuffer, int a_nBufLen )
{

	if( !a_String )
		return false;

	size_t i(0);

	// Mooving pointer to the begining of string
	for( ; a_String[i] < TAKE_OUT_FROM && a_String[i] != 0; ++i );

	if( !a_String[i] )
	{
		return false;
	}

	// remembering begining of string
	size_t unInd(i), unStrLen(0);

	//calculating length of string
	for( ; a_String[i] > TAKE_OUT_FROM; ++i, ++unStrLen );

	unStrLen = ((size_t)(a_nBufLen-1)<unStrLen) ? (size_t)(a_nBufLen-1) : unStrLen;

	memcpy( a_pcBuffer, a_String + unInd, unStrLen );
	a_pcBuffer[ unStrLen ] = 0;
	memmove( a_String, a_String + unInd + unStrLen, i + 1 );


	return true;

}



LINKAGE_SRC bool StringParser1::FindFirstString(char* a_String, TypeCharPtr*const& a_spValue )
{

	if( !a_String )
		return false;

	size_t i(0);

	// Mooving pointer to the begining of string
	for( ; a_String[i] < TAKE_OUT_FROM && a_String[i] != 0; ++i );

	if( !a_String[i] )
	{
		return false;
	}

	// remembering begining of string
	size_t unInd(i), unStrLen(0);

	//calculating length of string
	for( ; a_String[i] > TAKE_OUT_FROM; ++i, ++unStrLen );


	// reallocing memory and copying nikname
	bool bRet;

	char *aTemp, *aString = *a_spValue;

	aTemp = (char*)realloc( aString, 1 + unStrLen );

	if( aTemp )
	{
		aString = aTemp;
		bRet = true;
		memcpy( aString, a_String + unInd, unStrLen );
		aString[ unStrLen ] = 0;
		i = strlen( a_String + unInd + unStrLen );
		memmove( a_String, a_String + unInd + unStrLen, i + 1 );
	}
	else
	{
		bRet = false;
		//m_ErrorPrinter.ErrorFunc( "%s: There is not enough memory to create buffer for %s", a_FncName, a_Src );
	}

	*a_spValue = aString;


	return bRet;

}



LINKAGE_SRC bool StringParser1::AllocAndCopyStr(const char* a_pcSource, TypeCharPtr*const& a_ppcDest, size_t a_unAdditionBytes)
{

	if( !a_pcSource )
		return false;

	size_t unStrLen( strlen(a_pcSource) );

	if( !unStrLen )
		return false;

	char*& pcDest = *a_ppcDest;

	char* pcTemp = (char*)realloc(pcDest, (unStrLen + 1) /** sizeof(char)*/ + a_unAdditionBytes);

	if( !pcTemp )
	{
		//throw "Low memory!";
		return false;
	}

	pcDest = pcTemp;

	memcpy( pcDest, a_pcSource, (unStrLen+1) /** sizeof(char)*/ );

	return true;

}
