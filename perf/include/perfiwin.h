/*
 * 	Copyright (c) 2005, 2006 Hong Kong ASTRI Company Limited
 * 	All rights reserved.
 */

/*
	Copyright (C) 2005, 2006.  Free Software Foundation, Inc.

	This program is free software; you can redistribute it and/or modify it
	under the terms of version 2 of the GNU General Public License as
	published by the Free Software Foundation.

	This program is distributed in the hope that it would be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Further, this software is distributed without any warranty that it is
	free of the rightful claim of any third person regarding infringement
	or the like.  Any license provided herein, whether implied or
	otherwise, applies only to this software file.  Patent licenses, if
	any, provided herein do not apply to combinations of this program with
	other software, or any other product whatsoever.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write the Free Software Foundation, Inc., 59
	Temple Place - Suite 330, Boston MA 02111-1307, USA.
*/


#ifndef PERFIWIN_H_
#define PERFIWIN_H_

#include "perfiwinitem.h"
#include "perfbranch.h"
#include "perfdatadep.h"

class PerfIWin {
	private:
		PerfBranch& _branch;
		PerfDataDep& _dataCheck;
		
		IWinItems _coreIwinlist;
		IWinItems _cp1Iwinlist;
		IWinItems _cp2Iwinlist;

		UINT _coreIwinSize;
		UINT _cp1IwinSize;
		UINT _cp2IwinSize;
		
		UINT _coreIwinMaxSize;
		UINT _cp1IwinMaxSize;
		UINT _cp2IwinMaxSize;				
		
		UINT _coreLastFetch;
		BOOL _hasMacro;
		
	private:
		INT _buildMacroList(PerfIWinItem *iwinitem, ExpInstr *instr, FetchBufIter bufIter, EP_PROCESSOR pro, UINT cyc);		
		INT _insertUcode(ExpInstr *instr, FetchBufIter bufIter, EP_PROCESSOR pro, UINT cyc);
		void _updateCore();
		void _updateCp1();
		void _updateCp2();
		UINT _updateMacro(IWinItems& macro_list);
		void _dec_coreIwinSize(UINT i) { _coreIwinSize-=i;}
		void _dec_cp1IwinSize(UINT i) { _cp1IwinSize-=i;}			
		void _dec_cp2IwinSize(UINT i) { _cp2IwinSize-=i;}			
	public:
		PerfIWin(PerfBranch&, PerfDataDep&);
		
		BOOL isIWinEmpty(void) { return (_coreIwinSize==0&&_cp1IwinSize==0&&_cp2IwinSize==0); }
		IWinItems* getCoreIwinlist() { return &_coreIwinlist; }
		IWinItems* getCp1Iwinlist() { return &_cp1Iwinlist; }
		IWinItems* getCp2Iwinlist() { return  &_cp2Iwinlist; }		
		
		void add(ExpInstr *instr, FetchBufIter bufIter, EP_PROCESSOR cp, UINT cyc);
		void remove(IWinItemsIter item, EP_PROCESSOR cp);
		void remove(EP_PROCESSOR cp);
		void pre_update();
		void post_update(UINT, UINT, UINT);
		
		BOOL coreIwinReady() { return (_coreIwinSize<_coreIwinMaxSize);}
		BOOL cp1IwinReady() { return (_cp1IwinSize<_cp1IwinMaxSize);}
		BOOL cp2IwinReady() { return (_cp2IwinSize<_cp2IwinMaxSize);}
		BOOL hasMacro() { return _hasMacro; }

		void freeCoreBuf() { _coreIwinSize--;}
		void freeCp1Buf() { _cp1IwinSize--;}
		void freeCp2Buf() { _cp2IwinSize--;}

		//drink buffer for debug use only
		void drinkBuf() { 
			_coreIwinSize = 0;
			_cp1IwinSize = 0;
			_cp2IwinSize = 0;
		}
		
		UINT coreIwinSize() { return _coreIwinSize;}
		void coreIwinSize(UINT size) {_coreIwinSize = size;}
		UINT cp1IwinSize() { return _cp1IwinSize;}
		void cp1IwinSize(UINT size) { _cp1IwinSize = size;}
		UINT cp2IwinSize() { return _cp2IwinSize;}
		void cp2IwinSize(UINT size) { _cp2IwinSize = size;}

		
		UINT coreIwinMaxSize() { return _coreIwinMaxSize;}
		void coreIwinMaxSize(UINT size) { _coreIwinMaxSize = size;}
		UINT cp1IwinMaxSize() { return _cp1IwinMaxSize;}
		void cp1IwinMaxSize(UINT size) { _cp1IwinMaxSize = size;}
		UINT cp2IwinMaxSize() { return _cp2IwinMaxSize;}
		void cp2IwinMaxSize(UINT size) { _cp2IwinMaxSize = size;}										

		UINT coreLastFetch() { return _coreLastFetch;}
		void coreLastFetch(UINT id) { _coreLastFetch = (_coreLastFetch<id)?id:_coreLastFetch;}	
};


#endif /*PERFPROFILER_H_*/
