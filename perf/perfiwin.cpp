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


#include "perfiwin.h"
#include "decode.h"
#include "perfprofiler.h"

PerfIWin::PerfIWin(PerfBranch& branch, PerfDataDep& dataCheck): _branch(branch), _dataCheck(dataCheck) {
		_coreIwinMaxSize = DEFAULT_CORE_IWIN_SIZE;
		_cp1IwinMaxSize = DEFAULT_CP1_IWIN_SIZE;
		_cp2IwinMaxSize = DEFAULT_CP2_IWIN_SIZE;
		_coreIwinSize = 0;
		_cp1IwinSize = 0;
		_cp2IwinSize = 0;		
		
		_coreLastFetch = 0;
		_hasMacro = FALSE;
}

void PerfIWin::add(ExpInstr *instr, FetchBufIter bufIter, EP_PROCESSOR cp, UINT cyc) {
	PerfIWinItem item(instr, bufIter, cp, cyc);
	
	//Check branch
	_branch.branchLatency(&item);
	
	switch (cp) {
		case PROCESSOR_CORE:
			_coreIwinlist.push_back(item);
			coreLastFetch(item.perf_id());
			_coreIwinSize++;
			break;
		case PROCESSOR_CP1:
			_cp1Iwinlist.push_back(item);
			_cp1IwinSize++;
			break;		
		case PROCESSOR_CP2:
			#ifdef _EXPAND_UCODE
				if(item.type()==ISSUE_TYPE_MACRO) {
					INT cycle = _insertUcode(instr, bufIter, PROCESSOR_CP2_UCODE, cyc);
					IsTrue((cycle>0), ("Null macro list.\n"));		
				}
				else {
					_cp2Iwinlist.push_back(item);
				}
			#else
				if(item.type()==ISSUE_TYPE_MACRO) {
					INT cycle = _buildMacroList(&item, instr, bufIter, PROCESSOR_CP2_UCODE, cyc);
					IsTrue((cycle>0), ("Null macro list.\n"));	
					item.unit(item.macroInstr.begin()->unit());
				}			
				_cp2Iwinlist.push_back(item);
			#endif
			_cp2IwinSize++;
			break;
		default:
			IsTrue((0), ("Undefined processor.\n"));		
	}
}

void PerfIWin::remove(IWinItemsIter item, EP_PROCESSOR cp) {
	switch (cp) {
		case PROCESSOR_CORE:
			_coreIwinlist.erase(item);
			break;
		case PROCESSOR_CP1:
			_cp1Iwinlist.erase(item);
			break;		
		case PROCESSOR_CP2:
			_cp2Iwinlist.erase(item);
			break;
		default:
			IsTrue((0), ("Undefined processor.\n"));		
	}	
}

void PerfIWin::remove(EP_PROCESSOR cp) {
	switch (cp) {
		case PROCESSOR_CORE:
			_coreIwinlist.pop_front();
			break;
		case PROCESSOR_CP1:
			_cp1Iwinlist.pop_front();
			break;		
		case PROCESSOR_CP2:
			_cp2Iwinlist.pop_front();
			break;
		default:
			IsTrue((0), ("Undefined processor.\n"));		
	}	
}

INT PerfIWin::_insertUcode(ExpInstr *instr, FetchBufIter bufIter, EP_PROCESSOR pro, UINT cyc) {
	if(pro==PROCESSOR_CP2_UCODE) {
		InstrList instrList;
		instr->macroCnt(bufIter->meta);
		INT cycle = DecoderUnit::getInstance().getMacro(instr, &instrList);
		UINT sub_id = 0;
		if(cycle>0) {
			InstrListIter listiter = instrList.begin();
			while(listiter!=instrList.end()) {
				ExpInstr i = *listiter;
				PerfIWinItem item(&i, bufIter, pro, cyc);
				item.sub_id(sub_id);
				item.perf_id(item.perf_id()+sub_id);
				sub_id++;
				if(sub_id==1) {
					item.info(INSTR_INFO_MACRO_START);
				}
				else {
					item.xfer_id(0);
					if(sub_id==instrList.size()) {
						item.info(INSTR_INFO_MACRO_END);
					}
				}
				_cp2Iwinlist.push_back(item);
				listiter++;
			}
		}
		return cycle;
	}
	else {
		IsTrue((0), ("No macro instruction in this processor.\n"));
	}
	return 0;
}

INT PerfIWin::_buildMacroList(PerfIWinItem *iwinitem, ExpInstr *instr, FetchBufIter bufIter, EP_PROCESSOR pro, UINT cyc) {
	if(pro==PROCESSOR_CP2_UCODE) {
		InstrList instrList;
		instr->macroCnt(bufIter->meta);
		INT cycle = DecoderUnit::getInstance().getMacro(instr, &instrList);
		if(cycle>0) {
			InstrListIter listiter = instrList.begin();
			while(listiter!=instrList.end()) {
				ExpInstr i = *listiter;
				PerfIWinItem item(&i, bufIter, pro, cyc);
				iwinitem->macroInstr.push_back(item);
				listiter++;
			}
			IWinItemsIter ucode_ter = iwinitem->macroInstr.begin();
			ucode_ter->status(IW_READY);
			iwinitem->unit(ucode_ter->unit());
			
		}
		return cycle;
	}
	else {
		IsTrue((0), ("No macro instruction in this processor.\n"));
	}
	return 0;
}

void PerfIWin::_updateCore() {
	IWinItemsIter iwin_iter = _coreIwinlist.begin();
	IWinItemsIter iwin_end_iter = _coreIwinlist.end();
	while (iwin_iter != iwin_end_iter) {
		PerfIWinItem item = *iwin_iter;
		if(iwin_iter->status()==IW_COMPLETED) {
			//this instr is complete, remove it from the list
			IWinItemsIter tmp_iter = iwin_iter;
			iwin_iter++;
			_coreIwinlist.erase(tmp_iter);			
		}
		else {
			if(iwin_iter->status()==IW_READY||iwin_iter->status()==IW_XFER_READY) {
				PROFILER_LOG_CORE_ISSUED(iwin_iter)
				iwin_iter->status(IW_ISSUED);
				_coreIwinSize--;
				PROFILER_CHECK_WRITE_ORDER(iwin_iter)
				IsTrue((iwin_iter->latency()>0), ("Instruction latency = 0"));
			}
			if(iwin_iter->status()==IW_ISSUED) {
				iwin_iter->dec_latency();
				if(iwin_iter->latency()==0)	{
					iwin_iter->status(IW_COMPLETED);
					_branch.lastBrID(iwin_iter);					
				}	
			}
			iwin_iter ++;
		}
	}
}

void PerfIWin::_updateCp1() {
	IWinItemsIter iwin_iter = _cp1Iwinlist.begin();
	IWinItemsIter iwin_end_iter = _cp1Iwinlist.end();
	while (iwin_iter != iwin_end_iter) {
		if(iwin_iter->status()==IW_COMPLETED) {
			//this instr is complete, remove it from the list
			IWinItemsIter tmp_iter = iwin_iter;
			iwin_iter++;
			_cp1Iwinlist.erase(tmp_iter);			
		}		
		else {
			if(iwin_iter->status()==IW_READY) {
				PROFILER_LOG_CP1_ISSUED(iwin_iter)
				iwin_iter->status(IW_ISSUED);
				//_cp1IwinSize--; 
				//CP1 iwin size update immediately after issue
				PROFILER_CHECK_WRITE_ORDER(iwin_iter)
				IsTrue((iwin_iter->latency()>0), ("Instruction latency = 0"));
			}
			if(iwin_iter->status()==IW_ISSUED) {
				iwin_iter->dec_latency();
				if(iwin_iter->latency()==0) {	
					iwin_iter->status(IW_COMPLETED);			
					_branch.lastBrID(iwin_iter);
				}	
			}
			iwin_iter ++;	
		}
	}	
}

UINT PerfIWin::_updateMacro(IWinItems &macro_list) {
	IWinItemsIter iwin_iter = macro_list.begin();
	IWinItemsIter iwin_end_iter = macro_list.end();
	BOOL isDone = FALSE;
	UINT list_size = macro_list.size();
//	if(iwin_iter->raw()==0x4b3050a0) {
//		printf("Got\n");
//	}
	while (iwin_iter != iwin_end_iter && isDone==FALSE) {
		_DEBUG_IWIN_ITER
		if(iwin_iter->status()==IW_COMPLETED) {
			//this instr is complete, remove it from the list
			IWinItemsIter tmp_iter = iwin_iter;
			iwin_iter++;
			macro_list.erase(tmp_iter);			
		}	
		else {		
			if(iwin_iter->status()==IW_READY) {
				PROFILER_LOG_CP2_ISSUED(iwin_iter)
				iwin_iter->status(IW_ISSUED);
				IsTrue((iwin_iter->latency()>0), ("Instruction latency = 0"));
			}
			if(iwin_iter->status()==IW_ISSUED) {
				iwin_iter->dec_latency();
				if(iwin_iter->latency()==0) {	
					iwin_iter->status(IW_COMPLETED);
					//_dataCheck.releaseMacroLock(iwin_iter);
					_branch.lastBrID(iwin_iter);
					if(list_size==1) {
						list_size--;
					}
				}	
			}
			isDone = TRUE;
		}
	}
	return list_size;
}

void PerfIWin::_updateCp2() {
	IWinItemsIter iwin_iter = _cp2Iwinlist.begin();
	IWinItemsIter iwin_end_iter = _cp2Iwinlist.end();
	while (iwin_iter != iwin_end_iter) {
		_DEBUG_IWIN_ITER
		if(iwin_iter->status()==IW_COMPLETED) {
			//this instr is complete, remove it from the list
			IWinItemsIter tmp_iter = iwin_iter;
			iwin_iter++;
			_cp2Iwinlist.erase(tmp_iter);			
		}		
		else {	
			if(iwin_iter->status()==IW_READY||iwin_iter->status()==IW_XFER_READY) {
				if(iwin_iter->type()!=ISSUE_TYPE_MACRO) {
					PROFILER_LOG_CP2_ISSUED(iwin_iter)
				}
				iwin_iter->status(IW_ISSUED);
				if(iwin_iter->type()!=ISSUE_TYPE_UCODE) {
					_cp2IwinSize--;
				}
				else if(iwin_iter->info()==INSTR_INFO_MACRO_END) {
					_cp2IwinSize--;
				}
				
				PROFILER_CHECK_WRITE_ORDER(iwin_iter)
				IsTrue((iwin_iter->latency()>0), ("Instruction latency = 0"));
			}
			if(iwin_iter->status()==IW_ISSUED) {
				if(iwin_iter->type()==ISSUE_TYPE_MACRO) {
					_hasMacro = TRUE;
					if (_updateMacro(iwin_iter->macroInstr)==0)
					{//this instr is complete, remove it from the list
						_branch.lastBrID(iwin_iter);
						_dataCheck.unlock(iwin_iter);
						iwin_iter->status(IW_COMPLETED);
					}	
				}
				else {
					iwin_iter->dec_latency();
					if(iwin_iter->latency()==0) {	
						_branch.lastBrID(iwin_iter);
						iwin_iter->status(IW_COMPLETED);
					}	
				}
			}
			iwin_iter ++;
		}
	}	
}

void PerfIWin::pre_update() {
	_updateCore();
	_updateCp1();
	_updateCp2();
}

void PerfIWin::post_update(UINT core_issued, UINT cp1_issued,  UINT cp2_issued) {
	_dec_cp1IwinSize(cp1_issued); //CP1 updates its issue window immediately
	_hasMacro = FALSE;
}

