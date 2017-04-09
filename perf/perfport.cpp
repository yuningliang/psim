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


#include "perfport.h"
#include "perfcom.h"

PerfPort::PerfPort() {
	for(INT i = 0; i <REG_SET_REG_ONLY_MAX; i++) {
		_regCurRead[i] = 0;
		_regCurWrite[i] = 0;

		_regRead[i] = 0;
		_regWrite[i] = 0;
	}
	_regMaxRead[REG_UNDEFINED] = DEFAULT_DONT_CARE_PORT;
	_regMaxWrite[REG_UNDEFINED] = DEFAULT_DONT_CARE_PORT;	
	_regMaxRead[REG_SET_GPR] = DEFAULT_CORE_READ_PORT;
	_regMaxWrite[REG_SET_GPR] = DEFAULT_CORE_WRITE_PORT;	
	_regMaxRead[REG_SET_CP1_GPR] = DEFAULT_DONT_CARE_PORT;
	_regMaxWrite[REG_SET_CP1_GPR] = DEFAULT_DONT_CARE_PORT;	
	_regMaxRead[REG_SET_CP1_CTRL] = DEFAULT_DONT_CARE_PORT;
	_regMaxWrite[REG_SET_CP1_CTRL] = DEFAULT_DONT_CARE_PORT;	
	_regMaxRead[REG_SET_CP2_SIMD] = DEFAULT_CP2RF_READ_PORT;
	_regMaxWrite[REG_SET_CP2_SIMD] = DEFAULT_CP2RF_WRITE_PORT;	
	_regMaxRead[REG_SET_CP2_SCALAR] = DEFAULT_CP2SC_READ_PORT;
	_regMaxWrite[REG_SET_CP2_SCALAR] = DEFAULT_CP2SC_WRITE_PORT;	
	_regMaxRead[REG_SET_CP2_ACC] = DEFAULT_DONT_CARE_PORT;
	_regMaxWrite[REG_SET_CP2_ACC] = DEFAULT_DONT_CARE_PORT;		
	_regMaxRead[REG_SET_CP2_CTRL] = DEFAULT_DONT_CARE_PORT;
	_regMaxWrite[REG_SET_CP2_CTRL] = DEFAULT_DONT_CARE_PORT;		
	_regMaxRead[REG_SET_CP3_SR] = DEFAULT_DONT_CARE_PORT;
	_regMaxWrite[REG_SET_CP3_SR] = DEFAULT_DONT_CARE_PORT;		

}

void PerfPort::reset() {
	for(INT i = 0; i <REG_SET_REG_ONLY_MAX; i++) {
		_regCurRead[i] = 0;
		_regCurWrite[i] = 0;

		_regRead[i] = 0;
		_regWrite[i] = 0;
	}	
}

void PerfPort::clear() {
	for(INT i = 0; i <REG_SET_REG_ONLY_MAX; i++) {
		_regCurRead[i] = 0;
		_regCurWrite[i] = 0;
	}	
}

BOOL PerfPort::isPortJam(EP_PROCESSOR cp) {
	BOOL isJam = FALSE;
	UINT portReadRequested = 0;
	UINT portWriteRequested = 0;
	switch (cp) {
		case PROCESSOR_CORE:
			portReadRequested =  _regRead[REG_SET_GPR] + _regCurRead[REG_SET_GPR];
			portWriteRequested = _regWrite[REG_SET_GPR] + _regCurWrite[REG_SET_GPR];
			if(portReadRequested>_regMaxRead[REG_SET_GPR]||portWriteRequested>_regMaxWrite[REG_SET_GPR]) {
				isJam = TRUE;
			}
			break;
		case PROCESSOR_CP1:
			break;		
		case PROCESSOR_CP2:
			portReadRequested =  _regRead[REG_SET_CP2_SIMD] + _regCurRead[REG_SET_CP2_SIMD];
			portWriteRequested = _regWrite[REG_SET_CP2_SIMD] + _regCurWrite[REG_SET_CP2_SIMD];		
			if(portReadRequested>_regMaxRead[REG_SET_CP2_SIMD]||portWriteRequested>_regMaxWrite[REG_SET_CP2_SIMD]) {
				isJam = TRUE;
			}
			else {
				portReadRequested =  _regRead[REG_SET_CP2_SCALAR] + _regCurRead[REG_SET_CP2_SCALAR];
				portWriteRequested = _regWrite[REG_SET_CP2_SCALAR] + _regCurWrite[REG_SET_CP2_SCALAR];	
				if(portReadRequested>_regMaxRead[REG_SET_CP2_SCALAR]||portWriteRequested>_regMaxWrite[REG_SET_CP2_SCALAR]) {
					isJam = TRUE;
				}				
			}
			break;
		default:
			IsTrue((0), ("Undefined processor.\n"));		
	}	
	return isJam;
}

void PerfPort::lock(EP_PROCESSOR cp) {
	switch (cp) {
		case PROCESSOR_CORE:
			_regRead[REG_SET_GPR] += _regCurRead[REG_SET_GPR];
			_regWrite[REG_SET_GPR] += _regCurWrite[REG_SET_GPR];
			break;
		case PROCESSOR_CP1:
			break;		
		case PROCESSOR_CP2:
			_regRead[REG_SET_CP2_SIMD] += _regCurRead[REG_SET_CP2_SIMD];
			_regWrite[REG_SET_CP2_SIMD] += _regCurWrite[REG_SET_CP2_SIMD];		
			_regRead[REG_SET_CP2_SCALAR] += _regCurRead[REG_SET_CP2_SCALAR];
			_regWrite[REG_SET_CP2_SCALAR] += _regCurWrite[REG_SET_CP2_SCALAR];	
			break;
		default:
			IsTrue((0), ("Undefined processor.\n"));		
	}	
}
