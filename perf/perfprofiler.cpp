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

#ifdef _INCLUDE_PROFILER

#include "decode.h"
#include "tool/prompt.h"
#include "perfprofiler.h"

PerfProfiler profiler;

PerfProfiler::PerfProfiler() : _decoder(DecoderUnit::getInstance()) {
	INT i;
	for(i = 0; i<MAX_REG_INDEX; i++) {
		_reg_write_lock[i] = 0;
	}
	for(i = 0; i<INSTR_MAX; i++) {
		_instrCnt[i] = 0;
	}	
	reset();
	
	isPrintfull(FALSE);
	isPrintlite(FALSE);
	isPrintDetailCnt(FALSE);
	incBufProfiling(FALSE);
	incIWinProfiling(FALSE);
	incIWinFull(FALSE);
	incIssueAnalysis(FALSE);
	incFetchHiddenCyc(FALSE);
	psimInstrMode(FALSE);
	linePerPrint(1);
	_loopCnt = 0;
	_totalCoreCnt = 0;
	_totalCp1Cnt = 0;
	_totalCp2Cnt = 0;
	_totalucodeCnt = 0;
	_coreIWinFullCnt = 1;
	_cp2IWinFullCnt = 1;
	_out = stdout;
	_fetchStallHiddenCyc = 0;
	_fetchStallAccuCyc = 0;
}

void PerfProfiler::reset() {
	INT i;
	_totalCoreCnt += _coreCnt;
	_totalCp1Cnt += _cp1Cnt;
	_totalCp2Cnt += _cp2Cnt;
	_coreCnt = 0;
	_cp1Cnt = 0;
	_cp2Cnt = 0;	
	for(i = 0; i<CORE_MAX_ISSUE; i++) {
		_coreItem[i].hasItem = FALSE;
	}
	for(i = 0; i<CP1_MAX_ISSUE; i++) {
		_cp1Item[i].hasItem = FALSE;
	}
	for(i = 0; i<CP2_MAX_ISSUE; i++) {
		_cp2Item[i].hasItem = FALSE;
	}		
}

void PerfProfiler::_printInstrGroupDetail() {
	fprintf(_out, "\n  -- Instructions Distribution --\n");
	for(INT i = 1; i<(INSTR_MAX-1); i++) {
		fprintf(_out, "	%s = %u (%0.4f)\n", instr_group_long_string[i], _instrCnt[i], ((float) _instrCnt[i])/_totalIssuedCnt());
	}
}

void PerfProfiler::_printInstr(IWinItemsIter iwin_iter, ExpInstr* instr) {
	if(psimInstrMode()==FALSE) {
		Prompt::show(_out, instr, iwin_iter->perf_id(), NULL);
	}
	else {
		RegVecIter regIter = iwin_iter->dest_reg.begin();
		RegVecIter endIter = iwin_iter->dest_reg.end();
		fprintf(_out, "%08x %08x %s(%x) dest[ ", iwin_iter->perf_id(), iwin_iter->pc(), instr->fmtMap()->mn(), iwin_iter->instr_id());
		while (regIter!=endIter) {
			INT index = *regIter;
			fprintf(_out, "%d ", index);
			regIter++;
		}
		fprintf(_out, "] src[ ");
		//Src dependence check
		regIter = iwin_iter->src_reg.begin();
		endIter = iwin_iter->src_reg.end();			
		while (regIter!=endIter) {
			INT index = *regIter;
			fprintf(_out, "%d ", index);
			regIter++;
		}	
		fprintf(_out, "] (%x, %x, %x)", iwin_iter->spec_id(), iwin_iter->xfer_id(), iwin_iter->hazard());		
		if(iwin_iter->type()!=ISSUE_TYPE_UCODE) {
			fprintf(_out, "\n");
		}
		else {
			fprintf(_out, " u-%d\n", iwin_iter->sub_id());
		}
	}
}

void PerfProfiler::init(PerfIWin& iwin, PerfFetch& fetch) {
	UINT i;
	
	if(incBufProfiling()==TRUE) {
		_coreBufSize = new UINT[fetch.bufSize()];
		_cp1BufSize = new UINT[fetch.bufSize()];
		_cp2BufSize = new UINT[fetch.bufSize()];
		for(i = 0; i<fetch.bufSize(); i++) {
			_coreIWinSize[i] = 0;
		}
	}

	if(incIWinProfiling()==TRUE) {
		_coreIWinSize = new UINT[iwin.coreIwinMaxSize()];
		_cp1IWinSize = new UINT[iwin.cp1IwinMaxSize()];
		_cp2IWinSize = new UINT[iwin.cp2IwinMaxSize()];	
		for(i = 0; i<iwin.coreIwinMaxSize(); i++) {
			_coreBufSize[i] = 0;
		}	
		for(i = 0; i<iwin.cp1IwinMaxSize(); i++) {
			_cp1IWinSize[i] = 0;
		}
		for(i = 0; i<iwin.cp2IwinMaxSize(); i++) {
			_cp2IWinSize[i] = 0;
		}	
	}
	
	if(incIWinFull()==TRUE) {
		for(i = 0; i<HS_IWIN_FULL_MAX; i++) {
			_iwinFullCnt[i] = 0;
		}
	}

	if(incIssueAnalysis()==TRUE) {
		for(i = 0; i<HS_IWIN_FULL_MAX; i++) {
			_issueAnalysisCnt[i] = 0;
		}
		for(i = 0; i<CORE_MAX_ISSUE+1; i++) {
			_coreIssueAnalysisCnt[CORE_MAX_ISSUE] = 1;
		}
		for(i = 0; i<CP2_MAX_ISSUE+1; i++) {
			_cp2IssueAnalysisCnt[CP2_MAX_ISSUE] = 1;
		}		
	}
	#if _INSTR_MONITOR
		for(i = 0; i<HS_IWIN_FULL_MAX; i++) {
			_monitorAnalysisCnt[i] = 0;
		}	
		_monitorType = INSTR_CP2_SIMD;
		_monitorProcessor = PROCESSOR_CP2;
		_monitorLastIssue = 0;
		_monitorCnt = 0;
		_monitorSameInstr = 0;		
	#endif		
}

void PerfProfiler::checkIWinFull(PerfIWin& iwin) {
	if(incIWinFull()==TRUE) {
		BOOL counted = FALSE;
		if(iwin.coreIwinSize()==iwin.coreIwinMaxSize()) {
			IWinItems* iwinList = iwin.getCoreIwinlist();
			IWinItemsIter iwin_iter = iwinList->begin();
			IWinItemsIter iwin_end_iter = iwinList->end();
			BOOL done = FALSE;
			while(iwin_iter!=iwin_end_iter&&done==FALSE) {
				if(iwin_iter->status()==IW_NOT_READY) {
					ES_HAZARD_STATUS hazard = iwin_iter->hazard();
					if(hazard!=HS_BR_STALL) {
						if(hazard==HS_NONE) {
							hazard = HS_XFER_SYN;
						}
						_iwinFullCnt[hazard]++;
						_coreIWinFullCnt++;
						counted = TRUE;
					}
					done = TRUE;
				}
				else
					iwin_iter++;
			}
		}
		if(counted==TRUE) {
			_iwinFullCnt[0]++;
		}
	}
}

void PerfProfiler::printIWinFullResult(UINT cyc) {
	if(incIWinFull()==TRUE) {
		_iwinFullCnt[0] += 1;
		fprintf(_out, "\n######## IWin Full analysis Result (Core only) ########\n");
		fprintf(_out, "Total cycle with iwin full:	%u	(%0.2f)\n", _iwinFullCnt[0], ((float)_iwinFullCnt[0])/cyc);
		fprintf(_out, "Cycle with core iwin full:	%u	(%0.2f)\n", _coreIWinFullCnt, ((float)_coreIWinFullCnt)/cyc);
		//fprintf(_out, "Cycle with cp2 iwin full:	%u	(%0.2f)\n", _cp2IWinFullCnt, ((float)_cp2IWinFullCnt)/cyc);
		fprintf(_out, "\n   -- Reason --\n");	
		fprintf(_out, "Engine not ready:		%u	(%0.2f)\n", _iwinFullCnt[HS_ENGINE_NOT_READY], ((float)_iwinFullCnt[HS_ENGINE_NOT_READY])/_iwinFullCnt[0]);
		fprintf(_out, "Engine is Full:			%u	(%0.2f)\n", _iwinFullCnt[HS_ENGINE_FULL],  ((float)_iwinFullCnt[HS_ENGINE_FULL])/_iwinFullCnt[0]);
		fprintf(_out, "Invalid instruction order:	%u	(%0.2f)\n", _iwinFullCnt[HS_INSTR_ORDER], ((float)_iwinFullCnt[HS_INSTR_ORDER])/_iwinFullCnt[0]);
		fprintf(_out, "Xfer pipe not clear:		%u	(%0.2f)\n", _iwinFullCnt[HS_XFER_PIPE_NOT_CLEAR], ((float)_iwinFullCnt[HS_XFER_PIPE_NOT_CLEAR])/_iwinFullCnt[0]);
		fprintf(_out, "Xfer not synchronizated:	%u	(%0.2f)\n", _iwinFullCnt[HS_XFER_SYN], ((float)_iwinFullCnt[HS_XFER_SYN])/_iwinFullCnt[0]);
		fprintf(_out, "Speculated instruction:		%u	(%0.2f)\n", _iwinFullCnt[HS_BR_STALL], ((float)_iwinFullCnt[HS_BR_STALL])/_iwinFullCnt[0]);
		fprintf(_out, "Register port jam:		%u	(%0.2f)\n", _iwinFullCnt[HS_PORT_JAM], ((float)_iwinFullCnt[HS_PORT_JAM])/_iwinFullCnt[0]);
		fprintf(_out, "Register Macro reserved:	%u	(%0.2f)\n", _iwinFullCnt[HS_REG_MACRO], ((float)_iwinFullCnt[HS_REG_MACRO])/_iwinFullCnt[0]);
		fprintf(_out, "Register WaW hazard:		%u	(%0.2f)\n", _iwinFullCnt[HS_REG_WAW], ((float)_iwinFullCnt[HS_REG_WAW])/_iwinFullCnt[0]);
		fprintf(_out, "Register RaW hazard:		%u	(%0.2f)\n", _iwinFullCnt[HS_REG_RAW], ((float)_iwinFullCnt[HS_REG_RAW])/_iwinFullCnt[0]);
		fprintf(_out, "Register WaR hazard:		%u	(%0.2f)\n", _iwinFullCnt[HS_REG_WAR], ((float)_iwinFullCnt[HS_REG_WAR])/_iwinFullCnt[0]);

	}
}		

void PerfProfiler::printIWinFullResultLine(UINT cyc) {
	if(incIWinFull()==TRUE) {
		_iwinFullCnt[0] += 1;
		fprintf(_out, "%u(%0.2f)", _iwinFullCnt[0], ((float)_iwinFullCnt[0])/cyc);
		fprintf(_out, ",%u(%0.2f)", _coreIWinFullCnt, ((float)_coreIWinFullCnt)/cyc);
		//fprintf(_out, ",%u(%0.2f)", _cp2IWinFullCnt, ((float)_cp2IWinFullCnt)/cyc);
		fprintf(_out, ",%u(%0.2f)", _iwinFullCnt[HS_ENGINE_NOT_READY], ((float)_iwinFullCnt[HS_ENGINE_NOT_READY])/_iwinFullCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _iwinFullCnt[HS_ENGINE_FULL],  ((float)_iwinFullCnt[HS_ENGINE_FULL])/_iwinFullCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _iwinFullCnt[HS_INSTR_ORDER], ((float)_iwinFullCnt[HS_INSTR_ORDER])/_iwinFullCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _iwinFullCnt[HS_XFER_PIPE_NOT_CLEAR], ((float)_iwinFullCnt[HS_XFER_PIPE_NOT_CLEAR])/_iwinFullCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _iwinFullCnt[HS_XFER_SYN], ((float)_iwinFullCnt[HS_XFER_SYN])/_iwinFullCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _iwinFullCnt[HS_BR_STALL], ((float)_iwinFullCnt[HS_BR_STALL])/_iwinFullCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _iwinFullCnt[HS_PORT_JAM], ((float)_iwinFullCnt[HS_PORT_JAM])/_iwinFullCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _iwinFullCnt[HS_REG_MACRO], ((float)_iwinFullCnt[HS_REG_MACRO])/_iwinFullCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _iwinFullCnt[HS_REG_WAW], ((float)_iwinFullCnt[HS_REG_WAW])/_iwinFullCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _iwinFullCnt[HS_REG_RAW], ((float)_iwinFullCnt[HS_REG_RAW])/_iwinFullCnt[0]);
		fprintf(_out, ",%u(%0.2f)\n", _iwinFullCnt[HS_REG_WAR], ((float)_iwinFullCnt[HS_REG_WAR])/_iwinFullCnt[0]);

	}
}

void PerfProfiler::_monitorInstrType(PerfIWin& iwin) {
	#if _INSTR_MONITOR
	if(_monitorType>INSTR_UNDEFINED) {
		if(_instrCnt[_monitorType]==_monitorLastIssue) {
			IWinItems* iwinList = iwin.getCoreIwinlist();
			IWinItemsIter iwin_iter;
			IWinItemsIter iwin_end_iter;
			ES_HAZARD_STATUS lastHazrd = HS_NONE;
			INT i = 0;
			INT cnt = 0;
			BOOL iwinFull = FALSE;
		
			switch (_monitorProcessor) {
				case PROCESSOR_CORE:
					iwinList = iwin.getCoreIwinlist();
					iwinFull = iwin.coreIwinMaxSize()==iwin.coreIwinSize();
					break;
				case PROCESSOR_CP1:
					iwinList = iwin.getCp1Iwinlist();
					iwinFull = iwin.cp1IwinMaxSize()==iwin.cp1IwinSize();
					break;
				case PROCESSOR_CP2:
					iwinList = iwin.getCp2Iwinlist();
					iwinFull = iwin.cp2IwinMaxSize()==iwin.cp2IwinSize();
					break;
				default:
					break;
			}
			iwin_iter = iwinList->begin();
			iwin_end_iter = iwinList->end();	
			while(iwin_iter!=iwin_end_iter&&cnt==0) {
				if(iwin_iter->status()==IW_NOT_READY) {
					ES_HAZARD_STATUS hazard = iwin_iter->hazard();
					if(i==0&&hazard==HS_BR_STALL&&iwinFull) {
						//branch missed predicted situation
						break;
					}
					if(lastHazrd==HS_ENGINE_FULL) {
						hazard = HS_ENGINE_FULL;
					}
					else if(lastHazrd==HS_BR_STALL) {
						hazard = HS_BR_STALL;
					}
					else {
						lastHazrd = hazard;
					}
					if(hazard==HS_NONE) {
						hazard = HS_XFER_SYN;
					}
					if(iwin_iter->unit()==_monitorType) {
						_monitorAnalysisCnt[hazard]++;
						cnt++;
					}
					i++;
				}
				iwin_iter++;
			}
			if(cnt==0) {
				_monitorAnalysisCnt[0]++;
			}
			_monitorCnt += cnt;
		}
		else {
			_monitorLastIssue = _instrCnt[_monitorType];
		}
	}
	#endif	
}

void PerfProfiler::checkIssue(PerfIWin& iwin, UINT coreIssued, UINT cp2Issued) {
	if(incIssueAnalysis()==TRUE) {
		if(coreIssued<((UINT)CORE_MAX_ISSUE)) {
			IsTrue((coreIssued!=3), ("Invalid issue number (%d)", coreIssued));
			IWinItems* iwinList = iwin.getCoreIwinlist();
			IWinItemsIter iwin_iter = iwinList->begin();
			IWinItemsIter iwin_end_iter = iwinList->end();
			ES_HAZARD_STATUS lastHazrd = HS_NONE;
			INT i = 0;
			_coreIssueAnalysisCnt[CORE_MAX_ISSUE]++;
			_coreIssueAnalysisCnt[coreIssued]++;

			while(iwin_iter!=iwin_end_iter) {
				if(iwin_iter->status()==IW_NOT_READY) {
					ES_HAZARD_STATUS hazard = iwin_iter->hazard();
					if(i==0&&hazard==HS_BR_STALL&&iwin.coreIwinMaxSize()==iwin.coreIwinSize()) {
						//branch missed predicted situation
						break;
					}
					else {
						if(lastHazrd==HS_ENGINE_FULL) {
							hazard = HS_ENGINE_FULL;
						}
						else if(lastHazrd==HS_BR_STALL) {
							hazard = HS_BR_STALL;
						}
						else {
							lastHazrd = hazard;
						}
						if(hazard==HS_NONE) {
							hazard = HS_XFER_SYN;
						}
						_issueAnalysisCnt[hazard]++;
						_issueAnalysisCnt[0]++;
						i++;
					}
				}
				iwin_iter++;
			}
		}
	}
	#if _INSTR_MONITOR
		_monitorInstrType(iwin);
	#endif
}

void PerfProfiler::printIssueResult(UINT cyc) {
	if(incIssueAnalysis()==TRUE) {
		_coreIssueAnalysisCnt[CORE_MAX_ISSUE]--;
		_cp2IssueAnalysisCnt[CP2_MAX_ISSUE]--;
		fprintf(_out, "\n######## Issue analysis Result (Core only)########\n");
		fprintf(_out, "Total Instruction involved:		%u\n", _issueAnalysisCnt[0]);
		fprintf(_out, "Total cycle with issue < max:		%u	(%0.2f)\n",_coreIssueAnalysisCnt[CORE_MAX_ISSUE]+_cp2IssueAnalysisCnt[CP2_MAX_ISSUE], ((float)_coreIssueAnalysisCnt[CORE_MAX_ISSUE]+_cp2IssueAnalysisCnt[CP2_MAX_ISSUE])/cyc);
		fprintf(_out, "Total cycle with core issue < max:	%u	(%0.2f)\n", _coreIssueAnalysisCnt[CORE_MAX_ISSUE], ((float)_coreIssueAnalysisCnt[CORE_MAX_ISSUE])/cyc);
		for(INT i = 0; i<CORE_MAX_ISSUE; i++) {
			fprintf(_out, "	Core issue = %d:			%u	(%0.2f)\n", i, _coreIssueAnalysisCnt[i], ((float)_coreIssueAnalysisCnt[i])/cyc);
		}
		/*
		fprintf(_out, "Total cycle with cp2 issue < max:	%u	(%0.2f)\n", _cp2IssueAnalysisCnt[CP2_MAX_ISSUE], ((float)_cp2IssueAnalysisCnt[CP2_MAX_ISSUE])/cyc);
		for(INT i = 0; i<CORE_MAX_ISSUE; i++) {
			fprintf(_out, "	Cp2 issue = %d:			%u	(%0.2f)\n", i, _cp2IssueAnalysisCnt[i], ((float)_cp2IssueAnalysisCnt[i])/cyc);
		}
		*/
		fprintf(_out, "\n   -- Reason --\n");	
		fprintf(_out, "Engine not ready:		%u	(%0.2f)\n", _issueAnalysisCnt[HS_ENGINE_NOT_READY], ((float)_issueAnalysisCnt[HS_ENGINE_NOT_READY])/_issueAnalysisCnt[0]);
		fprintf(_out, "Engine is Full:			%u	(%0.2f)\n", _issueAnalysisCnt[HS_ENGINE_FULL],  ((float)_issueAnalysisCnt[HS_ENGINE_FULL])/_issueAnalysisCnt[0]);
		fprintf(_out, "Invalid instruction order:	%u	(%0.2f)\n", _issueAnalysisCnt[HS_INSTR_ORDER], ((float)_issueAnalysisCnt[HS_INSTR_ORDER])/_issueAnalysisCnt[0]);
		fprintf(_out, "Xfer pipe not clear:		%u	(%0.2f)\n", _issueAnalysisCnt[HS_XFER_PIPE_NOT_CLEAR], ((float)_issueAnalysisCnt[HS_XFER_PIPE_NOT_CLEAR])/_issueAnalysisCnt[0]);
		fprintf(_out, "Xfer not synchronizated:	%u	(%0.2f)\n", _issueAnalysisCnt[HS_XFER_SYN], ((float)_issueAnalysisCnt[HS_XFER_SYN])/_issueAnalysisCnt[0]);
		fprintf(_out, "Speculated instruction:		%u	(%0.2f)\n", _issueAnalysisCnt[HS_BR_STALL], ((float)_issueAnalysisCnt[HS_BR_STALL])/_issueAnalysisCnt[0]);
		fprintf(_out, "Register port jam:		%u	(%0.2f)\n", _issueAnalysisCnt[HS_PORT_JAM], ((float)_issueAnalysisCnt[HS_PORT_JAM])/_issueAnalysisCnt[0]);
		fprintf(_out, "Register Macro reserved:	%u	(%0.2f)\n", _issueAnalysisCnt[HS_REG_MACRO], ((float)_issueAnalysisCnt[HS_REG_MACRO])/_issueAnalysisCnt[0]);
		fprintf(_out, "Register WaW hazard:		%u	(%0.2f)\n", _issueAnalysisCnt[HS_REG_WAW], ((float)_issueAnalysisCnt[HS_REG_WAW])/_issueAnalysisCnt[0]);
		fprintf(_out, "Register RaW hazard:		%u	(%0.2f)\n", _issueAnalysisCnt[HS_REG_RAW], ((float)_issueAnalysisCnt[HS_REG_RAW])/_issueAnalysisCnt[0]);
		fprintf(_out, "Register WaR hazard:		%u	(%0.2f)\n", _issueAnalysisCnt[HS_REG_WAR], ((float)_issueAnalysisCnt[HS_REG_WAR])/_issueAnalysisCnt[0]);

	}
	#if _INSTR_MONITOR
		if(_monitorType>INSTR_UNDEFINED) {
			fprintf(_out, "\n######## Issue analysis Result for instruction type (%s) ########\n", instr_group_long_string[_monitorType]);
			fprintf(_out, "Total Instruction count:		%u\n", _instrCnt[_monitorType]);
			fprintf(_out, "Issue rate of the instruction:		%0.4f\n", ((float)_instrCnt[_monitorType])/cyc);
			fprintf(_out, "Instruction involves in the checking:	%u\n", _monitorCnt);
			fprintf(_out, "\n   -- Reason --\n");	
			fprintf(_out, "Cycle without the instruction in i-win:		%u	(%0.2f)\n", _monitorAnalysisCnt[0], ((float)_monitorAnalysisCnt[0])/cyc);
			fprintf(_out, "Engine not ready:		%u	(%0.2f)\n", _monitorAnalysisCnt[HS_ENGINE_NOT_READY], ((float)_monitorAnalysisCnt[HS_ENGINE_NOT_READY])/_monitorCnt);
			fprintf(_out, "Engine is Full:			%u	(%0.2f)\n", _monitorAnalysisCnt[HS_ENGINE_FULL],  ((float)_monitorAnalysisCnt[HS_ENGINE_FULL])/_monitorCnt);
			fprintf(_out, "Invalid instruction order:	%u	(%0.2f)\n", _monitorAnalysisCnt[HS_INSTR_ORDER], ((float)_monitorAnalysisCnt[HS_INSTR_ORDER])/_monitorCnt);
			fprintf(_out, "Xfer pipe not clear:		%u	(%0.2f)\n", _monitorAnalysisCnt[HS_XFER_PIPE_NOT_CLEAR], ((float)_monitorAnalysisCnt[HS_XFER_PIPE_NOT_CLEAR])/_monitorCnt);			
			fprintf(_out, "Xfer not synchronizated:	%u	(%0.2f)\n", _monitorAnalysisCnt[HS_XFER_SYN], ((float)_monitorAnalysisCnt[HS_XFER_SYN])/_monitorCnt);
			fprintf(_out, "Speculated instruction:		%u	(%0.2f)\n", _monitorAnalysisCnt[HS_BR_STALL], ((float)_monitorAnalysisCnt[HS_BR_STALL])/_monitorCnt);
			fprintf(_out, "Register port jam:		%u	(%0.2f)\n", _monitorAnalysisCnt[HS_PORT_JAM], ((float)_monitorAnalysisCnt[HS_PORT_JAM])/_monitorCnt);
			fprintf(_out, "Register Macro reserved:	%u	(%0.2f)\n", _monitorAnalysisCnt[HS_REG_MACRO], ((float)_monitorAnalysisCnt[HS_REG_MACRO])/_monitorCnt);
			fprintf(_out, "Register WaW hazard:		%u	(%0.2f)\n", _monitorAnalysisCnt[HS_REG_WAW], ((float)_monitorAnalysisCnt[HS_REG_WAW])/_monitorCnt);
			fprintf(_out, "Register RaW hazard:		%u	(%0.2f)\n", _monitorAnalysisCnt[HS_REG_RAW], ((float)_monitorAnalysisCnt[HS_REG_RAW])/_monitorCnt);
			fprintf(_out, "Register WaR hazard:		%u	(%0.2f)\n", _monitorAnalysisCnt[HS_REG_WAR], ((float)_monitorAnalysisCnt[HS_REG_WAR])/_monitorCnt);
		}

	#endif	
}

void PerfProfiler::printIssueResultLine(UINT cyc) {
	if(incIssueAnalysis()==TRUE) {
		fprintf(_out, "%u", _issueAnalysisCnt[0]);
		fprintf(_out, ",%u(%0.2f)",_coreIssueAnalysisCnt[CORE_MAX_ISSUE]+_cp2IssueAnalysisCnt[CP2_MAX_ISSUE], ((float)_coreIssueAnalysisCnt[CORE_MAX_ISSUE]+_cp2IssueAnalysisCnt[CP2_MAX_ISSUE])/cyc);
		fprintf(_out, ",%u(%0.2f)", _coreIssueAnalysisCnt[CORE_MAX_ISSUE], ((float)_coreIssueAnalysisCnt[CORE_MAX_ISSUE])/cyc);
		for(INT i = 0; i<CORE_MAX_ISSUE; i++) {
			fprintf(_out, ",%u(%0.2f)", _coreIssueAnalysisCnt[i], ((float)_coreIssueAnalysisCnt[i])/cyc);
		}
		/*
		fprintf(_out, ",%u(%0.2f)", _cp2IssueAnalysisCnt[CP2_MAX_ISSUE], ((float)_cp2IssueAnalysisCnt[CP2_MAX_ISSUE])/cyc);
		for(INT i = 0; i<CP2_MAX_ISSUE; i++) {
			fprintf(_out, ",%u(%0.2f)", _cp2IssueAnalysisCnt[i], ((float)_cp2IssueAnalysisCnt[i])/cyc);
		}
		*/
		fprintf(_out, ",%u(%0.2f)", _issueAnalysisCnt[HS_ENGINE_NOT_READY], ((float)_issueAnalysisCnt[HS_ENGINE_NOT_READY])/_issueAnalysisCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _issueAnalysisCnt[HS_ENGINE_FULL],  ((float)_issueAnalysisCnt[HS_ENGINE_FULL])/_issueAnalysisCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _issueAnalysisCnt[HS_INSTR_ORDER], ((float)_issueAnalysisCnt[HS_INSTR_ORDER])/_issueAnalysisCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _issueAnalysisCnt[HS_XFER_PIPE_NOT_CLEAR], ((float)_issueAnalysisCnt[HS_XFER_PIPE_NOT_CLEAR])/_issueAnalysisCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _issueAnalysisCnt[HS_XFER_SYN], ((float)_issueAnalysisCnt[HS_XFER_SYN])/_issueAnalysisCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _issueAnalysisCnt[HS_BR_STALL], ((float)_issueAnalysisCnt[HS_BR_STALL])/_issueAnalysisCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _issueAnalysisCnt[HS_PORT_JAM], ((float)_issueAnalysisCnt[HS_PORT_JAM])/_issueAnalysisCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _issueAnalysisCnt[HS_REG_MACRO], ((float)_issueAnalysisCnt[HS_REG_MACRO])/_issueAnalysisCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _issueAnalysisCnt[HS_REG_WAW], ((float)_issueAnalysisCnt[HS_REG_WAW])/_issueAnalysisCnt[0]);
		fprintf(_out, ",%u(%0.2f)", _issueAnalysisCnt[HS_REG_RAW], ((float)_issueAnalysisCnt[HS_REG_RAW])/_issueAnalysisCnt[0]);
		fprintf(_out, ",%u(%0.2f)\n", _issueAnalysisCnt[HS_REG_WAR], ((float)_issueAnalysisCnt[HS_REG_WAR])/_issueAnalysisCnt[0]);

	}	
}

void PerfProfiler::checkFetchHiddenCyc(PerfFetch& fetch) {
	if(incFetchHiddenCyc()==TRUE) {
		if(fetch.fetchStallCyc()>0) {
			_fetchStallAccuCyc++;
			if(_coreCnt>0||_cp1Cnt>0||_cp2Cnt>0) {
				_fetchStallHiddenCyc += _fetchStallAccuCyc;
				_fetchStallAccuCyc = 0;
			}
		}
		else {
			_fetchStallAccuCyc = 0;
		}
	}
}

void PerfProfiler::printICacheDetailLine(PerfCache& cache, UINT cyc) {
	if(cache.incCache()) {
		fprintf(_out, "%u", cache.icacheAccess());
		fprintf(_out, ",%u(%0.2f)", cache.icacheMissed(), ((float)cache.icacheMissed())/cache.icacheAccess());
		fprintf(_out, ",%u", cache.icacheMissed()*cache.icacheMissCyc());
		if(incFetchHiddenCyc()==TRUE) {
			fprintf(_out, ",%u(%0.2f)", _fetchStallHiddenCyc, ((float) _fetchStallHiddenCyc)/(cache.icacheMissed()*cache.icacheMissCyc()));
		}
		fprintf(_out, "\n");
	}
}

void PerfProfiler::bufSizeProfiling(PerfFetch& fetch) {
	if(incBufProfiling()==TRUE) {
		_coreBufSize[fetch.curCoreBufSize()]+=1;
		_cp1BufSize[fetch.curCp1BufSize()]+=1;
		_cp2BufSize[fetch.curCp2BufSize()]+=1;		
	}
}

void PerfProfiler::iwinSizeProfiling(PerfIWin& iwin) {
	if(incBufProfiling()==TRUE) {
		_coreIWinSize[iwin.coreIwinSize()]+=1;
		_cp1IWinSize[iwin.cp1IwinSize()]+=1;
		_cp2IWinSize[iwin.cp2IwinSize()]+=1;		
	}
}

void PerfProfiler::printFullReport(UINT totalCycle, PerfOrder& order, PerfBranch& branch, PerfEngine& engine, 
					 PerfIWin& iwin, PerfDataDep& dataCheck, PerfDispatch& dispatch, PerfFetch& fetch, PerfCache& cache) {
	fprintf(_out, "\nCompeleted Successfully!\n");
	fprintf(_out, "\n######## Overall results ########\n");
	fprintf(_out, "\n   -- Instructions Fetched --\n");	
	fprintf(_out, "Core instructions fetched:	%u\n", fetch.corecnt());
	fprintf(_out, "CP1 instructions fetched:	%u\n", fetch.cp1cnt());
	fprintf(_out, "CP2 instructions fetched:	%u\n", fetch.cp2cnt());
	fprintf(_out, "CP3 instructions fetched:	%u\n", fetch.cp3cnt());
	fprintf(_out, "BC instructions fetched:		%u\n", fetch.bccnt());
	fprintf(_out, "CP-Core Xfer Pair fetched:	%u\n", fetch.cpXfer());
	fprintf(_out, "Macro instructions fetched:	%u\n", fetch.macrocnt());
	fprintf(_out, "NOP instructions dicarded:	%u\n", fetch.nopcnt());
	fprintf(_out, "Total instructions read:	%u\n", fetch.totalcnt() );
	
	if(cache.incCache()) {
		fprintf(_out, "\n   -- Instruction Cache --\n");	
		fprintf(_out, "Total icache access:		%u\n", cache.icacheAccess());
		fprintf(_out, "icache missed:		%u (%0.4f)\n", cache.icacheMissed(), ((float)cache.icacheMissed())/cache.icacheAccess());
		fprintf(_out, "icache miss penalty:		%u\n", cache.icacheMissed()*cache.icacheMissCyc());
		fprintf(_out, "Total dcache access:		%u\n", cache.dcacheAccess());
		fprintf(_out, "dcache missed:		%u (%0.4f)\n", cache.dcacheMissed(), ((float)cache.dcacheMissed())/cache.dcacheAccess());
		fprintf(_out, "dcache miss penalty:		%u\n", cache.dcacheMissed()*cache.dcacheMissCyc());	
		fprintf(_out, "icache missed hidden cycle:	%u (%0.4f)\n", _fetchStallHiddenCyc, ((float)_fetchStallHiddenCyc)/(cache.icacheMissed()*cache.icacheMissCyc()));	
	}
	
	fprintf(_out, "\n   -- Instructions Issued --\n");		
	fprintf(_out, "Core instructions issued:		%u\n", _totalCoreCnt);
	fprintf(_out, "CP1 instructions issued:		%u\n", _totalCp1Cnt);
	fprintf(_out, "CP2 instructions issued:		%u\n", _totalCp2Cnt);
	fprintf(_out, "Expanded Macro instructions issued:	%u\n", _totalucodeCnt);	
	fprintf(_out, "Total jump instruction:			%u\n", branch.totalJr());
	fprintf(_out, "	Function call/return jump:	%u\n", branch.callCnt());
	fprintf(_out, "	Java change mode:			%u\n", branch.chmodCnt());
	fprintf(_out, "	Java Exception:			%u\n", branch.excepCnt());
	fprintf(_out, "	Other jump:			%u\n", branch.otherJr());
	fprintf(_out, "	Branch instruction:			%u\n", branch.totalBr());
	if(branch.incBrPenalty()==TRUE) {
		fprintf(_out, "Branch miss predicted:			%u (%0.4f)\n", branch.missBr(), ((float)branch.missBr())/branch.totalBr() );
	}
	fprintf(_out, "Total instructions issued:		%u\n", _totalIssuedCnt());
	if(isPrintDetailCnt()==TRUE) {
		_printInstrGroupDetail();
	}

	fprintf(_out, "\n   -- Issue rate --\n");
	fprintf(_out, "Total cycle:				%u\n", totalCycle);
	fprintf(_out, "Mips issue rate:			%0.4f\n", ((float)_totalCoreCnt)/totalCycle);
	fprintf(_out, "CP1 issue rate:				%0.4f\n", ((float)_totalCp1Cnt)/totalCycle);
	fprintf(_out, "CP2 issue rate:				%0.4f\n", ((float)_totalCp2Cnt)/totalCycle);
	fprintf(_out, "Overrall issue rate:			%0.4f\n", ((float)_totalIssuedCnt())/totalCycle);

}

void PerfProfiler::printLineReport(FILE* line_out, UINT totalCycle, PerfOrder& order, PerfBranch& branch, PerfEngine& engine, 
					 PerfIWin& iwin, PerfDataDep& dataCheck, PerfDispatch& dispatch, PerfFetch& fetch, PerfCache& cache) {
	/*fprintf(_out, "core_fcnt cp1_fcnt cp2_fcnt cp3_fcnt bc_cnt xfer_fcnt macro_fcnt nop_fcnt total_fcnt 
	 * 				 ic_total ic_miss ic_penalty dc_total dc_miss dc_penalty ic_miss_hidden
	 * 				 core_icnt cp1_icnt cp2_icnt ucode_icnt jr_icnt br_icnt total_icnt 
	 * 				 AL0 LS0 BR0 LS1 XF1 LS2 XF2 AL1 LS1 XF1 BR1 SIM MAC SUM MAD BIT SCN LS2 XF2 XFI BR2 BR3 LS3 XF3
	 * 				 cyc core_ir cp1_ir cp2_ir total_ir
	 * ");*/			 	
	fprintf(line_out, "%u", fetch.corecnt());
	fprintf(line_out, ",%u", fetch.cp1cnt());
	fprintf(line_out, ",%u", fetch.cp2cnt());
	fprintf(line_out, ",%u", fetch.cp3cnt());
	fprintf(line_out, ",%u", fetch.bccnt());
	fprintf(line_out, ",%u", fetch.cpXfer());
	fprintf(line_out, ",%u", fetch.macrocnt());
	fprintf(line_out, ",%u", fetch.nopcnt());
	fprintf(line_out, ",%u", fetch.totalcnt() );

	fprintf(line_out, ",%u", cache.icacheAccess());
	fprintf(line_out, ",%u(%0.2f)", cache.icacheMissed(), ((float)cache.icacheMissed())/cache.icacheAccess());
	fprintf(line_out, ",%u", cache.icacheMissed()*cache.icacheMissCyc());
	fprintf(line_out, ",%u", cache.dcacheAccess());
	fprintf(line_out, ",%u(%0.2f)", cache.dcacheMissed(), ((float)cache.dcacheMissed())/cache.dcacheAccess());
	fprintf(line_out, ",%u", cache.dcacheMissed()*cache.dcacheMissCyc());	
	fprintf(line_out, ",%u(%0.2f)", _fetchStallHiddenCyc, ((float)_fetchStallHiddenCyc)/(cache.icacheMissed()*cache.icacheMissCyc()));	
	
	fprintf(line_out, ",%u", _totalCoreCnt);
	fprintf(line_out, ",%u", _totalCp1Cnt);
	fprintf(line_out, ",%u", _totalCp2Cnt);
	fprintf(line_out, ",%u", _totalucodeCnt);	
	fprintf(line_out, ",%u", branch.totalJr());
	fprintf(line_out, ",%u(%0.2f)", branch.totalBr(), ((float)branch.missBr())/branch.totalBr());
	fprintf(line_out, ",%u", _totalIssuedCnt());
	for(INT i = 1; i<(INSTR_MAX-1); i++) {
		fprintf(line_out, ",%u", _instrCnt[i]);
	}

	fprintf(line_out, ",%u", totalCycle);
	fprintf(line_out, ",%0.2f", ((float)_totalCoreCnt)/totalCycle);
	fprintf(line_out, ",%0.2f", ((float)_totalCp1Cnt)/totalCycle);
	fprintf(line_out, ",%0.2f", ((float)_totalCp2Cnt)/totalCycle);
	fprintf(line_out, ",%0.2f", ((float)_totalIssuedCnt())/totalCycle);
	fprintf(line_out, "\n");
}

void PerfProfiler::_printBufDetail(UINT cyc, PerfFetch& fetch) {
	
	FetchBufIter iter = fetch.getCoreBufCur();
	FetchBufIter iter_end = fetch.getCoreBufEnd();
	fprintf(_out, "####Fetch Debug: Cycle %08u ####\n", cyc);
	fprintf(_out, "Core (%d): \n", fetch.curCoreBufSize());
	while(iter!=iter_end) {
		UINT raw = iter->raw;
		UINT8* pr = (UINT8 *) &raw;
		ExpInstr instr;
		_decoder.decode(pr, &instr);
		Prompt::show(_out, &instr, iter->perfID, NULL);
		iter++;
	}
	iter = fetch.getCp1BufCur();
	iter_end = fetch.getCp1BufEnd();
	fprintf(_out, "CP1 (%d): \n", fetch.curCp1BufSize());
	while(iter!=iter_end) {
		UINT raw = iter->raw;
		UINT8* pr = (UINT8 *) &raw;
		ExpInstr instr;
		_decoder.decode(pr, &instr);
		Prompt::show(_out, &instr, iter->perfID, NULL);
		iter++;
	}
	iter = fetch.getCp2BufCur();
	iter_end = fetch.getCp2BufEnd();
	fprintf(_out, "CP2 (%d): \n", fetch.curCp2BufSize());
	while(iter!=iter_end) {
		UINT raw = iter->raw;
		UINT8* pr = (UINT8 *) &raw;
		ExpInstr instr;
		_decoder.decode(pr, &instr);
		Prompt::show(_out, &instr, iter->perfID, NULL);
		iter++;
	}
	fprintf(_out, "#############\n");
}

void PerfProfiler::_printIWinDetail(UINT cyc, PerfIWin& iwin) {
	
	IWinItems* iwinList = iwin.getCoreIwinlist();
	IWinItemsIter iwin_iter = iwinList->begin();
	IWinItemsIter iwin_end_iter = iwinList->end();
	fprintf(_out, "####IWin Debug: Cycle %08u ####\n", cyc);
	fprintf(_out, "Core (%d): \n", iwin.coreIwinSize());
	while(iwin_iter!=iwin_end_iter) {
		UINT raw = iwin_iter->raw();
		UINT8* pr = (UINT8 *) &raw;
		ExpInstr instr;
		_decoder.decode(pr, &instr);
		_printInstr(iwin_iter, &instr);	
		iwin_iter++;
	}
	iwinList = iwin.getCp1Iwinlist();
	iwin_iter = iwinList->begin();
	iwin_end_iter = iwinList->end();	
	fprintf(_out, "CP1 (%d): \n", iwin.cp1IwinSize());
	while(iwin_iter!=iwin_end_iter) {
		UINT raw = iwin_iter->raw();
		UINT8* pr = (UINT8 *) &raw;
		ExpInstr instr;
		_decoder.decode(pr, &instr);
		_printInstr(iwin_iter, &instr);	
		iwin_iter++;	
	}
	iwinList = iwin.getCp2Iwinlist();
	iwin_iter = iwinList->begin();
	iwin_end_iter = iwinList->end();	
	fprintf(_out, "CP2 (%d): \n", iwin.cp2IwinSize());
	while(iwin_iter!=iwin_end_iter) {
		UINT raw = iwin_iter->raw();
		UINT8* pr = (UINT8 *) &raw;
		ExpInstr instr;
		_decoder.decode(pr, &instr);
		_printInstr( iwin_iter, &instr);	
		iwin_iter++;	
	}	
	fprintf(_out, "#############\n");
}

void PerfProfiler::printIWinDetail(UINT cyc, PerfIWin& iwin) {
	#ifdef _DEBUG_IWIN
	if(iwin.coreIwinSize()>=((UINT)_DEBUG_IWIN)||iwin.cp1IwinSize()>=((UINT)_DEBUG_IWIN)||iwin.cp2IwinSize()>=((UINT)_DEBUG_IWIN)) {
		_printIWinDetail(cyc, iwin);
	}	
	#endif
}

void PerfProfiler::printBufDetail(UINT cyc, PerfFetch& fetch) {
	#ifdef _DEBUG_BUF
	if(fetch.curCoreBufSize()>=((UINT)_DEBUG_BUF)||fetch.curCp1BufSize()>=((UINT)_DEBUG_BUF)||fetch.curCp2BufSize()>=((UINT)_DEBUG_BUF)) {
		_printBufDetail(cyc, fetch);
	}	
	#endif	
}

void PerfProfiler::infiniteLoopDetect(UINT cyc, PerfIWin& iwin, PerfFetch& fetch) {
	if(_coreCnt==0&&_cp1Cnt==0&&_cp2Cnt==0) {
		_loopCnt++;
		if(_loopCnt>MAX_LOOP_CNT) {
			fprintf(_out, "####Infinite loop Error:	\n");
			fprintf(_out, "Infinite loop detected at cycle = %u.\n", cyc-_loopCnt);
			fprintf(_out, "Instruction executed = %u\n", _totalIssuedCnt());
			_printBufDetail(cyc, fetch);
			_printIWinDetail(cyc, iwin);
			fflush(_out);
			if(_out!=stdout)
				fclose(_out);			
			exit(0);
		}
	}
	else {
		_loopCnt = 0;
	}
}

void PerfProfiler::_printIssueLite(UINT cyc) {

	fprintf(_out, "%09d: [%s %s %s] [%s] [%s %s %s]\n",
		cyc,
		instr_group_string[_coreItem[0].hasItem==TRUE?_coreItem[0].iwin_iter->unit():INSTR_UNDEFINED],
		instr_group_string[_coreItem[1].hasItem==TRUE?_coreItem[1].iwin_iter->unit():INSTR_UNDEFINED],
		instr_group_string[_coreItem[2].hasItem==TRUE?_coreItem[2].iwin_iter->unit():INSTR_UNDEFINED],
		
		instr_group_string[_cp1Item[0].hasItem==TRUE?_cp1Item[0].iwin_iter->unit():INSTR_UNDEFINED],
		
		instr_group_string[_cp2Item[0].hasItem==TRUE?_cp2Item[0].iwin_iter->unit():INSTR_UNDEFINED],
		instr_group_string[_cp2Item[1].hasItem==TRUE?_cp2Item[1].iwin_iter->unit():INSTR_UNDEFINED],
		instr_group_string[_cp2Item[2].hasItem==TRUE?_cp2Item[2].iwin_iter->unit():INSTR_UNDEFINED]
	);	
}

void PerfProfiler::_printIssueDetail(UINT cyc) {
	INT i;
	for(i = 0; i<CORE_MAX_ISSUE; i++) {
		if(_coreItem[i].hasItem==TRUE) {
			UINT raw = _coreItem[i].iwin_iter->raw();
			UINT8* pr = (UINT8 *) &raw;
			ExpInstr instr;
			_decoder.decode(pr, &instr);
			fprintf(_out, "   Core: ");
			_printInstr(_coreItem[i].iwin_iter, &instr);
		}	
		else {
			fprintf(_out, "   Core: --------Empty-------\n");
		}	
	}
	for(i = 0; i<CP1_MAX_ISSUE; i++) {
		if(_cp1Item[i].hasItem==TRUE) {
			UINT raw = _cp1Item[i].iwin_iter->raw();
			UINT8* pr = (UINT8 *) &raw;
			ExpInstr instr;
			_decoder.decode(pr, &instr);
			fprintf(_out, "   Cp1 : ");
			_printInstr(_cp1Item[i].iwin_iter, &instr);
		}	
		else {
			fprintf(_out, "   Cp1 : --------Empty-------\n");
		}	
	}
	for(i = 0; i<CP2_MAX_ISSUE; i++) {
		if(_cp2Item[i].hasItem==TRUE) {
			UINT raw = _cp2Item[i].iwin_iter->raw();
			UINT8* pr = (UINT8 *) &raw;
			ExpInstr instr;
			_decoder.decode(pr, &instr);
			fprintf(_out, "   Cp2 : ");
			_printInstr(_cp2Item[i].iwin_iter, &instr);
		}	
		else {
			fprintf(_out, "   Cp2 : --------Empty-------\n");
		}	
	}		
}

void PerfProfiler::checkWriteOrder(IWinItemsIter iwin_iter) {
	RegVecIter regIter = iwin_iter->dest_reg.begin();
	RegVecIter endIter = iwin_iter->dest_reg.end();
	UINT id = iwin_iter->perf_id();
	while (regIter!=endIter) {
		INT index = *regIter;
			if(id>=_reg_write_lock[index]) {
				_reg_write_lock[index] = id;
			}
			else {
				Decoder& dcd = DecoderUnit::getInstance();
				UINT raw = iwin_iter->raw();
				UINT8* pr = (UINT8 *) &raw;
				ExpInstr instr;
				dcd.decode(pr, &instr);
				fprintf(_out, "####Denpendence Error:	\n");
				_printInstr(iwin_iter, &instr);	
				fprintf(_out, "Last instr access register index %d is 0x%x\n", index, _reg_write_lock[index]);
				fprintf(_out, "Instruction executed = %u\n", _totalIssuedCnt());
				fflush(_out);
				if(_out!=stdout)
					fclose(_out);
				exit(0);
			}
		regIter++;
	}	
}



void PerfProfiler::clear() {

	if(incBufProfiling()==TRUE) {
		delete _coreBufSize;
		delete _cp1BufSize;
		delete _cp2BufSize;
	}
	
	if(incIWinProfiling()==TRUE) {
		delete _coreIWinSize;
		delete _cp1IWinSize;
		delete _cp2IWinSize;		
	}	
}

#endif
