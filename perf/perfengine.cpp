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


#include "perfengine.h"

PerfEngine::PerfEngine() {
	_totalEngineLocked = 0;
	reset();
}

void PerfEngine::reset() {
	_units = 0;
	_coreUsedUnits = 0;
	_cp1UsedUnits = 0;
	_cp2UsedUnits = 0;
}

EU_CPU_UNIT PerfEngine::checkReadyUnit(EU_INSTR_GROUP unit, UINT id, EP_PROCESSOR cp) {
	
	//Check # used engine
	switch (cp) {
		case PROCESSOR_CORE:
			if(_coreUsedUnits>=CORE_MAX_ISSUE) {
				return UNIT_PROCESSOR_FULL;
			}
			break;
		case PROCESSOR_CP1:
			if(_cp1UsedUnits>=CP1_MAX_ISSUE) {
				return UNIT_PROCESSOR_FULL;
			}		
			break;		
		case PROCESSOR_CP2:
			if(_cp2UsedUnits>=CP2_MAX_ISSUE) {
				return UNIT_PROCESSOR_FULL;
			}	
			break;
		default:
			IsTrue((0), ("PerfEngine: Undefined processor.\n"));		
	}
		
	//Check engine ready
	switch (unit) {
		case INSTR_RISC_ALU:
			switch (id) {
				case core_mfhi_id:
				case core_mflo_id:
				case core_mult_id:
				case core_multu_id:	
					if(_isReady(RISC_ALU_LS)==TRUE) {
						return RISC_ALU_LS;
					}
					break;
				default:				
					if(_isReady(RISC_ALU)==TRUE) {
						return RISC_ALU;
					}
					else if(_isReady(RISC_ALU_LS)==TRUE) {
						return RISC_ALU_LS;
					}
					break;
			}
			break;
		case INSTR_CP1_LS:
		case INSTR_CP1_ALU:
		case INSTR_CP1_XFER:
		case INSTR_CP1_BR:
			if(_isReady(CP1_FALU)==TRUE) {
				return CP1_FALU;
			}		
			break;		
		case INSTR_RISC_LS:
			if(_isReady(RISC_LS_BR)==TRUE) {
				return RISC_LS_BR;
			}
			else if(_isReady(RISC_ALU_LS)==TRUE) {
				return RISC_ALU_LS;
			}
			break;
		case INSTR_RISC_BR:
		case INSTR_CP3_BR:
			if(_isReady(RISC_LS_BR)==TRUE) {
				return RISC_LS_BR;
			}
			break;	
		case INSTR_CP2_SCALAR_SUM4:
			if(_isReady(CP2_SCALAR_SUM4)==TRUE) {
				return CP2_SCALAR_SUM4;
			}
			break;	
		case INSTR_CP2_BR:		
		case INSTR_CP2_SCALAR_MAD:
			if(_isReady(CP2_SCALAR_MAD)==TRUE) {
				return CP2_SCALAR_MAD;
			}
			break;			
		case INSTR_CP2_SCALAR_BIT:
			if(_isReady(CP2_SCALAR_BIT)==TRUE) {
				return CP2_SCALAR_BIT;
			}
			break;			
		case INSTR_CP2_SCALAR_SCAN:
			if(_isReady(CP2_SCALAR_SCAN)==TRUE) {
				return CP2_SCALAR_SCAN;
			}
			break;	
		case INSTR_CP2_SIMD:
			if(_isReady(CP2_SIMD)==TRUE) {
				return CP2_SIMD;
			}
			break;
		case INSTR_CP2_XFER_INT:
		case INSTR_CP2_XFER_EXT:
		case INSTR_CP2_LS:
			if(_isReady(CP2_XFER)==TRUE) {
				return CP2_XFER;
			}
			break;

		case INSTR_RISC_CP1_LS:
		case INSTR_RISC_CP2_LS:		
		case INSTR_CP3_LS:
		case INSTR_RISC_CP1_XFER:
		case INSTR_RISC_CP2_XFER:
		case INSTR_CP3_XFER:	
			if(_isReady(RISC_ALU_LS)==TRUE) {
				return RISC_ALU_LS;
			}
			break;					
		default:
			IsTrue((0), ("PerfEngine: Undefined engine unit.\n"));		
	}	
	return UNIT_NOT_READY;
}

void PerfEngine::lockEngine(EU_CPU_UNIT unit, EP_PROCESSOR cp) {
	_totalEngineLocked++;
	switch (cp) {
		case PROCESSOR_CORE:
			_lockUnit(unit);
			_coreUsedUnits++;
			break;
		case PROCESSOR_CP1:
			_lockUnit(unit);
			_cp1UsedUnits++;
			break;		
		case PROCESSOR_CP2:
			_lockUnit(unit);
			_cp2UsedUnits++;
			break;
		default:
			IsTrue((0), ("PerfEngine: Undefined processor.\n"));		
	}	
}
