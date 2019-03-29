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
 * MEX code corresponding to rload.m
 * (for info to some vars (ofile, ifile, vlist) see also in rload.m)
 * 
 * @author Johannes Kissel<johannes.kissel@ipe.fzk.de>
 */

#include "mroot.h"


mxArray *root2mx(TObject *rootVar, mxClassID type);
TFile *openFileRaw(const char* a_path, Open_t a_mode);

// standard MEX function signature
void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
	
	#if defined PATROL
	__mp_startleaktable();
	#endif
	
	// extract args
	const mxArray *ofile = mxGetCell(prhs[0], 0),
	              *ifile = mxGetCell(prhs[0], 1),
				  *vlist = mxGetCell(prhs[0], 2);
	
	mxArray *ipath = mxGetField(ifile, 0, "path");
	
	// open file
	TFile *file;
	
	try {
        file = openFile(ofile,READ);
        //file = openFileRaw(TEST_FILE_NAME,READ);
	} catch (int e) {
		switch (e) {
			case 0: mexMsgTxt(ERR, "Extraction of ofile data failed."); break;
			case 1: mexMsgTxt(ERR, "Error opening file."); break;
			case 2: mexMsgTxt(ERR, "File version not supported."); break;
		}
	}
	
	// go to directory within file
	bool success = cd_seq(ipath, false);
	
	if (!success) {
		delete file;
		mexMsgTxt(ERR, "Failed to go to desired directory in file.");
	}
	
	TList *keys = gDirectory->GetListOfKeys();
	
	// load variables
	int n = mxGetNumberOfElements(vlist);
	
	const char **vname = (const char**) malloc(n * sizeof(void*));
	mxArray **vdata = (mxArray**) malloc(n * sizeof(void*));
	
	if (vname == NULL || vdata == NULL) { mexMsgTxt(ERR, "Memory allocation failed."); }
	
	for (int i = 0; i < n; i++) {
		
		// extract variable name
		char *name = mxGetStr(mxGetCell(vlist, i));
		
		if (name == NULL) {
			delete file; free(vname); free(vdata);
			mexMsgTxt(ERR, "Failed to extract variable name.");
		}
		
		// load rootVar from file
		TObject *rootVar = gDirectory->Get(name);
		
		if (rootVar == NULL) {
			delete file; free(vname); free(vdata);
			mexMsgTxt(ERR, "Variable %s not found.", name);
		}
		
		// check type of var
		if (rootVar->IsA() != TMap::Class() || ((TMap*) rootVar)->GetValue("type") == NULL) {
			delete file; free(vname); free(vdata);
			mexMsgTxt(ERR, "Unknown type of variable %s.", name);
		}
		
		// transform rootVar to mxVar
		mxArray *mxVar;
		
		try {
			mxVar = root2mx(rootVar, mxSTRUCT_CLASS);
		} catch (int e) {
			delete file; free(vname); free(vdata);
			
			switch (e) {
				case 0: mexMsgTxt(ERR, "Unkown type of variable %s.", name); break;
				case 1: mexMsgTxt(ERR, "Failed to reformat variable %s.", name); break;
				case 2: mexMsgTxt(ERR, "Memory allocation failed for transformation of %s.", name); break;
			}
		}
		
		// buffer variable name and variable
		vname[i] = name;
		vdata[i] = mxVar;
		
		#if defined DEBUG
		mexMsgTxt(MSG, "l: %2i, %s\n", mxGetClassID(mxVar), name); // l = load
		#endif
		
	}
	
	// create result struct
	mxArray *vars = mxCreateStructMatrix(1, 1, n, vname);
	
	for (int i = 0; i < n; i++) {
		mxSetFieldByNumber(vars, 0, i, vdata[i]);
		free((char*) vname[i]);
	}
	
	plhs[0] = vars;
	
	// finalize
	delete file; free(vname); free(vdata);
	
	#if defined PATROL
	__mp_stopleaktable();
	
	__mp_leaktable(0, MP_LT_ALLOCATED, MP_LT_BOTTOM);
	__mp_leaktable(0, MP_LT_FREED,     MP_LT_BOTTOM);
	__mp_leaktable(0, MP_LT_UNFREED,   MP_LT_BOTTOM);
	#endif
	
}

/**
 * Get a ROOT variables class ID.
 * 
 * Integer matrixes are handled as logical arrays,
 * because they are also stored in a TBits container.
 * 
 * @param rootVar The ROOT var
 * @return An mxClassID
 */
mxClassID rootGetClassID(TObject *rootVar) {
	mxClassID id = mxUNKNOWN_CLASS;
	
	if        (rootVar->IsA() == TObjArray::Class()) {
		id = mxCELL_CLASS;
	} else if (rootVar->IsA() == TMap::Class()) {
		id = mxSTRUCT_CLASS;
	} else if (rootVar->IsA() == TBits::Class()) {
		id = mxLOGICAL_CLASS;
	} else if (rootVar->IsA() == TObjString::Class()) {
		id = mxCHAR_CLASS;
	} else if (rootVar->IsA() == TVectorT<double>::Class()) {
		id = mxDOUBLE_CLASS;
	} else if (rootVar->IsA() == TVectorT<float>::Class()) {
		id = mxSINGLE_CLASS;
	}
	
	return id;
}

mxArray *root2mx(TObject *rootVar, mxClassID type) {
	// check type of variable
	if (type == mxUNKNOWN_CLASS) { type = rootGetClassID(rootVar); }
	if (type == mxUNKNOWN_CLASS) { throw 0; }
	
	#if defined DEBUG
	mexMsgTxt(MSG, "t: %2i\n", type); // t = transform
	#endif
	
	mxArray *mxVar = NULL;
	
	// transform to variable
	switch (type) {
		case mxCELL_CLASS: {
			TObjArray *tmp = (TObjArray*) rootVar;
			TIterator *iter = tmp->MakeIterator();
			int n = tmp->GetLast() + 1, i = 0;
			
			mxVar = mxCreateCellMatrix(1, n);
			
			// loop over cells
			while (TObject *cell = iter->Next()) {
				mxSetCell(mxVar, i, root2mx(cell, mxUNKNOWN_CLASS));
				i++;
			}
			
			break;
		}
		case mxSTRUCT_CLASS: {
			TMap *tmp = (TMap*) rootVar;
			TMapIter *iter = new TMapIter(tmp);
			int n = tmp->GetSize(), i = 0;
			
			type = (mxClassID) *((TVectorT<double>*) tmp->GetValue("type"))->GetMatrixArray();
			
			const char **names = (const char**) malloc(n * sizeof(void*));
			mxArray **fields = (mxArray**) malloc(n * sizeof(void*));
			
			if (names == NULL || fields == NULL) { throw 2; }
			
			// loop over fields
			while (TObject *key = iter->Next()) {
				TObject *val = tmp->GetValue(key);
				
				names[i]  = ((TObjString*) key)->GetName();
				fields[i] = ((!strcmp(names[i], "data")
				           || !strcmp(names[i], "real")
						   || !strcmp(names[i], "imag"))
 						   && type > 7)
					? root2mx(val, type) // data is int array
					: root2mx(val, mxUNKNOWN_CLASS);
				
				i++;
			}
			
			mxVar = mxCreateStructMatrix(1, 1, n, names);
			
			for (i = 0; i < n; i++) {
				mxSetFieldByNumber(mxVar, 0, i, fields[i]);
			}
			
			delete iter; free(names); free(fields);
			
			// reformat to variable
			mxVar = (mxArray*) reformat(mxVar, R2M);
			break;
		}
		case mxLOGICAL_CLASS: {
			TBits *tmp = (TBits*) rootVar;
			int n = tmp->GetNbits();
			
			mxVar = mxCreateLogicalMatrix(1, n);
			mxLogical *p = mxGetLogicals(mxVar);
			
			// loop over bits
			for (int i = 0; i < n; i++) {
				p[i] = tmp->TestBitNumber(i);
			}
			
			break;
		}
		case mxCHAR_CLASS: {
			TObjString *tmp = (TObjString*) rootVar;
			mxVar = mxCreateString(tmp->GetName());
			break;
		}
		case mxDOUBLE_CLASS: {
			TVectorT<double> *tmp = (TVectorT<double>*) rootVar;
			int n = tmp->GetNrows();
			
			mxVar = mxCreateNumericMatrix(1, n, mxDOUBLE_CLASS, mxREAL);
			memcpy(mxGetPr(mxVar), tmp->GetMatrixArray(), n * sizeof(double));
			break;
		}
		case mxSINGLE_CLASS: {
			TVectorT<float> *tmp = (TVectorT<float>*) rootVar;
			int n = tmp->GetNrows();
			
			mxVar = mxCreateNumericMatrix(1, n, mxSINGLE_CLASS, mxREAL);
			memcpy(mxGetPr(mxVar), tmp->GetMatrixArray(), n * sizeof(float));
			break;
		}
		case mxINT8_CLASS: {
			TBits *tmp = (TBits*) rootVar;
			int n = tmp->GetNbytes() / sizeof(Char_t);
			
			mxVar = mxCreateNumericMatrix(1, n, mxINT8_CLASS, mxREAL);
			tmp->Get((Char_t*) mxGetData(mxVar));
			break;
		}
		case mxUINT8_CLASS: {
			TBits *tmp = (TBits*) rootVar;
			int n = tmp->GetNbytes() / sizeof(UChar_t);
			
			mxVar = mxCreateNumericMatrix(1, n, mxUINT8_CLASS, mxREAL);
			tmp->Get((UChar_t*) mxGetData(mxVar));
			break;
		}
		case mxINT16_CLASS: {
			TBits *tmp = (TBits*) rootVar;
			int n = tmp->GetNbytes() / sizeof(Short_t);
			
			mxVar = mxCreateNumericMatrix(1, n, mxINT16_CLASS, mxREAL);
			tmp->Get((Short_t*) mxGetData(mxVar));
			break;
		}
		case mxUINT16_CLASS: {
			TBits *tmp = (TBits*) rootVar;
			int n = tmp->GetNbytes() / sizeof(UShort_t);
			
			mxVar = mxCreateNumericMatrix(1, n, mxUINT16_CLASS, mxREAL);
			tmp->Get((UShort_t*) mxGetData(mxVar));
			break;
		}
		case mxINT32_CLASS: {
			TBits *tmp = (TBits*) rootVar;
			int n = tmp->GetNbytes() / sizeof(Int_t);
			
			mxVar = mxCreateNumericMatrix(1, n, mxINT32_CLASS, mxREAL);
			tmp->Get((Int_t*) mxGetData(mxVar));
			break;
		}
		case mxUINT32_CLASS: {
			TBits *tmp = (TBits*) rootVar;
			int n = tmp->GetNbytes() / sizeof(UInt_t);
			
			mxVar = mxCreateNumericMatrix(1, n, mxUINT32_CLASS, mxREAL);
			tmp->Get((UInt_t*) mxGetData(mxVar));
			break;
		}
		case mxINT64_CLASS: {
			TBits *tmp = (TBits*) rootVar;
			int n = tmp->GetNbytes() / sizeof(Long64_t);
			
			mxVar = mxCreateNumericMatrix(1, n, mxINT64_CLASS, mxREAL);
			tmp->Get((Long64_t*) mxGetData(mxVar));
			break;
		}
		case mxUINT64_CLASS: {
			TBits *tmp = (TBits*) rootVar;
			int n = tmp->GetNbytes() / sizeof(ULong64_t);
			
			mxVar = mxCreateNumericMatrix(1, n, mxUINT64_CLASS, mxREAL);
			tmp->Get((ULong64_t*) mxGetData(mxVar));
			break;
		}
	}
	
	return mxVar;
}
