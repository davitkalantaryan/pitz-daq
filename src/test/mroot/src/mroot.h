/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) 2008 Johannes Kissel<johannes.kissel@ipe.fzk.de>        *
 *                                                                       *
 * This file is part of the mroot interface.                             *
 *                                                                       *
 * mroot is free software: you can redistribute it and/or modify         *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * mroot is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with mroot. If not, see <http://www.gnu.org/licenses/>.         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * The main header file of the mroot interface.
 *
 * @author Johannes Kissel<johannes.kissel@ipe.fzk.de>
 */

////////////////
//  Includes  //
////////////////

// Windows Compliance for ROOT
#if defined _WIN32
	#include "w32pragma.h"
#endif

// ROOT
#include "TFile.h"
#include "TXMLFile.h"
#include "TNetFile.h"
#include "TSQLFile.h"

#include "TAuthenticate.h"

#include "TObjArray.h"
#include "TMap.h"
#include "TBits.h"
#include "TObjString.h"
#include "TVectorT.h"
#include "TList.h"

// Tools
#if defined PROFILE
	#define SHINY_PROFILER TRUE
	#include "Shiny.h"
#endif

#if defined PATROL
	#include "mpatrol.h"
#endif

// MATLAB
#include "mex.h"
#include "matrix.h"

////////////////////
//  Enumerations  //
////////////////////

/** File Types */
typedef enum { BIN, XML, NET, SQL } File_t;

/** Message Types */
typedef enum { MSG, WRN, ERR } Msg_t;

/** Transformation directions */
typedef enum { R2M, M2R } Dir_t;

/** File open modes */
typedef enum { NEW, CREATE, RECREATE, UPDATE, READ} Open_t;

//////////////////////
//  Function Stubs  //
//////////////////////

/**
 * Prints a message with format tags via MEX funtions.
 * 
 * mexWarnMsgTxt and mexErrMsgTxt do not support format tags.
 * mexMsgTxt adds this feature using format tags (of any kind and number).
 * This is made possible by cstdarg.h for variable arguments handling.
 * 
 * @param type MSG, WARN or ERR
 * @param str Format string
 */
void mexMsgTxt(Msg_t type, char *str, ...);

/**
 * Extracts a MATLAB-String to a char array.
 * @param str The MATLAB-String
 * @return Extracted char array
 */
char *mxGetStr(mxArray *mstr);

/**
 * Convert a MATLAB-String to a TObjString.
 * @param str The MATLAB-String
 * @return The TObjString
 */
TObjString *mxGetObjStr(mxArray *mstr);

/**
 * Open a ROOT file.
 * @param ofile A struct specifying a ROOT file
 * @return A TFile object
 */
TFile *openFile(const mxArray *ofile, Open_t mode);

/**
 * Get mroot version with which file was saved.
 * (Assumes pwd to be file root.)
 * @return Version number as MATLAB-String
 */
mxArray *getFVer();

/**
 * Sequentially change directory.
 * @param ipath Array with subdirectories
 * @param mk Force directories to be created
 * @return TRUE on success
 */
bool cd_seq(const mxArray *ipath, bool mk);

/**
 * Workaround to recursively delete ROOT collections (TMap, TCollection)
 * while their SetOwner method seems not to work.
 * @param obj The collection/object to delete
 */
void rdelete(TObject *obj);

/**
 * Calls MATLAB to reformat given mxArray for easier transformation.
 * @param mxVar The mxArray
 * @param direction Direction of transformation
 * @return The reformated mxArray
 */
const mxArray *reformat(const mxArray *mxVar, Dir_t direction);
