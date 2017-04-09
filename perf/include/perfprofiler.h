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


#ifndef PERFPROFILER_H_
#define PERFPROFILER_H_



#ifdef _INCLUDE_PROFILER
#include "perfmain.h"
#include "perfdispatch.h"
#include "perffetch.h"
#include "perfiwin.h"
#include "perfdefs.h"
#include "perforder.h"
#include "perfcache.h"

#define _HAS_PROFILER 1

#define _INSTR_MONITOR 1

struct perfprofiler_eqaddr {
	bool operator() (ADDR a1, ADDR a2)
  	{
    	return (a1==a2);
	}
};

typedef struct profiler_instr_Item {BOOL hasItem; IWinItemsIter iwin_iter;} InstrItem;

class PerfProfiler {
	private:
		Decoder& _decoder;
		FILE *_out;
		FILE *_icache_out;
		
		BOOL _isPrintfull;
		BOOL _isPrintlite;
		BOOL _isPrintDetailCnt;
		BOOL _incBufProfiling;
		BOOL _incIWinProfiling;		
		BOOL _incIWinFull;
		BOOL _incIssueAnalysis;
		BOOL _incFetchHiddenCyc;
		BOOL _psimInstrMode;
		UINT _linePerPrint;
		
		UINT _reg_write_lock[MAX_REG_INDEX];
		InstrItem _coreItem[CORE_MAX_ISSUE];
		InstrItem _cp1Item[CP1_MAX_ISSUE];
		InstrItem _cp2Item[CP2_MAX_ISSUE];
		UINT _instrCnt[INSTR_MAX];
		UINT _iwinFullCnt[HS_IWIN_FULL_MAX];
		UINT _issueAnalysisCnt[HS_IWIN_FULL_MAX];
		UINT _coreIssueAnalysisCnt[CORE_MAX_ISSUE+1];
		UINT _cp2IssueAnalysisCnt[CP2_MAX_ISSUE+1];
		UINT _coreIWinFullCnt;
		UINT _cp2IWinFullCnt;
		UINT _totalCoreCnt;
		UINT _totalCp1Cnt;
		UINT _totalCp2Cnt;
		UINT _totalucodeCnt;
		UINT _fetchStallHiddenCyc;
		UINT _fetchStallAccuCyc;

		UINT* _coreBufSize;
		UINT* _cp1BufSize;
		UINT* _cp2BufSize;
		UINT* _coreIWinSize;
		UINT* _cp1IWinSize;
		UINT* _cp2IWinSize;
		
		#if _INSTR_MONITOR
		EU_INSTR_GROUP _monitorType;
		UINT _monitorProcessor;
		UINT _monitorLastIssue;
		UINT _monitorCnt;
		UINT _monitorAnalysisCnt[HS_IWIN_FULL_MAX];
		UINT _monitorSameInstr;
		#endif
		
		//current cycle issued #
		UINT _coreCnt;
		UINT _cp1Cnt;
		UINT _cp2Cnt;
		
		UINT _loopCnt;

		
	private:
		void _printIssueLite(UINT cyc);
		void _printIssueDetail(UINT cyc);	
		void _printIWinDetail(UINT cyc, PerfIWin& iwin);
		void _printBufDetail(UINT cyc, PerfFetch& fetch);	
		void _printInstrGroupDetail();	
		UINT _totalIssuedCnt() { return (_totalCoreCnt + _totalCp1Cnt + _totalCp2Cnt); }
		void _printInstr(IWinItemsIter iwin_iter, ExpInstr* instr);
		void _monitorInstrType(PerfIWin& iwin);
	
	public:
		PerfProfiler();
		BOOL psimInstrMode() { return _psimInstrMode; }
		void psimInstrMode(BOOL t) {_psimInstrMode = t;}		
		BOOL isPrintfull() { return _isPrintfull; }
		void isPrintfull(BOOL t) {_isPrintfull = t;}
		BOOL isPrintlite() { return _isPrintlite; }		
		void isPrintlite(BOOL t) { _isPrintlite = t;}	
		BOOL isPrintDetailCnt() { return _isPrintDetailCnt; }		
		void isPrintDetailCnt(BOOL t) { _isPrintDetailCnt = t;}			
		UINT linePerPrint() { return _linePerPrint; }		
		BOOL incBufProfiling(void) { return _incBufProfiling; }
		void incBufProfiling(BOOL t) { _incBufProfiling = t; }
		BOOL incIWinProfiling(void) { return _incIWinProfiling; }
		void incIWinProfiling(BOOL t) { _incIWinProfiling = t; }	
		BOOL incIWinFull(void) { return _incIWinFull; }
		void incIWinFull(BOOL t) { _incIWinFull = t; }
		BOOL incIssueAnalysis(void) { return _incIssueAnalysis; }
		void incIssueAnalysis(BOOL t) { _incIssueAnalysis = t; }
		BOOL incFetchHiddenCyc(void) { return _incFetchHiddenCyc; }
		void incFetchHiddenCyc(BOOL t) { _incFetchHiddenCyc = t; }		
		
		void linePerPrint(UINT c) { _linePerPrint = c; }	
		void checkWriteOrder(IWinItemsIter iwin_iter);
		void checkIWinFull(PerfIWin& iwin);
		void checkIssue(PerfIWin& iwin, UINT coreIssued, UINT cp2Issued);
		void checkFetchHiddenCyc(PerfFetch& fetch);
		void infiniteLoopDetect(UINT cyc, PerfIWin& iwin, PerfFetch& fetch);
		void printSetting(PerfOrder& order, PerfBranch& branch, PerfEngine& engine, 
							 PerfIWin& iwin, PerfDataDep& dataCheck, PerfDispatch& dispatch,
							 PerfFetch& fetch, PerfCache& cache);		
		void printFullReport(UINT cyc, PerfOrder& order, PerfBranch& branch, PerfEngine& engine, 
							 PerfIWin& iwin, PerfDataDep& dataCheck, PerfDispatch& dispatch,
							 PerfFetch& fetch, PerfCache& cache);
		void printLineReport(FILE*line_out, UINT cyc, PerfOrder& order, PerfBranch& branch, PerfEngine& engine, 
							 PerfIWin& iwin, PerfDataDep& dataCheck, PerfDispatch& dispatch,
							 PerfFetch& fetch, PerfCache& cache);
		
		void printIWinFullResult(UINT cyc);		
		void printIssueResult(UINT cyc);		
		void printIWinFullResultLine(UINT cyc);		
		void printIssueResultLine(UINT cyc);	
		void printICacheDetailLine(PerfCache& cache, UINT cyc);				 
		void reset();
		void init(PerfIWin& iwin, PerfFetch& fetch);

		void printIssue(UINT cyc);		
		void setCore(IWinItemsIter iwin_iter);
		void setCp1(IWinItemsIter iwin_iter);
		void setCp2(IWinItemsIter iwin_iter);
		void out(FILE* o) { _out = o; }

		void iwinSizeProfiling(PerfIWin& iwin);
		void iwinFullProfiling(PerfIWin& iwin);
		void printIWinDetail(UINT cyc, PerfIWin& iwin);
		void printBufDetail(UINT cyc, PerfFetch& fetch);
		void bufSizeProfiling(PerfFetch& fetch);
		void clear();
				
};


_INLINE void PerfProfiler::printIssue(UINT cyc) {
	if(cyc%linePerPrint()==0) {
		if(isPrintlite()==TRUE) {
			_printIssueLite(cyc);
		}
		if(isPrintfull()==TRUE) {
			if(isPrintlite()!=TRUE) {
				fprintf(stdout, "Cycle: %09d\n", cyc);
			}
			_printIssueDetail(cyc);
		}
	}
}	
	
_INLINE void PerfProfiler::setCore(IWinItemsIter iwin_iter) {
	_coreItem[_coreCnt].hasItem = TRUE;
	_coreItem[_coreCnt].iwin_iter = iwin_iter;
	_instrCnt[iwin_iter->unit()]+=1;
	_coreCnt++;
}

_INLINE void PerfProfiler::setCp1(IWinItemsIter iwin_iter) {
	_cp1Item[_cp1Cnt].hasItem = TRUE;
	_cp1Item[_cp1Cnt].iwin_iter = iwin_iter;	
	_instrCnt[iwin_iter->unit()]+=1;
	_cp1Cnt++;			
}

_INLINE void PerfProfiler::setCp2(IWinItemsIter iwin_iter) {
	_cp2Item[_cp2Cnt].hasItem = TRUE;
	_cp2Item[_cp2Cnt].iwin_iter = iwin_iter;		
	_instrCnt[iwin_iter->unit()]+=1;		
	if(iwin_iter->type()==ISSUE_TYPE_UCODE) {
		_totalucodeCnt++;
	}
	_cp2Cnt++;			
}

extern PerfProfiler profiler;
#define PROFILER_PSIM_INSTR_MODE(t) profiler.psimInstrMode(t);
#define PROFILER_INIT(iwin, fetch) profiler.init(iwin, fetch);
#define PROFILER_END() profiler.end();
#define PROFILER_CHECK_IWIN_FULL(iwin) profiler.checkIWinFull(iwin);
#define PROFILER_CHECK_FETCH_HIDDEN_CYC(fetch) profiler.checkFetchHiddenCyc(fetch);
#define PROFILER_CHECK_ISSUE(iwin, coreIssued, cp2Issued) profiler.checkIssue(iwin, coreIssued, cp2Issued);
#define PROFILER_LOG_CORE_ISSUED(iwin_iter) profiler.setCore(iwin_iter);
#define PROFILER_LOG_CP1_ISSUED(iwin_iter) profiler.setCp1(iwin_iter);
#define PROFILER_LOG_CP2_ISSUED(iwin_iter) profiler.setCp2(iwin_iter);
#define PROFILER_PRINT_ISSUE(cyc) profiler.printIssue(cyc);
#define PROFILER_LOG_IWIN(iwin) profiler.iwinSizeProfiling(iwin);
#define PROFILER_LOG_BUF(fetch) profiler.bufSizeProfiling(fetch);
#define PROFILER_RESET profiler.reset();
#define PROFILER_PRINT_FULL(t) profiler.isPrintfull(t);
#define PROFILER_PRINT_LITE(t) profiler.isPrintlite(t);
#define PROFILER_LINE_PER_PRINT(line) profiler.linePerPrint(line);
#define PROFILER_PRINT_FULL_REPORT(cyc, order, branch, engine, iwin, dataCheck, dispatch, fetch, cache) profiler.printFullReport(cyc, order, branch, engine, iwin, dataCheck, dispatch, fetch, cache);
#define PROFILER_PRINT_LINE_REPORT(line_out, cyc, order, branch, engine, iwin, dataCheck, dispatch, fetch, cache) profiler.printLineReport(line_out, cyc, order, branch, engine, iwin, dataCheck, dispatch, fetch, cache);
#define PROFILER_PRINT_OUT(o) profiler.out(o);
#define PROFILER_PRINT_IWIN_FULL(cyc) profiler.printIWinFullResult(cyc);
#define PROFILER_PRINT_ISSUE_ANALYSIS(cyc) profiler.printIssueResult(cyc);
#define PROFILER_PRINT_IWIN_FULL_LINE(cyc) profiler.printIWinFullResultLine(cyc);
#define PROFILER_PRINT_ISSUE_ANALYSIS_LINE(cyc) profiler.printIssueResultLine(cyc);
#define PROFILER_PRINT_FETCH_HIDDEN_CYC_LINE(cache, cyc) profiler.printICacheDetailLine(cache, cyc);
#define PROFILER_PRINT_INSTR_DIST(t) profiler.isPrintDetailCnt(t);
#define PROFILER_CLEAR profiler.clear();

#define PROFILER_IWIN_PROFILING(t) profiler.incIWinProfiling(t);
#define PROFILER_IWIN_FULL(t) profiler.incIWinFull(t);
#define PROFILER_ISSUE_ANALYSIS(t) profiler.incIssueAnalysis(t);
#define PROFILER_FETCH_HIDDEN_CYC(t) profiler.incFetchHiddenCyc(t);
#define PROFILER_FETCH_PROFILING(t) profiler.incBufProfiling(t);

#define PROFILER_NOT_ENABLE_WARMING(str)

#ifdef _DEBUG_IWIN
	#define PROFILER_DEBUG_IWIN(cyc, iwin) profiler.printIWinDetail(cyc, iwin);
#else
	#define PROFILER_DEBUG_IWIN(cyc, iwin)
#endif
#ifdef _DEBUG_BUF
	#define PROFILER_DEBUG_FETCH(cyc, fetch) profiler.printBufDetail(cyc, fetch);
#else
	#define PROFILER_DEBUG_FETCH(cyc, fetch)
#endif

#ifdef _DEBUG_DEPENDENCE
	#define PROFILER_CHECK_WRITE_ORDER(iwin_iter) profiler.checkWriteOrder(iwin_iter);
#else
	#define PROFILER_CHECK_WRITE_ORDER(iwin_iter)
#endif

#ifdef _DEBUG_LOOP_CNT
	#define PROFILER_CHECK_LOOP(cyc, iwin, fetch) profiler.infiniteLoopDetect(cyc, iwin, fetch);
#else
	#define PROFILER_CHECK_LOOP(cyc, iwin, fetch)
#endif

#else
//non-profiler build dummy #define
#define _HAS_PROFILER 0
#define PROFILER_PSIM_INSTR_MODE(t)
#define PROFILER_INIT(iwin, fetch)
#define PROFILER_END()
#define PROFILER_CHECK_IWIN_FULL(iwin)
#define PROFILER_CHECK_ISSUE(iwin, coreIssued, cp2Issued)
#define PROFILER_CHECK_FETCH_HIDDEN_CYC(fetch)
#define PROFILER_LOG_BUF(fetch)
#define PROFILER_LOG_IWIN(iwin)
#define PROFILER_LOG_CORE_ISSUED(item)
#define PROFILER_LOG_CP1_ISSUED(item)
#define PROFILER_LOG_CP2_ISSUED(item)
#define PROFILER_CHECK_WRITE_ORDER(iwin_iter)
#define PROFILER_PRINT_ISSUE(cyc)
#define PROFILER_RESET
#define PROFILER_PRINT_FULL(t)
#define PROFILER_PRINT_LITE(t)
#define PROFILER_LINE_PER_PRINT(line)
#define PROFILER_PRINT_FULL_REPORT(cyc, order, branch, engine, iwin, dataCheck, dispatch, fetch, cache)
#define PROFILER_PRINT_LINE_REPORT(cyc, order, branch, engine, iwin, dataCheck, dispatch, fetch, cache)
#define PROFILER_PRINT_OUT(o)
#define PROFILER_PRINT_IWIN_FULL(cyc) 
#define PROFILER_PRINT_ISSUE_ANALYSIS(cyc)
#define PROFILER_PRINT_IWIN_FULL_LINE(cyc)
#define PROFILER_PRINT_ISSUE_ANALYSIS_LINE(cyc)
#define PROFILER_PRINT_FETCH_HIDDEN_CYC_LINE(cache, cyc)
#define PROFILER_PRINT_INSTR_DIST(t)
#define PROFILER_CLEAR
#define PROFILER_IWIN_PROFILING(t)
#define PROFILER_IWIN_FULL(t)
#define PROFILER_ISSUE_ANALYSIS(t)
#define PROFILER_FETCH_PROFILING(t)
#define PROFILER_FETCH_HIDDEN_CYC(t)
#define PROFILER_NOT_ENABLE_WARMING(str) fprintf(stdout, "No effect profiler-built option (%s)\n"); 
#define PROFILER_DEBUG_IWIN(cycle, iwin)
#define PROFILER_DEBUG_FETCH(cyc, fetch)
#define PROFILER_CHECK_LOOP(cyc, iwin, fetch)
#endif

#endif /*PERFPROFILER_H_*/
