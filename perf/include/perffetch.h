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


#ifndef PERFFETCH_H_
#define PERFFETCH_H_

#include "perfdefs.h"
#include "perfiwin.h"
#include "perfbranch.h"
#include "perfcache.h"
#include "decode.h"
#include <stdio.h>
#include <vector>

#if _DEBUG_FETCH_SPEED
	#define DEBUG_FETCH_SPEED printf
#else
	#define DEBUG_FETCH_SPEED
#endif	

class PerfFetch {
	private:
		PerfIWin& _iwin;
		PerfBranch& _branch;
		PerfCache& _cache;
	
		UINT _bufSize;
		UINT _instrFetchSize;
		UINT _bcFetchSize;
		UINT _fetchSpeed;
		BOOL _includeMeta;
		BOOL _includeNOP;
		BOOL _ignoreXfer;
		UINT _coreiwinSize;	
		UINT _cp1iwinSize;	
		UINT _cp2iwinSize;	
		UINT _startCnt;
		ADDR _startPC;
		UINT _endCnt;
				
		FILE *_streamFP;
		UINT _perfID;
		UINT _specID;
		UINT _curBufSize;
		UINT _curCoreBufSize;
		UINT _curCp1BufSize;
		UINT _curCp2BufSize;
		INT _fileState;
		UINT _fetchStallCyc;

		FetchBuf _coreBuf;
		FetchBuf _cp1Buf;
		FetchBuf _cp2Buf;
		FetchBuf _comBuf;
		FetchBufIter _coreBufIter;
		FetchBufIter _cp1BufIter;
		FetchBufIter _cp2BufIter;
		FetchBufIter _comBufIter;
		
		UINT _corecnt;
		UINT _cp1cnt;
		UINT _cp2cnt;
		UINT _cp3cnt;
		UINT _cp3chmod;
		UINT _cpXfer;
		UINT _macrocnt;
		UINT _nopcnt;
		UINT _totalcnt;		
		UINT _bccnt;
		
	private:
		void _readStream();
		//INT _fillBuf(UINT size, BranchUnit* branch);	
		BOOL _isBufNotFull(EINSTR_TYPE type);
		INT _fillBuf();
		INT _fillWinMips(UINT);
		INT _fillWinCp1(UINT);
		INT _fillWinCp2(UINT);		
		void _bufSizeProfiling();
		void _swapSpeed() {
			fetchSpeed( (fetchSpeed()==instrFetchSize())?bcFetchSize():instrFetchSize() );
		}
		UINT _xferID(UINT id) { 
			return (ignoreXfer()==TRUE?0:id); 
		}
				  	
	public:
		PerfFetch(PerfIWin& iwin, PerfBranch& branch, PerfCache& cache);
		BOOL init();
		UINT bufGC(FetchBuf *bufList, FetchBufIter bufIter);
		INT fillBuf();
		void fillWin(UINT cyc) {
			_fillWinMips(cyc);
			_fillWinCp1(cyc);
			_fillWinCp2(cyc);				
		}

		void drinkBuf() {
			_curCoreBufSize = 0;
			_curCp1BufSize = 0;
			_curCp2BufSize = 0;			
		} //for fetch debug only
				
		BOOL isBufEmpty(void) { 
			return (_curBufSize==0&&_curCoreBufSize==0&&_curCp1BufSize==0&&_curCp2BufSize==0); 
		}
		
		//Wrapper
		BOOL includeMeta(void) { return _includeMeta; }
		void includeMeta(BOOL t) { _includeMeta = t; }
		BOOL includeNOP(void) { return _includeNOP; }
		void includeNOP(BOOL t) { _includeNOP = t;	}
		BOOL ignoreXfer(void) { return _ignoreXfer; }
		void ignoreXfer(BOOL t) { _ignoreXfer = t;	}		
		UINT fetchStallCyc(void) { return _fetchStallCyc; }
		void fetchStallCyc(UINT c) { _fetchStallCyc = c;	}	
		void decFetchStallCyc() { _fetchStallCyc--;	}	

		void streamFP(FILE* f) { _streamFP = f;	}		
		UINT bufSize(void) { return _bufSize; }
		void bufSize(UINT s) { _bufSize = s; }
		UINT curCoreBufSize(void) { return _curCoreBufSize; }
		UINT curCp1BufSize(void) { return _curCp1BufSize; }
		UINT curCp2BufSize(void) { return _curCp2BufSize; }	
		UINT instrFetchSize(void) { return _instrFetchSize; }
		void instrFetchSize(UINT i) { 
				_instrFetchSize = i; 
				_fetchSpeed = i;
			}
		UINT bcFetchSize(void) { return _bcFetchSize; }
		void bcFetchSize(UINT i) { _bcFetchSize = i; }
		UINT fetchSpeed(void) { return _fetchSpeed; }
		void fetchSpeed(UINT s) { _fetchSpeed = s; }		
		UINT coreiwinSize(void) { return _coreiwinSize; }
		UINT cp1iwinSize(void) { return _cp1iwinSize; }
		UINT cp2iwinSize(void) { return _cp2iwinSize; }
		UINT corecnt(void) { return _corecnt; }
		UINT cp1cnt(void) { return _cp1cnt; }
		UINT cp2cnt(void) { return _cp2cnt; }
		UINT cp3cnt(void) { return _cp3cnt; }
		UINT cpXfer(void) { return (_cpXfer-1); }
		UINT macrocnt(void) { return _macrocnt; }
		UINT nopcnt(void) { return _nopcnt; }
		UINT totalcnt(void) { return _totalcnt; }		
		UINT bccnt(void) { return _bccnt; }	
		UINT startCnt(void) { return _startCnt; }	
		void startCnt(UINT c) { _startCnt = c; }
		ADDR startPC(void) { return _startPC; }	
		void startPC(ADDR a) { _startPC = a; }
		UINT endCnt(void) { return _endCnt; }	
		void endCnt(UINT c) { _endCnt = c; }		
		FetchBufIter getCoreBufEnd(void) { return _coreBuf.end(); }
		FetchBufIter getCp1BufEnd(void) { return _cp1Buf.end(); }
		FetchBufIter getCp2BufEnd(void) { return _cp2Buf.end(); }
		FetchBufIter getCoreBufCur(void) { return _coreBufIter; }
		FetchBufIter getCp1BufCur(void) { return _cp1BufIter; }
		FetchBufIter getCp2BufCur(void) { return _cp2BufIter; }
};

#endif /*PERFFETCH_H_*/
