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
 * Provides some basic functions to the mroot interface.
 * 
 * @author Johannes Kissel<johannes.kissel@ipe.fzk.de>
 */

#include "mroot.h"

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
void mexMsgTxt(Msg_t type, char *str, ...) {
	char *msg = (char*) malloc(256); // max length of messages
	if (msg == NULL) mexErrMsgTxt("Memory allocation failed.");
	
	va_list args;
	va_start(args, str);
	vsprintf(msg, str, args);
	va_end(args);
	
	switch (type) {
		case MSG: mexPrintf(msg);     break;
		case ERR: mexErrMsgTxt(msg);  break;
		case WRN: mexWarnMsgTxt(msg); break;
	}
	
	free(msg);
}

/**
 * Extracts a MATLAB-String to a char array.
 * @param str The MATLAB-String
 * @return Extracted char array
 */
char *mxGetStr(mxArray *mstr) {
	int   len = mxGetNumberOfElements(mstr) + 1;
	char *str = (char*) malloc(len);
	
	if (str != NULL && mxGetString(mstr, str, len) != 0) {
		free(str);
		str  = NULL;
	}
	
	return str;
}

/**
 * Convert a MATLAB-String to a TObjString.
 * @param str The MATLAB-String
 * @return The TObjString
 */
TObjString *mxGetObjStr(mxArray *mstr) {
	char *str = mxGetStr(mstr);
	TObjString *obj = NULL;
	
	if (str != NULL) {
		obj = new TObjString(str);
		free(str);
	}
	
	return obj;
}

/**
 * Open a ROOT file.
 * @param ofile A struct specifying a ROOT file
 * @return A TFile object
 */
TFile *openFileRaw(const char* a_path, Open_t a_mode) {
	
	

	
    const char *m;
	
    switch (a_mode) {
		case NEW:
		case CREATE:   m = "CREATE";   break;
		case RECREATE: m = "RECREATE"; break;
		case UPDATE:   m = "UPDATE";   break;
		case READ:     m = "READ";     break;
	}
	
    TFile *file = new TFile(   a_path, m);
	
	// check file
    if ((!file) || file->IsZombie() ) {
		delete file;
		throw 1;
	}
		
	return file;
}


/**
 * Open a ROOT file.
 * @param ofile A struct specifying a ROOT file
 * @return A TFile object
 */
TFile *openFile(const mxArray *ofile, Open_t mode) {
    File_t type = (File_t) (int) mxGetScalar(mxGetField(ofile, 0, "type"));
    char *path = NULL, *usr = NULL, *pwd = NULL;

    try { path = mxGetStr(mxGetField(ofile, 0, "path")); } catch (int e) { throw 0; }

    if (type > 1) { // TNetFile or TSQLFile
        try {
            usr = mxGetStr(mxGetField(ofile, 0, "usr"));
            pwd = mxGetStr(mxGetField(ofile, 0, "pwd"));
        } catch (int e) {
            free(path);
            throw 0;
        }
    }

    char *m;

    switch (mode) {
        case NEW:
        case CREATE:   m = "CREATE";   break;
        case RECREATE: m = "RECREATE"; break;
        case UPDATE:   m = "UPDATE";   break;
        case READ:     m = "READ";     break;
    }

    TFile *file = NULL;

    // files are always opened with update option
    switch (type) {
        case BIN: {
            file = new TFile(   path, m);
            break;
        }
        case XML: {
            file = new TXMLFile(path, m);
            break;
        }
        case NET: {
            TAuthenticate::SetGlobalUser(usr);
            TAuthenticate::SetGlobalPasswd(pwd);

            file = new TNetFile(path, m);
            break;
        }
        case SQL: {
            file = new TSQLFile(path, m, usr, pwd);
            break;
        }
    }

    // free stuff
    free(path);

    if (type > 1) { // TNetFile or TSQLFile
        free(usr);
        free(pwd);
    }


    // check file
    if (file->IsZombie() || file == NULL) {
        delete file;
        throw 1;
    }

    // check file version
    mxArray *fver = getFVer(),
             *ver = mxGetField(ofile, 0, "version");

    if (fver == NULL) {
        if (mode == READ) {
            throw 2;
        } else { // save it
            TMap *info = new TMap();
            info->Add(new TObjString("version"), mxGetObjStr(ver));
            info->Write("rootinfo", TObject::kSingleKey);
            rdelete(info);
        }
    } else {
        mxArray *lhs, *rhs[2];

        rhs[0] = fver;
        rhs[1] =  ver;

        int res = mexCallMATLAB(1, &lhs, 2, &rhs[0], "reformat");
        if (res != 0) { throw 2; }

        if (mxGetScalar(lhs) > 0) { throw 2; }
    }

    return file;
}

/**
 * Get mroot version with which file was saved.
 * (Assumes pwd to be file root.)
 * @return Version number as MATLAB-String
 */
mxArray *getFVer() {
	mxArray *ver = NULL;
	TMap *info = (TMap*) gDirectory->Get("mroot");
	
	if (info != NULL) {
		TObjString *v = (TObjString*) info->GetValue("version");
		
		if (v != NULL) {
			ver = mxCreateString(v->GetName());
		}
	}
	
	return ver;
}

/**
 * Sequentially change directory.
 * @param ipath Array with subdirectories
 * @param mk Force directories to be created
 * @return TRUE on success
 */
bool cd_seq(const mxArray *ipath, bool mk = false) {
	bool success = true;
	int n = mxGetNumberOfElements(ipath);
	
	for (int i = 0; i < n; i++) {
		char *dir = mxGetStr(mxGetCell(ipath, i));
		if (dir == NULL) return false;
		
		if (mk) { gDirectory->mkdir(dir); }
		
		success = gDirectory->cd(dir);
		free(dir);
		
		if (!success) return false;
	}
	
	return success;
}

/**
 * Workaround to recursively delete ROOT collections (TMap, TCollection)
 * while their SetOwner method seems not to work.
 * @param obj The collection/object to delete
 */
void rdelete(TObject *obj) {
	if (obj->InheritsFrom("TMap")) { // TMap
 		TMap *map = (TMap*) obj;
		TMapIter *iter = new TMapIter(map);
		
		while (TObject *key = iter->Next()) {
			rdelete(map->GetValue(key));
			delete key;
		}
		
		delete iter;
		delete map;
	} else if (obj->InheritsFrom("TCollection")) { // another TCollection
		TCollection *col = (TSeqCollection*) obj;
		TIterator *iter = col->MakeIterator();
		
		while (TObject *item = iter->Next()) {
			rdelete(item);
		}
		
		delete col;
	} else { // another TObject
		delete obj;
	}
}

/**
 * Calls MATLAB to reformat given mxArray for easier transformation.
 * @param mxVar The mxArray
 * @param direction Direction of transformation
 * @return The reformated mxArray
 */
const mxArray *reformat(const mxArray *mxVar, Dir_t direction) {
	mxArray *lhs, *rhs[2];
	
	rhs[0] = (mxArray*) mxVar;
	rhs[1] = mxCreateDoubleScalar(direction);
	
	int res = mexCallMATLAB(1, &lhs, 2, &rhs[0], "reformat");
	if (res != 0) { throw 0; }
	
	return lhs;
}
