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

#ifndef PERFORDER_H_
#define PERFORDER_H_

#include "perfdefs.h"

class PerfOrder {
	private:
		BOOL _incLSorder;
		BOOL _incCP2order;
		BOOL _incXferPipeClear;
		BOOL _incSynLSorder;
		OrderStatus _orderStatus[MAX_PIPE_GROUP];
	
	private:
		BOOL _isValidLS(BOOL isXfer);
		BOOL _isValidCP2(EP_PIPE_ORDER);
		BOOL _isValidXfer();
		
	public:	
		PerfOrder();
		void reset();	
		void xferReset();	
		ES_HAZARD_STATUS checkOrder(EU_INSTR_GROUP unit);
		void lock(EU_INSTR_GROUP unit);
		void incLSorder(BOOL t){ _incLSorder = t; }
		BOOL incLSorder() { return _incLSorder; }
		void incCP2order(BOOL t) { _incCP2order = t; }
		BOOL incCP2order() { return _incCP2order; }
		void incXferPipeClear(BOOL t) { _incXferPipeClear = t; }
		BOOL incXferPipeClear() { return _incXferPipeClear; }	
		void incSynLSorder(BOOL t) { _incSynLSorder = t; }
		BOOL incSynLSorder() { return _incSynLSorder; }			
};

_INLINE BOOL PerfOrder::_isValidCP2(EP_PIPE_ORDER group) {
	if(_orderStatus[group].access==TRUE) {
		return FALSE;
	}
	else {
		_orderStatus[group].access = TRUE;
		return TRUE;
	}
}

_INLINE BOOL PerfOrder::_isValidLS(BOOL isXfer) {
	if(isXfer==FALSE) {
		if(_orderStatus[GROUP_RISC_LS1].access==TRUE) {
			if(_orderStatus[GROUP_RISC_LS2].access==TRUE) {
				return FALSE;
			}
			else {
				_orderStatus[GROUP_RISC_LS2].access = TRUE;
				return TRUE;
			}				
		}
		else {
			_orderStatus[GROUP_RISC_LS1].access = TRUE;
			return TRUE;
		}	
	}
	else {
		BOOL ret = TRUE;
		if(_orderStatus[GROUP_RISC_LS1].access==TRUE||_orderStatus[GROUP_RISC_LS2].access==TRUE) {
			ret = FALSE;
		}
		_orderStatus[GROUP_RISC_LS1].access = TRUE;
		_orderStatus[GROUP_RISC_LS2].access = TRUE;		
		return ret;
	}
}

_INLINE BOOL PerfOrder::_isValidXfer() {
	if(_orderStatus[GROUP_RISC_LS1].cycle>PIPE_ALL_READY||_orderStatus[GROUP_RISC_LS2].cycle>PIPE_ALL_READY) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

#endif /*PERFORDER_H_*/
