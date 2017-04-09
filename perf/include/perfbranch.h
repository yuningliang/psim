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


#ifndef PERFBRANCH_H_
#define PERFBRANCH_H_

#include "perfdefs.h"
#include "perfiwinitem.h"

class PerfBranch {
	private:
		UINT _missBr;
		UINT _totalBr;
		UINT _chmodCnt;
		UINT _callCnt;
		UINT _otherJr;
		UINT _excepCnt;
		
		BOOL _incExcepPenalty;
		BOOL _incBrPenalty;
		BOOL _incJrPenalty;
		BOOL _incSpeculated;
		
		UINT _lastBrID;
		
		INT _jrCyc;
		INT _brCyc;
		INT _excepCyc;
		INT _chmodJCyc;
		INT _chmodNCyc;
		INT _basicReolveCyc;
	
	private:
			
	public:
		PerfBranch();
		ES_HAZARD_STATUS checkBranch(IWinItemsIter iwin_iter);
		void branchLatency(PerfIWinItem* item);
		INT speculatedID(BufItem item, UINT id, EINSTR_TYPE type);
		
		UINT lastBrID(void) { return _lastBrID; }
		void lastBrID(IWinItemsIter iwin_iter);
		
		UINT missBr(void) { return _missBr; }
		UINT totalBr(void) { return _totalBr; }
		UINT totalJr(void) { return (_chmodCnt + _callCnt + _otherJr + _excepCnt); }
		UINT callCnt(void) { return _callCnt; }
		UINT otherJr(void) { return _otherJr; }
		UINT chmodCnt(void) { return _chmodCnt; }
		UINT excepCnt(void) { return _excepCnt; }

		BOOL incSpeculated(void) { return _incSpeculated; }
		void incSpeculated(BOOL t) { _incSpeculated = t; }
		BOOL incBrPenalty(void) { return _incBrPenalty; }
		void incBrPenalty(BOOL t) { _incBrPenalty = t; }			
		BOOL incJrPenalty(void) { return _incJrPenalty; }
		void incJrPenalty(BOOL t) { _incJrPenalty = t; }	
		BOOL incExcepPenalty(void) { return _incExcepPenalty; }
		void incExcepPenalty(BOOL t) { _incExcepPenalty = t; }
		
		void excepCyc(INT c) { _excepCyc = c; }
		INT excepCyc(void) { return (_excepCyc+_basicReolveCyc); }
		void jrCyc(INT c) { _jrCyc = c; }
		INT jrCyc(void) { return (_jrCyc+_basicReolveCyc); }
		void brCyc(INT c) { _brCyc = c; }
		INT brCyc(void) { return (_brCyc+_basicReolveCyc); }
		void chmodJCyc(INT c) { _chmodJCyc = c; }
		INT chmodJCyc(void) { return (_chmodJCyc+_basicReolveCyc); }
		void chmodNCyc(INT c) { _chmodNCyc = c; }
		INT chmodNCyc(void) { return (_chmodNCyc+_basicReolveCyc); }				
		void basicReolveCyc(INT c) { _basicReolveCyc = c; }
		INT basicReolveCyc(void) { return _basicReolveCyc; }
};

_INLINE ES_HAZARD_STATUS PerfBranch::checkBranch(IWinItemsIter iwin_iter) {
	if(iwin_iter->spec_id()>_lastBrID) {
		return HS_BR_STALL;
	}
	else {
		return HS_NONE;
	}
	
}


#endif /*PERFBRANCH_H_*/
