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


#include "perffetch.h"
#include "tool/prompt.h"

PerfFetch::PerfFetch(PerfIWin& iwin, PerfBranch& branch, PerfCache& cache) 
					: _iwin(iwin), _branch(branch), _cache(cache){
	
	includeMeta(FALSE);
	includeNOP(FALSE);
	ignoreXfer(FALSE);
	
	_coreiwinSize = DEFAULT_CORE_IWIN_SIZE;	
	_cp1iwinSize = DEFAULT_CP1_IWIN_SIZE;	
	_cp2iwinSize = DEFAULT_CP2_IWIN_SIZE;	
	_instrFetchSize = DEFAULT_FETCH_SPEED;
	_bcFetchSize = DEFAULT_BC_SPEED;
	_fetchSpeed = _instrFetchSize;
	_bufSize = DEFAULT_BUF_SIZE;
	
	_corecnt = 0;
	_cp1cnt = 0;
	_cp2cnt = 0;
	_cp3cnt = 0;
	_cp3chmod = 0;
	_cpXfer = 1;
	_macrocnt = 0;
	_nopcnt = 0;
	_totalcnt = 0;	
	_bccnt = 0;
			
	_perfID = 1;
	_specID = 1;
	
	_curBufSize = 0;
	_curCoreBufSize = 0;
	_curCp1BufSize = 0;	
	_curCp2BufSize = 0;	
	_fileState = 0;
	_fetchStallCyc = 0;
	_startCnt = 0;
	_startPC = 0;
	_endCnt = 0xffffffff;	
}

BOOL PerfFetch::init() {
	WORD raw = 0;
	ADDR pc = 0;
	INT meta = 0;		
	if(_startPC!=0) {
		INT fileState = !EOF;
		BOOL isFind = FALSE;
		while(fileState!=EOF&&isFind==FALSE) {
			fileState = includeMeta()==TRUE?
			   			 fscanf(_streamFP, "%x %x %x", &pc, &raw, &meta):
			  			 fscanf(_streamFP, "%x %x", &pc, &raw);		
			if(pc==_startPC) {
				isFind = TRUE;
			}
		}
		if(fileState==EOF) {
			fprintf(stderr, "Specified PC (%x) cannot be found in the trace\n", _startPC);
			return FALSE;
		}
	}
	if(_startCnt>0) {
		INT fileState = !EOF;
		UINT cnt = 0;
		while(fileState!=EOF&&cnt<_startCnt) {
			fileState = includeMeta()==TRUE?
			   			 fscanf(_streamFP, "%x %x %x", &pc, &raw, &meta):
			  			 fscanf(_streamFP, "%x %x", &pc, &raw);
			cnt++;  			 		
		}
		if(fileState==EOF) {
			fprintf(stderr, "Start insruction count(%u) too big\n", _startCnt);
			return FALSE;
		}
	}	
	_readStream();
	_comBufIter = _comBuf.begin();
	_coreBufIter = _coreBuf.begin();
	_cp1BufIter = _cp1Buf.begin();
	_cp2BufIter = _cp2Buf.begin();
	return TRUE;	
}

BOOL PerfFetch::_isBufNotFull(EINSTR_TYPE type) {
	switch(type) {
		case EI_ls:
		case EI_alu:
		case EI_br:
		case EI_cp3_br:
		case EI_cp3_xfer:
		case EI_cp3_ls:
			return (_curCoreBufSize<_bufSize);
			break;
		case EI_falu:
		case EI_fbr:
			return (_curCp1BufSize<_bufSize);
			break;				
		case EI_fxfer:
		case EI_fls:
			return ((_curCoreBufSize<_bufSize)&&(_curCp1BufSize<_bufSize));
			break;
		case EI_cp2_scalar:
		case EI_cp2_simd:
		case EI_cp2_macro:
		case EI_cp2_misc:
		case EI_cp2_br:
			return (_curCp2BufSize<_bufSize);
			break;
		case EI_cp2_xfer:
		case EI_cp2_ls:
			return ((_curCoreBufSize<_bufSize)&&(_curCp2BufSize<_bufSize));
			break;
		default:
			IsTrue((0), ("Undefined instruction type."));
			break;
	}	
	return FALSE;		
}

INT PerfFetch::_fillWinMips(UINT cyc) {
	if(_curCoreBufSize>0) {
		WORD raw;
		UINT8* pr;
		Decoder& dcd = DecoderUnit::getInstance();
		FetchBufIter coreBufEnd = _coreBuf.end();
		INT i = 0;
		while( _iwin.coreIwinReady()&&_coreBufIter!=coreBufEnd )
		{
			ExpInstr instr;
			raw = _coreBufIter->raw;
			pr = (UINT8 *) &raw;
			dcd.decode(pr, &instr);
			_iwin.add(&instr, _coreBufIter, PROCESSOR_CORE, cyc);
			_coreBufIter++;
			_curCoreBufSize--;
			i++;
		}	
		return i;
	}
	return 0;
}

INT PerfFetch::_fillWinCp1(UINT cyc) {
	if(_curCp1BufSize>0) {	
		WORD raw;
		UINT8* pr;
		Decoder& dcd = DecoderUnit::getInstance();
		FetchBufIter cp1BufEnd = _cp1Buf.end();
		INT i = 0;
		while(_iwin.cp1IwinReady()&&_cp1BufIter!=cp1BufEnd)
		{
			ExpInstr instr;
			raw = _cp1BufIter->raw;
			pr = (UINT8 *) &raw;
			dcd.decode(pr, &instr);
			_iwin.add(&instr, _cp1BufIter, PROCESSOR_CP1, cyc);
			_cp1BufIter++;
			_curCp1BufSize--;
			i++;
		}		
		return i;
	}
	return 0;
}

INT PerfFetch::_fillWinCp2(UINT cyc) {
	if(_curCp2BufSize>0) {	
		WORD raw;
		UINT8* pr;
		Decoder& dcd = DecoderUnit::getInstance();
		FetchBufIter cp2BufEnd = _cp2Buf.end();
		INT i = 0;
		while(_iwin.cp2IwinReady()&&_cp2BufIter!=cp2BufEnd)
		{
			ExpInstr instr;
			raw = _cp2BufIter->raw;
			pr = (UINT8 *) &raw;
			dcd.decode(pr, &instr);
			_iwin.add(&instr, _cp2BufIter, PROCESSOR_CP2, cyc);
			_cp2BufIter++;
			_curCp2BufSize--;
			i++;
		}		
		return i;
	}
	return 0;
}

INT PerfFetch::_fillBuf() {
	UINT fetch_cnt = 0;
	if(_curBufSize>0) {
		Decoder& dcd = DecoderUnit::getInstance();
		FetchBufIter comBufEnd = _comBuf.end();
		#if defined _DEBUG_FETCH
			fprintf(stdout, "Buf Size: %u %u %u %u\n", _curBufSize, _curCoreBufSize, _curCp1BufSize, _curCp2BufSize);
		#endif			
		while(fetch_cnt<fetchSpeed()&&_comBufIter!=comBufEnd&&fetchStallCyc()==0)
		{
			BufItem item;
			BufItem xItem;
			WORD raw;
			UINT8* pr;
			ExpInstr instr;
			EINSTR_TYPE type;
			INT s = 0;
			UINT instr_id = 0;
			ADDR pc = _comBufIter->pc;
			_bccnt+=(pc==0?1:0);
			fetchStallCyc(_cache.checkICache(pc));
			
			if(fetchStallCyc()==0) {
				item.pc = pc;
				item.raw = _comBufIter->raw;
				item.meta = _comBufIter->meta;
				item.xferID = 0;
				item.perfID = _perfID;
				item.specID = _specID;
	
				xItem.pc = pc ;
				xItem.raw = item.raw;
				xItem.meta = item.meta;
				xItem.xferID = 0;
				xItem.specID = _specID;			
	
				raw = item.raw;
				pr = (UINT8 *) &raw;
				s = dcd.decode(pr, &instr);
				type = instr.type();
				instr_id = instr.fmtMap()->id();
				IsTrue((s>0), ("Decoded instruction size = 0\n"));
			}
			
			if(s>0&&_isBufNotFull(type)==TRUE) {
				#if defined _DEBUG_FETCH
					fprintf(stdout, "Instr (%x):", instr_id);
					Prompt::show(stdout, &instr, 0, NULL);
				#endif	
		    	switch (type) {
					case EI_ls:
					case EI_alu: 
						_coreBuf.push_back(item);
			   			_corecnt++; 
			   			_curCoreBufSize++;
						if(_coreBufIter==_coreBuf.end()) {
							_coreBufIter--;
						}			   			
			      		break;
					case EI_falu:
						_cp1Buf.push_back(item);
			      		_cp1cnt++;
			      		_curCp1BufSize++;
			      		if(_cp1BufIter==_cp1Buf.end()) {
							_cp1BufIter--;
						}
			      		break;	      				
					case EI_fxfer:
					case EI_fls:
				    	item.xferID = _xferID(item.perfID);
			      		_corecnt++;
			      		_curCoreBufSize++;	
			   			_coreBuf.push_back(item);	
						if(_coreBufIter==_coreBuf.end()) {
							_coreBufIter--;
						}	
			   			_perfID++;
											
				    	xItem.xferID = item.xferID;
				    	xItem.perfID = _perfID;
			      		_cp1cnt++;
			      		_curCp1BufSize++;
			   			_cp1Buf.push_back(xItem);	
						if(_cp1BufIter==_cp1Buf.end()) {
							_cp1BufIter--;
						}					
						
			      		_cpXfer++;
			      		break;	 
			    	case EI_cp2_scalar:
			    	case EI_cp2_simd:
				    	_cp2Buf.push_back(item);
				    	_cp2cnt++;
				    	_curCp2BufSize++;
						if(_cp2BufIter==_cp2Buf.end()) {
							_cp2BufIter--;
						}			    		
						break;
			    	case EI_cp2_macro:
				    	_cp2Buf.push_back(item);
				    	_cp2cnt++;
			    		_macrocnt++;
				    	_curCp2BufSize++;
						if(_cp2BufIter==_cp2Buf.end()) {
							_cp2BufIter--;
						}		
				    	#ifdef _EXPAND_UCODE
				    		//reserved id for ucode
				    		_perfID += MAX_UCODE_SIZE;
				    	#endif							    		
						break;			    	
			    	case EI_cp2_xfer:
						if(instr.subtype()==EI_cp2_xfer_ext) {
					    	item.xferID = _xferID(item.perfID);
				      		_corecnt++;
				      		_curCoreBufSize++;	
				   			_coreBuf.push_back(item);	
							if(_coreBufIter==_coreBuf.end()) {
								_coreBufIter--;
							}	
				   			_perfID++;	
				   			
					    	xItem.xferID = item.xferID;
					    	xItem.perfID = _perfID;
				      		_cp2cnt++;
				      		_curCp2BufSize++;
				   			_cp2Buf.push_back(xItem);	
							if(_cp2BufIter==_cp2Buf.end()) {
								_cp2BufIter--;
							}					
				      		_cpXfer++;			   					    	
						}
						else {
							_cp2Buf.push_back(item);
							_cp2cnt++;
							_curCp2BufSize++;
							if(_cp2BufIter==_cp2Buf.end()) {
								_cp2BufIter--;
							}
						}
						break;
			    	case EI_cp2_ls:
				    	item.xferID = _xferID(item.perfID);
			      		_corecnt++;
			      		_curCoreBufSize++;	
			   			_coreBuf.push_back(item);	
						if(_coreBufIter==_coreBuf.end()) {
							_coreBufIter--;
						}	
			   			_perfID++;
											
				    	xItem.xferID = item.xferID;
				    	xItem.perfID = _perfID;
				    	#ifdef _EXPAND_UCODE
				    		//reserved id for ucode
				    		_perfID += MAX_UCODE_SIZE;
				    	#endif
			      		_cp2cnt++;
			      		_curCp2BufSize++;
			   			_cp2Buf.push_back(xItem);	
						if(_cp2BufIter==_cp2Buf.end()) {
							_cp2BufIter--;
						}					

			      		_cpXfer++;						
			      		break;
					case EI_cp3_ls:
					case EI_cp3_xfer:
						_coreBuf.push_back(item);
			      		_cp3cnt++;
			      		_curCoreBufSize++;
						if(_coreBufIter==_coreBuf.end()) {
							_coreBufIter--;
						}			      		
			      		break;		
			      	case EI_br:
			      		_specID = _branch.speculatedID(item,instr_id, type);
		      			_coreBuf.push_back(item);
			      		_corecnt++; 
			      		_curCoreBufSize++;
						if(_coreBufIter==_coreBuf.end()) {
							_coreBufIter--;
						}			
			      		break;
			      	case EI_fbr:
			      		_specID = _branch.speculatedID(item, instr_id, type);
						_cp1Buf.push_back(item);
			      		_cp1cnt++;
			      		_curCp1BufSize++;
			      		if(_cp1BufIter==_cp1Buf.end()) {
							_cp1BufIter--;
						}
			      		break;
			      	case EI_cp2_br:
			      		_specID = _branch.speculatedID(item, instr_id, type);
				    	_cp2Buf.push_back(item);
				    	_cp2cnt++;
				    	_curCp2BufSize++;
						if(_cp2BufIter==_cp2Buf.end()) {
							_cp2BufIter--;
						}
			      		break;			      					      		
			      	case EI_cp3_br:
			      		_specID = _branch.speculatedID(item, instr_id, type);
						_coreBuf.push_back(item);
			      		_cp3cnt++;
			      		_curCoreBufSize++;
			      		if((instr.fmtMap()->id()==((UINT16)cp3_chmod_id))) {
			      			_swapSpeed();
			      			DEBUG_FETCH_SPEED("Speed swapped (%d)\n", fetchSpeed());
			      		}
						if(_coreBufIter==_coreBuf.end()) {
							_coreBufIter--;
						}	
			      		break;	      		
			      	default:  
			      		IsTrue((0), ("Undefined instruction type."));
			      		break;
		    	}	
				fetch_cnt++;
				_comBufIter++;
				_perfID++;
		    }
		    else
		    	break;
		}
		_curBufSize -= fetch_cnt;
			
		IsTrue((_curBufSize>=0), ("curBufSize < 0."));
	}
	return fetch_cnt;
}

void PerfFetch::_readStream() {
	WORD raw = 0;
	ADDR pc = 0;
	INT meta = 0;	
	IsTrue((_streamFP!=NULL), ("Instr stream == NULL\n"));
	while(_curBufSize<MAX_PREREAD_BUF) {
		_fileState = includeMeta()==TRUE?
		   			 fscanf(_streamFP, "%x %x %x", &pc, &raw, &meta):
		  			 fscanf(_streamFP, "%x %x", &pc, &raw);
		if(_fileState!=EOF){
			if(!(includeNOP()==FALSE&&raw==0)) {
				BufItem item;
				item.pc = pc;
				item.raw = raw;
				item.meta = meta;
				_comBuf.push_back(item);	
				_curBufSize++;				
			}
			if(raw==0)
				_nopcnt++;
			_totalcnt++;
			if(_totalcnt>=_endCnt) {
				_fileState = EOF;
				//MAX count reached
			}
		}
		if(_fileState==EOF) {
			break;
		}
	}
}

UINT PerfFetch::bufGC(FetchBuf *bufList, FetchBufIter bufIter) {
	UINT old_size = bufList->size();
	if(old_size>0&&bufIter!=bufList->begin()) {
		bufList->erase(bufList->begin(), bufIter);
	}
	return old_size - bufList->size();
}

INT PerfFetch::fillBuf() {
	UINT size = 0;
	if(_curBufSize<(MAX_PREREAD_BUF>>1)&&_fileState!=EOF) {
		bufGC(&_comBuf, _comBufIter);
		bufGC(&_coreBuf, _coreBufIter);
		bufGC(&_cp1Buf, _cp1BufIter);
		bufGC(&_cp2Buf, _cp2BufIter);
		if(_fileState!=EOF)
			_readStream();
	}

	if(_curBufSize>0) {
		if(fetchStallCyc()==0)
			size = _fillBuf();
		else {
			decFetchStallCyc();
		}
	}
	return size;
}
