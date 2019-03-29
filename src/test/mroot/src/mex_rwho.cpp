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

#include "mroot.h"

bool mxAddCell(mxArray *array, mxArray *cell);

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	#if defined PATROL
	__mp_startleaktable();
	#endif
	
	const mxArray *ofile = mxGetCell(prhs[0], 0),
	              *ifile = mxGetCell(prhs[0], 1);
	
	mxArray *ipath = mxGetField(ifile, 0, "path");
	
	// open file
	TFile *file;
	
	try {
        file = openFile(ofile,READ);
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
	
	// create variable list
	mxArray *vlist = mxCreateCellMatrix(0, 1);
	
	TList *keys = gDirectory->GetListOfKeys();
	TIterator *iter = keys->MakeIterator();
	
	while (TObject *key = iter->Next()) {
		const char *name = key->GetName();
		TObject *rootVar = gDirectory->Get(name);
		
		// add if is mroot variable
		if (rootVar != NULL && rootVar->IsA() == TMap::Class()
				&& ((TMap*) rootVar)->GetValue("type") != NULL) {
			bool success = mxAddCell(vlist, mxCreateString(name));
			
			if (!success) {
				delete iter; delete file;
				mexMsgTxt(ERR, "Memory allocation failed.");
			}
		}
	}
	
	delete iter;
	delete file;
	
	plhs[0] = vlist;
	
	#if defined PATROL
	__mp_stopleaktable();
	
	__mp_leaktable(0, MP_LT_ALLOCATED, MP_LT_BOTTOM);
	__mp_leaktable(0, MP_LT_FREED,     MP_LT_BOTTOM);
	__mp_leaktable(0, MP_LT_UNFREED,   MP_LT_BOTTOM);
	#endif
	
}

/**
 * Add a cell to a m*1 dimensional cell array.
 * @param array Cell array to expand
 * @param cell Cell to add
 * @return true on success
 */
bool mxAddCell(mxArray *array, mxArray *cell) {
    size_t n  = mxGetNumberOfElements(array);
	
    size_t mem = (n + 1) * mxGetElementSize(array);
    size_t dims[] = {n + 1, 1};
	
	double *p = mxGetPr(array);
	p = (double*) mxRealloc(p, mem);
	if (p == NULL) return false;
	
	mxSetPr(array, p);
	mxSetDimensions(array, dims, 2);
	mxSetCell(array, n, cell);
	
	return true;
}
