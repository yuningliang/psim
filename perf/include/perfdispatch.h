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


#ifndef PERFDISPATCH_H_
#define PERFDISPATCH_H_

#include "perfiwin.h"
#include "perfdatadep.h"
#include "perfbranch.h"
#include "perfengine.h"
#include "perforder.h"
#include "perfdefs.h"

class PerfDispatch {
	private:
		PerfIWin& _iwin;
		PerfDataDep& _dataCheck;
		PerfBranch& _branch;
		PerfEngine& _engine;
		PerfOrder& _order;
		UINT _totalIssued;
		
		IWinItemsIter coreIssued[];
		IWinItemsIter cp1Issued[];
		IWinItemsIter cp2Issued[];
		
		UINT _coreCurIssued;
		UINT _cp1CurIssued;
		UINT _cp2CurIssued;
		
	private:
		void _issueCore();
		void _issueCp1();
		void _issueCp2();
		void _issueCp2Macro();
		BOOL _issueXfer(UINT xferID);
		ES_HAZARD_STATUS _isValid(IWinItemsIter iwin_iter, EP_PROCESSOR cp, EU_CPU_UNIT* unit_ptr);
		void _lockResource(IWinItemsIter iwin_iter, EU_CPU_UNIT unit, EP_PROCESSOR cp);
		
	public:	
		PerfDispatch(	PerfIWin& iwin,
						PerfDataDep& dataCheck,
						PerfBranch& branch,
						PerfEngine& engine,
						PerfOrder& order);
		void dispatch(UINT cyc);
		void update();
		UINT totalIssued() { return _totalIssued; }
	
};

_INLINE void PerfDispatch::_lockResource(IWinItemsIter iwin_iter, EU_CPU_UNIT unit, EP_PROCESSOR cp) {
	_dataCheck.lock(iwin_iter, cp);
	_engine.lockEngine(unit, cp);
	_order.lock(iwin_iter->unit());	
	_totalIssued++;
}

_INLINE ES_HAZARD_STATUS PerfDispatch::_isValid(IWinItemsIter iwin_iter, EP_PROCESSOR cp, EU_CPU_UNIT* unit_ptr) {
	EU_INSTR_GROUP instr_unit = iwin_iter->unit();
	ES_HAZARD_STATUS retStatus = HS_NONE;
	ES_HAZARD_STATUS regStatus = _dataCheck.check(iwin_iter, cp);
	ES_HAZARD_STATUS brStatus = _branch.checkBranch(iwin_iter);
	ES_HAZARD_STATUS orderStatus = _order.checkOrder(instr_unit);
	if(brStatus!=HS_NONE) {
		retStatus = brStatus;
	}
	else if(orderStatus!=HS_NONE) {
		retStatus = orderStatus;
	}	
	else if(regStatus!=HS_NONE) {
		retStatus = regStatus;
	}	
	else {
		EU_CPU_UNIT unit = _engine.checkReadyUnit(instr_unit, iwin_iter->instr_id(), cp);
		if(unit==UNIT_PROCESSOR_FULL) {
			retStatus = HS_ENGINE_FULL;
		}	
		else if(unit==UNIT_NOT_READY) {
			retStatus = HS_ENGINE_NOT_READY;
		}
		else {
			*unit_ptr = unit;
		}
	}
	iwin_iter->hazard(retStatus);
	return retStatus;
}

#endif /*PERFDISPATCH_H_*/
