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


#include "perfdispatch.h"
#include "perfprofiler.h"

PerfDispatch::PerfDispatch( PerfIWin& iwin,
							PerfDataDep& dataCheck,
							PerfBranch& branch,
							PerfEngine& engine,
							PerfOrder& order)	: 	
							_iwin(iwin) ,
							_dataCheck(dataCheck),
							_branch(branch),	
							_engine(engine),
							_order(order)
{

	_totalIssued = 0;
	_coreCurIssued = 0;
	_cp1CurIssued = 0;
	_cp2CurIssued = 0;	
}

BOOL PerfDispatch::_issueXfer(UINT xferID) {
	if(xferID>_iwin.coreLastFetch()) {
		return FALSE;
	}
	else {
		IWinItems* iwinlist = _iwin.getCoreIwinlist();;
		IWinItemsIter iwin_iter = iwinlist->begin();;
		IWinItemsIter iwin_end_iter = iwinlist->end();
		ES_HAZARD_STATUS status = HS_NONE;
		EU_CPU_UNIT unit;
		
		while(iwin_iter != iwin_end_iter) {
			_DEBUG_IWIN_ITER
			if(iwin_iter->status()==IW_NOT_READY) {
				status = _isValid(iwin_iter, PROCESSOR_CORE, &unit); 
				if(iwin_iter->perf_id()==xferID) {
					_dataCheck.xferReset();
					_order.xferReset();
					if(status==HS_NONE) {
						_lockResource(iwin_iter, unit, PROCESSOR_CORE);
						iwin_iter->status(IW_XFER_READY);
						_coreCurIssued++;
						return TRUE;
					}
					return FALSE;
				}
			}
			iwin_iter++;
		}
	}
	return FALSE;
}

void PerfDispatch::_issueCore() {
	IWinItems* iwinlist = _iwin.getCoreIwinlist();;
	IWinItemsIter iwin_iter = iwinlist->begin();;
	IWinItemsIter iwin_end_iter = iwinlist->end();
	BOOL stop = FALSE;
	ES_HAZARD_STATUS status = HS_NONE;
	EU_CPU_UNIT unit;

	while(iwin_iter != iwin_end_iter&&stop!=TRUE) {
		_DEBUG_IWIN_ITER
		if(iwin_iter->status()==IW_NOT_READY) {
			status = _isValid(iwin_iter, PROCESSOR_CORE, &unit); 
			if(status==HS_NONE) {
				if(iwin_iter->xfer_id()==0) {
					_lockResource(iwin_iter, unit, PROCESSOR_CORE);
					iwin_iter->status(IW_READY);
					_coreCurIssued++;
				}
			}
			else if(status==HS_ENGINE_FULL||status==HS_BR_STALL){
				stop = TRUE;
			}
		}
		else if(iwin_iter->status()==IW_XFER_READY) {
			status = _isValid(iwin_iter, PROCESSOR_CORE, &unit); 
			IsTrue((status!=HS_NONE), ("Invalid Xfer status"));
			iwin_iter->status(IW_READY);
			_dataCheck.updateCoreXferReg(iwin_iter);
		}		
		iwin_iter++;
	}	
}

void PerfDispatch::_issueCp1() {
	IWinItems* iwinlist = _iwin.getCp1Iwinlist();;
	IWinItemsIter iwin_iter = iwinlist->begin();;
	IWinItemsIter iwin_end_iter = iwinlist->end();
	ES_HAZARD_STATUS status = HS_NONE;
	EU_CPU_UNIT unit;
	//Core
	while(iwin_iter != iwin_end_iter&&(_cp1CurIssued==0&&status==HS_NONE)) {
		_DEBUG_IWIN_ITER
		if(iwin_iter->status()==IW_NOT_READY) {
			status = _isValid(iwin_iter, PROCESSOR_CP1, &unit); 
			if(status==HS_NONE) {
				UINT xferID = iwin_iter->xfer_id();
				if(xferID==0||_issueXfer(xferID)==TRUE) {
					_lockResource(iwin_iter, unit, PROCESSOR_CP1);
					iwin_iter->status(IW_READY);
					_cp1CurIssued++;
				}
			}
		}
		iwin_iter++;
	}	
}

void PerfDispatch::_issueCp2Macro() {
	IWinItems* iwinlist = _iwin.getCp2Iwinlist();
	IWinItemsIter iwin_iter = iwinlist->begin();
	IWinItemsIter iwin_end_iter = iwinlist->end();
	BOOL isDone = FALSE;

	while(iwin_iter!=iwin_end_iter) {
		_DEBUG_IWIN_ITER
		if(iwin_iter->type()==ISSUE_TYPE_MACRO) {
			IWinItemsIter ucode_iter = iwin_iter->macroInstr.begin();
			IWinItemsIter ucode_iter_end = iwin_iter->macroInstr.end();
			while(isDone==FALSE&&ucode_iter!=ucode_iter_end) {
				if(ucode_iter->status()==IW_NOT_READY) {
					EU_CPU_UNIT unit = _engine.checkReadyUnit(ucode_iter->unit(), ucode_iter->instr_id(), PROCESSOR_CP2);
					if(unit!=UNIT_PROCESSOR_FULL&&unit!=UNIT_NOT_READY) {
						_engine.lockEngine(unit, PROCESSOR_CP2);
						ucode_iter->status(IW_READY);
						_cp2CurIssued++;
					}
					isDone = TRUE;
				}
				ucode_iter++;
			}
		}
		iwin_iter++;
	}
}

void PerfDispatch::_issueCp2() {
	IWinItems* iwinlist = _iwin.getCp2Iwinlist();
	IWinItemsIter iwin_iter = iwinlist->begin();
	IWinItemsIter iwin_end_iter = iwinlist->end();
	BOOL stop = FALSE;
	ES_HAZARD_STATUS status = HS_NONE;
	EU_CPU_UNIT unit;

	while(iwin_iter != iwin_end_iter&&stop!=TRUE) {
		_DEBUG_IWIN_ITER
		if(iwin_iter->status()==IW_NOT_READY) {
			status = _isValid(iwin_iter, PROCESSOR_CP2, &unit); 
			if(status==HS_NONE) {
				UINT xferID = iwin_iter->xfer_id();
				if(xferID==0||_issueXfer(xferID)==TRUE) {
					_lockResource(iwin_iter, unit, PROCESSOR_CP2);
					iwin_iter->status(IW_READY);
					_cp2CurIssued++;
				}
			}
			else if(status==HS_ENGINE_FULL||status==HS_BR_STALL){
				stop = TRUE;
			}
		}
		iwin_iter++;
	}	
}

void PerfDispatch::dispatch(UINT cycle) {
	_coreCurIssued = 0;
	_cp1CurIssued = 0;
	_cp2CurIssued = 0;	
	_iwin.pre_update();
	PROFILER_DEBUG_IWIN(cycle, _iwin)
	PROFILER_CHECK_IWIN_FULL(_iwin)
	_engine.reset();
	_dataCheck.reset();
	_order.reset();
	#ifndef _EXPAND_UCODE
		if(_iwin.hasMacro()==TRUE) {
			_issueCp2Macro();
		}
	#endif
	_issueCp2();
	_issueCp1();
	_issueCore();
	_iwin.post_update(_coreCurIssued, _cp1CurIssued, _cp2CurIssued);
	PROFILER_CHECK_ISSUE(_iwin, _coreCurIssued, _cp2CurIssued)
}

void PerfDispatch::update() {
}
