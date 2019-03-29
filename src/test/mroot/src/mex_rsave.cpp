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
 * MEX code corresponding to rsave.m
 * (for info to some vars (ofile, ifile, vlist) see also rsave.m)
 * 
 * @author Johannes Kissel<johannes.kissel@ipe.fzk.de>
 */

#include "mroot.h"

TMap *mx2root(const mxArray *mxVar);

// standard MEX function signature
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	#if defined PATROL
	__mp_startleaktable();
	#endif
	
	// extract args
	const mxArray *ofile = mxGetCell(prhs[0], 0),
	              *ifile = mxGetCell(prhs[0], 1),
				  *vlist = mxGetCell(prhs[0], 2);
	
	bool append = (bool) mxGetScalar(mxGetField(ifile, 0, "append"));
	mxArray *ipath = mxGetField(ifile, 0, "path");
	
	// open file
	TFile *file;
	
	try {
		file = openFile(ofile, UPDATE);
	} catch (int e) {
		switch (e) {
			case 0: mexMsgTxt(ERR, "Extraction of ofile data failed."); break;
			case 1: mexMsgTxt(ERR, "Error opening file."); break;
			case 2: mexMsgTxt(ERR, "File version not supported."); break;
		}
	}
	
	// go to directory within file
	bool success = cd_seq(ipath, true);
	
	if (!success) {
		delete file;
		mexMsgTxt(ERR, "Failed to go to desired directory in file.");
	}
	
	// empty directory
	if (!append) {
		gDirectory->Delete("*;*");
	}
	
	TList *keys = gDirectory->GetListOfKeys();
	
	// write options: overwrite existing objects and save collections as single key
	int opt = TObject::kOverwrite | TObject::kSingleKey;
	
	// save vars
	int n = mxGetNumberOfElements(vlist);
	
	for (int i = 0; i < n; i++) {
		const mxArray *mxVar = prhs[i + 1];
		
		// extract variable name
		char *name = mxGetStr(mxGetCell(vlist, i));
		
		if (name == NULL) {
			delete file;
			mexMsgTxt(ERR, "Failed to extract string from array.");
		}
		
		// transform mxVar to rootVar
		TMap *rootVar;
		
		try {
			rootVar = mx2root(mxVar);
		} catch (int e) {
			delete file; free(name);
			
			switch (e) {
				case 0: mexMsgTxt(ERR, "Unkown type of variable %s.", name); break;
				case 1: mexMsgTxt(ERR, "Failed to reformat variable %s.", name); break;
				case 2: mexMsgTxt(ERR, "Memory allocation failed for transformation of %s.", name); break;
			}
		} catch (...) {
			delete file; free(name);
			mexMsgTxt(ERR, "Failed to transform variable %s.");
		}
		
		// write rootVar to file
		int res = rootVar->Write(name, opt);
		
		if (res == 0) {
			delete file; free(name);
			mexMsgTxt(ERR, "Failed to write variable %s to file.", name);
		}
		
		#if defined DEBUG
		mexMsgTxt(MSG, "s: %2i, %s\n", mxGetClassID(mxVar), name); // s = save
		#endif
		
		// finalize
		rdelete(rootVar);
		free(name);
	}
	
	delete file;
	
	#if defined PATROL
	__mp_stopleaktable();
	
	__mp_leaktable(0, MP_LT_ALLOCATED, MP_LT_BOTTOM);
	__mp_leaktable(0, MP_LT_FREED,     MP_LT_BOTTOM);
	__mp_leaktable(0, MP_LT_UNFREED,   MP_LT_BOTTOM);
	#endif
	
}

/**
 * Transforms an mxArray recursively to a ROOT map object.
 * @param mxVar The mxArray
 * @return TMap containing data and meta information
 */
TMap* mx2root(const mxArray* mxVar) {
	mxClassID type = mxGetClassID(mxVar);
	
	// check type of variable
	if (type == mxUNKNOWN_CLASS || type == mxOBJECT_CLASS) { throw 0; }
	
	// reformat variable to struct for transformation
	try { mxVar = reformat(mxVar, M2R); } catch (int e) { throw 1; }
	
	// begin transformation to ROOT map object
	TMap *rootVar = new TMap();
	
	// loop over fields
	int n = mxGetNumberOfFields(mxVar);
	
	for (int i = 0; i < n; i++) {
		
		// extract fieldname and field
		const char *name = mxGetFieldNameByNumber(mxVar, i);
		mxArray *field = mxGetFieldByNumber(mxVar, 0, i);
		
		// check type of field
		type = mxGetClassID(field);
		
		// check type of field
		if (type == mxUNKNOWN_CLASS || type == mxOBJECT_CLASS) {
			rdelete(rootVar);
			throw 0;
		}
		
		#if defined DEBUG
		mexMsgTxt(MSG, "t: %2i\n", type); // t = transform
		#endif
		
		// initialize key and value objects
		TObject *key = new TObjString(name), *val = NULL;
		
		// transform fields to ROOT objects (--> val)
		switch (type) {
			case mxCELL_CLASS: {
				int o = mxGetNumberOfElements(field);
				
				TObjArray *tmp = new TObjArray(0);
				
				// loop over cells
				for (int j = 0; j < o; j++) {
					mxArray *c = mxGetCell(field, j);
					tmp->Add(mx2root(c));
				}
				
				val = (TObject*) tmp;
				break;
			}
			case mxSTRUCT_CLASS: { break; } // can't happen after reformat
			case mxLOGICAL_CLASS: {
				bool *p = (bool*) mxGetPr(field);
				int o = mxGetNumberOfElements(field);
				
				TBits *tmp = new TBits(o);
				
				// loop over bits
				for (int j = 0; j < o; j++) {
					tmp->SetBitNumber(j, &p[j]);
				}
				
				val = (TObject*) tmp;
				break;
			}
			case mxCHAR_CLASS: {
				val = mxGetObjStr(field);
				
				if (val == NULL) {
					rdelete(rootVar);
					throw 2;
				}
				
				break;
			}
			case mxDOUBLE_CLASS: {
				val = new TVectorT<double>(mxGetNumberOfElements(field), (double*) mxGetPr(field));
				break;
			}
			case mxSINGLE_CLASS: {
				val = new TVectorT<float>(mxGetNumberOfElements(field), (float*) mxGetPr(field));
				break;
			}
			case mxINT8_CLASS: {
				Char_t *p = (Char_t*) mxGetData(field);
				
				// code duplication; see other int types
				int o = mxGetNumberOfElements(field),
				    s = mxGetElementSize(field) * 8; // 1 Byte = 8 Bit
				
				TBits *tmp = new TBits();
				tmp->Set(o * s, p);
				val = (TObject*) tmp;
				break;
			}
			case mxUINT8_CLASS: {
				UChar_t *p = (UChar_t*) mxGetData(field);
				
				// code duplication; see other int types
				int o = mxGetNumberOfElements(field),
				    s = mxGetElementSize(field) * 8; // 1 Byte = 8 Bit
				
				TBits *tmp = new TBits();
				tmp->Set(o * s, p);
				val = (TObject*) tmp;
				break;
			}
			case mxINT16_CLASS: {
				Short_t *p = (Short_t*) mxGetData(field);
				
				// code duplication; see other int types
				int o = mxGetNumberOfElements(field),
				    s = mxGetElementSize(field) * 8; // 1 Byte = 8 Bit
				
				TBits *tmp = new TBits();
				tmp->Set(o * s, p);
				val = (TObject*) tmp;
				break;
			}
			case mxUINT16_CLASS: {
				UShort_t *p = (UShort_t*) mxGetData(field);
				
				// code duplication; see other int types
				int o = mxGetNumberOfElements(field),
				    s = mxGetElementSize(field) * 8; // 1 Byte = 8 Bit
				
				TBits *tmp = new TBits();
				tmp->Set(o * s, p);
				val = (TObject*) tmp;
				break;
			}
			case mxINT32_CLASS: {
				Int_t *p = (Int_t*) mxGetData(field);
				
				// code duplication; see other int types
				int o = mxGetNumberOfElements(field),
				    s = mxGetElementSize(field) * 8; // 1 Byte = 8 Bit
				
				TBits *tmp = new TBits();
				tmp->Set(o * s, p);
				val = (TObject*) tmp;
				break;
			}
			case mxUINT32_CLASS: {
				UInt_t *p = (UInt_t*) mxGetData(field);
				
				// code duplication; see other int types
				int o = mxGetNumberOfElements(field),
				    s = mxGetElementSize(field) * 8; // 1 Byte = 8 Bit
				
				TBits *tmp = new TBits();
				tmp->Set(o * s, p);
				val = (TObject*) tmp;
				break;
			}
			case mxINT64_CLASS: {
				Long64_t *p = (Long64_t*) mxGetData(field);
				
				// code duplication; see other int types
				int o = mxGetNumberOfElements(field),
				    s = mxGetElementSize(field) * 8; // 1 Byte = 8 Bit
				
				TBits *tmp = new TBits();
				tmp->Set(o * s, p);
				val = (TObject*) tmp;
				break;
			}
			case mxUINT64_CLASS: {
				ULong64_t *p = (ULong64_t*) mxGetData(field);
				
				// code duplication; see other int types
				int o = mxGetNumberOfElements(field),
				    s = mxGetElementSize(field) * 8; // 1 Byte = 8 Bit
				
				TBits *tmp = new TBits();
				tmp->Set(o * s, p);
				val = (TObject*) tmp;
				break;
			}
		}
		
		// add key/value pair to ROOT object
		rootVar->Add(key, val);
		
	}
	
	return rootVar;
}
