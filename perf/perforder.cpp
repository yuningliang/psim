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


#include "perforder.h"

PerfOrder::PerfOrder() {
	INT i;
	for(i = 0; i<MAX_PIPE_GROUP; i++) {
		_orderStatus[i].access = FALSE;
		_orderStatus[i].cycle = 0;
	}
	_incLSorder = TRUE;
	_incCP2order = TRUE;
	_incXferPipeClear = TRUE;	
	_incSynLSorder = TRUE;
}

void PerfOrder::reset() {
	INT i;
	for(i = 0; i<MAX_PIPE_GROUP; i++) {
		if(_orderStatus[i].cycle>0) {
			_orderStatus[i].cycle -= 1;
		}
		_orderStatus[i].access = FALSE;
	}
}

void PerfOrder::xferReset() {
	//reset core groups only
	_orderStatus[GROUP_RISC_LS1].access = FALSE;
	_orderStatus[GROUP_RISC_LS2].access = FALSE;
}

ES_HAZARD_STATUS PerfOrder::checkOrder(EU_INSTR_GROUP unit) {
	BOOL t = TRUE;
	switch (unit) {
		case INSTR_RISC_ALU:
			return HS_NONE;
		case INSTR_CP1_LS:
		case INSTR_CP1_ALU:
		case INSTR_CP1_XFER:
		case INSTR_CP1_BR:
			return HS_NONE;
			
		case INSTR_RISC_LS:
		case INSTR_CP3_LS:
			if(incLSorder()==TRUE)
				return _isValidLS(FALSE)==TRUE?HS_NONE:HS_INSTR_ORDER;
			else 
				return HS_NONE;
		case INSTR_RISC_CP1_LS:
		case INSTR_RISC_CP2_LS:	
			if(incSynLSorder()==TRUE) {
				t = incLSorder()==TRUE?_isValidLS(TRUE):TRUE;
			}
			else {
				t = TRUE;
			}
			if(t==TRUE&&incXferPipeClear()==TRUE) {
				return _isValidXfer()==TRUE?HS_NONE:HS_XFER_PIPE_NOT_CLEAR;
			}
			else {
				return t==TRUE?HS_NONE:HS_INSTR_ORDER;
			}					
		case INSTR_RISC_CP1_XFER:
		case INSTR_RISC_CP2_XFER:
			t = incLSorder()==TRUE?_isValidLS(TRUE):TRUE;
			if(t==TRUE&&incXferPipeClear()==TRUE) {
				return _isValidXfer()==TRUE?HS_NONE:HS_XFER_PIPE_NOT_CLEAR;
			}
			else {
				return t==TRUE?HS_NONE:HS_INSTR_ORDER;
			}	
		case INSTR_CP3_XFER:	
			return HS_NONE;

		case INSTR_RISC_BR:
		case INSTR_CP3_BR:
			return HS_NONE;

		case INSTR_CP2_SCALAR_SUM4:
			return _isValidCP2(GROUP_CP2_SCALAR_SUM4)==TRUE?HS_NONE:HS_INSTR_ORDER;

		case INSTR_CP2_BR:		
		case INSTR_CP2_SCALAR_MAD:
			return _isValidCP2(GROUP_CP2_SCALAR_MAD)==TRUE?HS_NONE:HS_INSTR_ORDER;	

		case INSTR_CP2_SCALAR_BIT:
			return _isValidCP2(GROUP_CP2_SCALAR_BIT)==TRUE?HS_NONE:HS_INSTR_ORDER;

		case INSTR_CP2_SCALAR_SCAN:
			return _isValidCP2(GROUP_CP2_SCALAR_SCAN)==TRUE?HS_NONE:HS_INSTR_ORDER;

		case INSTR_CP2_SIMD:
			return _isValidCP2(GROUP_CP2_SIMD)==TRUE?HS_NONE:HS_INSTR_ORDER;

		case INSTR_CP2_XFER_INT:
		case INSTR_CP2_XFER_EXT:
		case INSTR_CP2_LS:
			return _isValidCP2(GROUP_CP2_XFER)==TRUE?HS_NONE:HS_INSTR_ORDER;

				
		default:
			IsTrue((0), ("PerfOrder: Undefined instruction group.\n"));		
	}	
	return HS_NONE;
}

void PerfOrder::lock(EU_INSTR_GROUP unit) {
	switch (unit) {
		case INSTR_RISC_ALU:
			break;
		case INSTR_CP1_LS:
		case INSTR_CP1_ALU:
		case INSTR_CP1_XFER:
		case INSTR_CP1_BR:
			break;		
		case INSTR_RISC_LS:
		case INSTR_CP3_LS:
			if(_orderStatus[GROUP_RISC_LS1].cycle<PIPE_CORE_CYCLE)
				_orderStatus[GROUP_RISC_LS1].cycle = PIPE_CORE_CYCLE;
			else
				_orderStatus[GROUP_RISC_LS2].cycle = PIPE_CORE_CYCLE;
			break;
		case INSTR_RISC_CP1_LS:
		case INSTR_RISC_CP2_LS:
		case INSTR_RISC_CP1_XFER:
		case INSTR_RISC_CP2_XFER:
		case INSTR_CP3_XFER:	
			break;	
		case INSTR_RISC_BR:
		case INSTR_CP3_BR:
			break;	
		case INSTR_CP2_SCALAR_SUM4:
			break;	
		case INSTR_CP2_BR:		
		case INSTR_CP2_SCALAR_MAD:
			break;			
		case INSTR_CP2_SCALAR_BIT:
			break;			
		case INSTR_CP2_SCALAR_SCAN:
			break;				
		case INSTR_CP2_SIMD:
			break;			
		case INSTR_CP2_XFER_INT:
		case INSTR_CP2_XFER_EXT:
		case INSTR_CP2_LS:
			break;			
		default:
			IsTrue((0), ("PerfOrder: Undefined instruction group.\n"));		
	}	
}


