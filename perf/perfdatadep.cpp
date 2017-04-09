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


#include "perfdatadep.h"
#include "perfcom.h"

#ifdef _DEBUG_DEPENDENCE
	#include "decode.h"
	#include "tool/prompt.h"
#endif

PerfDataDep::PerfDataDep() {
	INT i;
	for(i = 0; i<MAX_REG_INDEX; i++) {
		_reg_use[i].request = 0;
		_reg_use[i].lock = 0;
		_reg_define[i].request = 0;
		_reg_define[i].lock = 0;
		_reg_status[i].cycle = 0;
		_reg_status[i].type = RS_READY;
	}
}

ES_REG_STATUS PerfDataDep::_core_reg_access_type(EU_INSTR_GROUP unit, UINT id) {
	switch(unit)
	{
		case INSTR_RISC_LS:
		case INSTR_CP3_LS:
			return RS_CORE_WB;
		case INSTR_RISC_CP1_LS:
		case INSTR_RISC_CP2_LS:
			return RS_CORE_WB;
		case INSTR_RISC_CP1_XFER:
		case INSTR_RISC_CP2_XFER:
		case INSTR_CP3_XFER:
			return RS_CORE_WB;
		
		case INSTR_RISC_ALU:
			switch (id) {
				case core_mfhi_id:
				case core_mflo_id:
					return RS_CORE_WB;
				default:				
					return RS_CORE_NORMAL;
			}		

		case INSTR_RISC_BR:
		case INSTR_CP3_BR:
			return RS_CORE_NORMAL;

		//CP1 no reg by-pass
		case INSTR_CP1_LS:
		case INSTR_CP1_ALU:
		case INSTR_CP1_XFER:
		case INSTR_CP1_BR:
			return RS_CP1_NORMAL;
		default:
			IsTrue((0), ("Datadep (_core_reg_access_type): Undefined unit.\n"));
			return RS_ERROR;
	}
}

ES_REG_STATUS PerfDataDep::_cp2_reg_access_type(EU_INSTR_GROUP unit, UINT id, INT reg_index) {
	ER_REG_SET set = findRegSet(reg_index);
	INT org_index = 0;;
	switch(set) {
		case REG_SET_CP2_SIMD: 
		case REG_SET_CP2_SCALAR: 
			return RS_CP2_WB;
		case REG_SET_CP2_CTRL: 
			return RS_CP2_EX0;
		case REG_SET_CP2_ACC: 
			org_index = reg_index - REG_CP2_ACC;
			switch(org_index) {
				case SIMD_ACC_INDEX:
				case SCALAR_SUM4_ACC_INDEX:
				case SCALAR_MAD_ACC_INDEX:
				case SCALAR_SBIT_ACC_INDEX:
				case SCALAR_SCAN_ACC_INDEX:
					return RS_CP2_RF;
				case IMV_BIT_ACC_INT_INDEX:
					return RS_CP2_EX1;
				case IMV_BEST_MV_INT_INDEX:
				case IMV_BEST_SAD_INT_INDEX:
				case IMV_BEST_COST_INT_INDEX:
				case IMV_START_INT_INDEX:
					return RS_CP2_RF;
				case DBLK_ACC_INDEX:
					if(unit==INSTR_CP2_SCALAR_MAD) {
						return RS_CP2_RF;
					}
					else if(unit==INSTR_CP2_SCALAR_SUM4) {
						return RS_CP2_EX1;
					}
					else {
						IsTrue((0), ("Datadep (_cp2_reg_access_type): Unexpected DBLK_ACC_INDEX index.\n"));
						return RS_ERROR;						
					}
				case DBLK_HORIZ_CNT_INDEX:
				case DBLK_VERT_CNT_INDEX:
					return RS_CP2_DONT_CARE;
				case SCALAR_SCAN_INT_INDEX:
				case SWMV_PAT_INDEX:
					return RS_CP2_RF;
				default:
					IsTrue((0), ("Datadep (_cp2_reg_access_type): Undefined acc/internal register index.\n"));
					return RS_ERROR;					
			}
		default:
			IsTrue((0), ("Datadep (_cp2_reg_access_type): Undefined register set (%d).\n", set));
			return RS_ERROR;		
	}
	switch(unit)
	{
		case INSTR_CP2_XFER_EXT:
		case INSTR_CP2_LS:
		case INSTR_CP2_XFER_INT:
		case INSTR_CP2_SIMD:	
		case INSTR_CP2_SCALAR_SUM4:
		case INSTR_CP2_SCALAR_MAD:
		case INSTR_CP2_SCALAR_BIT:
		case INSTR_CP2_SCALAR_SCAN:
		case INSTR_CP2_BR:
		case INSTR_CP2_MACRO:
		default:
			IsTrue((0), ("Datadep (_cp2_reg_access_type): Undefined unit.\n"));
			return RS_ERROR;
	}	
}

void PerfDataDep::reset() {
	INT i;
	for(i = 0; i<MAX_REG_INDEX; i++) {
		_reg_use[i].request = 0;
		_reg_use[i].lock = 0;
		_reg_define[i].request = 0;
		_reg_define[i].lock = 0;
		if(_reg_status[i].type!=RS_READY) {
			if(_reg_status[i].cycle>0) {
				_reg_status[i].cycle-=1;
				if(_reg_status[i].cycle==0)
					_reg_status[i].type = RS_READY;
			}
		}		
	}
	_port.reset();
}

void PerfDataDep::xferReset() {
	INT i;
	//reset core gpr only
	for(i = 0; i<MAX_REG_GPR; i++) {
		_reg_use[i].request = 0;
		_reg_define[i].request = 0;
	}	
}

ES_HAZARD_STATUS PerfDataDep::_regHazardCheckCP2(IWinItemsIter iwin_iter) {
	UINT id = iwin_iter->perf_id();
	ES_HAZARD_STATUS status = HS_NONE;
	RegVecIter regIter = iwin_iter->dest_reg.begin();
	RegVecIter endIter = iwin_iter->dest_reg.end();
	_port.clear();
	while (regIter!=endIter) {
		INT index = *regIter;
		ES_HAZARD_STATUS s = HS_NONE;		
		if( (s = _reg_cp2_write_isHazard(index, id))==HS_NONE ) {
			_port.addWriteRequest(index);
		}
		else {
			status = (status<s)?s:status;
		}
		regIter++;
	}
	
	//Src dependence check
	regIter = iwin_iter->src_reg.begin();
	endIter = iwin_iter->src_reg.end();			
	while (regIter!=endIter) {
		INT index = *regIter;
		ES_HAZARD_STATUS s = HS_NONE;		
		if( (s = _reg_cp2_read_isHazard(index, id))==HS_NONE ) {
			_port.addReadRequest(index);			
		}	
		else {
			status = (status<s)?s:status;
		}			
		regIter++;
	}
	
	if(status==HS_NONE) {
		status = (_port.isPortJam(PROCESSOR_CP2)==TRUE)?HS_PORT_JAM:HS_NONE;
	}
	
	return status;;		
}

ES_HAZARD_STATUS PerfDataDep::_regHazardCheckCore(IWinItemsIter iwin_iter) {
	UINT id = iwin_iter->perf_id();
	ES_HAZARD_STATUS status = HS_NONE;
	RegVecIter regIter = iwin_iter->dest_reg.begin();
	RegVecIter endIter = iwin_iter->dest_reg.end();
	_port.clear();
	while (regIter!=endIter) {
		INT index = *regIter;
		ES_HAZARD_STATUS s = HS_NONE;
		if( (s = _reg_core_write_isHazard(index, id))==HS_NONE ) {
			_port.addWriteRequest(index);
		}		
		else {
			status = (status<s)?s:status;
		}			
		regIter++;
	}
	
	//Src dependence check
	regIter = iwin_iter->src_reg.begin();
	endIter = iwin_iter->src_reg.end();			
	while (regIter!=endIter) {
		INT index = *regIter;
		ES_HAZARD_STATUS s = HS_NONE;
		if((s = _reg_core_read_isHazard(index, id))==HS_NONE ) {
			_port.addReadRequest(index);
		}		
		else {
			status = (status<s)?s:status;
		}			
		regIter++;
	}
	if(status==HS_NONE) {
		status = (_port.isPortJam(PROCESSOR_CORE)==TRUE)?HS_PORT_JAM:HS_NONE;
	}	

	return status;	
}

void PerfDataDep::updateCoreXferReg(IWinItemsIter iwin_iter) {
	UINT id = iwin_iter->perf_id();
	RegVecIter regIter = iwin_iter->dest_reg.begin();
	RegVecIter endIter = iwin_iter->dest_reg.end();
	while (regIter!=endIter) {
		INT index = *regIter;
		_update_write_request(index, id);
		_update_write_lock(index, id);
		regIter++;
	}
	
	//Src dependence check
	regIter = iwin_iter->src_reg.begin();
	endIter = iwin_iter->src_reg.end();			
	while (regIter!=endIter) {
		INT index = *regIter;
		_update_read_request(index, id);
		_update_read_lock(index, id);
		regIter++;
	}
}

ES_HAZARD_STATUS PerfDataDep::_regHazardCheckCP1(IWinItemsIter iwin_iter) {
	UINT id = iwin_iter->perf_id();
	ES_HAZARD_STATUS status = HS_NONE;
	RegVecIter regIter = iwin_iter->dest_reg.begin();
	RegVecIter endIter = iwin_iter->dest_reg.end();
	
	while (regIter!=endIter) {
		INT index = *regIter;
		if( _reg_cp1_isHazard(index, id)==TRUE ) {
			status = HS_REG_WAW;
		}
		regIter++;
	}
	
	//Src dependence check
	regIter = iwin_iter->src_reg.begin();
	endIter = iwin_iter->src_reg.end();			
	while (regIter!=endIter) {
		INT index = *regIter;
		if( _reg_cp1_isHazard(index, id)==TRUE ) {
			status = HS_REG_RAW;
		}
		regIter++;
	}
	
	return status;	
}
		
ES_HAZARD_STATUS PerfDataDep::check(IWinItemsIter iwin_iter, EP_PROCESSOR cp) {
	switch (cp) {
		case PROCESSOR_CORE:
			return _regHazardCheckCore(iwin_iter);
		case PROCESSOR_CP1:
			return _regHazardCheckCP1(iwin_iter);;		
		case PROCESSOR_CP2:
			return _regHazardCheckCP2(iwin_iter);
		default:
			IsTrue((0), ("Undefined processor.\n"));		
	}	
	return HS_UNDEFINED;
}

ES_HAZARD_STATUS PerfDataDep::_reg_core_write_isHazard(INT index, UINT id) {
	ES_HAZARD_STATUS isHazard = HS_NONE;
	if(index==0) //ignore regsiter 0
		return HS_NONE;
	if(_reg_define[index].request>0&&_reg_define[index].request!=id) {
		//WaW hazard
		isHazard = HS_REG_WAW;
	}
	else if(_reg_use[index].request>0&&_reg_use[index].request!=id) {
		//WaR hazard
		if(_reg_use[index].request!=_reg_use[index].lock) {
			isHazard = HS_REG_WAR;			
		}
	}

	if(isHazard!=TRUE) {
		//check pipe data hazard
		if(_reg_status[index].cycle>PIPE_CORE_WAW_READY) {
			isHazard = HS_REG_WAW;
		}		
	}
	
	_update_write_request(index, id);
	return isHazard;
}

ES_HAZARD_STATUS PerfDataDep::_reg_core_read_isHazard(INT index, UINT id) {
	ES_HAZARD_STATUS isHazard = HS_NONE;
	if(index==0) //ignore regsiter 0
		return HS_NONE;
	if(_reg_define[index].request>0&&_reg_define[index].request!=id) {
		//RaW hazard
		isHazard = HS_REG_RAW;
	}

	if(isHazard==HS_NONE) {
		//check pipe data hazard
		if(_reg_status[index].type!=RS_CORE_NORMAL&&_reg_status[index].cycle>PIPE_CORE_WB_READY) {
			isHazard = HS_REG_RAW;
		}
	}
	
	_update_read_request(index, id);
	return isHazard;
}

BOOL PerfDataDep::_reg_cp1_isHazard(INT index, UINT id) {
	BOOL isHazard = FALSE;
	if(isHazard!=TRUE) {
		//check pipe data hazard
		if(_reg_status[index].cycle>PIPE_ALL_READY) {
			isHazard = TRUE;
		}		
	}
	return isHazard;;
}

ES_HAZARD_STATUS PerfDataDep::_reg_cp2_write_isHazard(INT index, UINT id) {
	ES_HAZARD_STATUS isHazard = HS_NONE;
	if(index==0) //ignore regsiter 0
		return HS_NONE;
	if(_reg_status[index].type==RS_CP2_MACRO_LOCK) {
		isHazard = HS_REG_MACRO;
	}
	else {
		if(_reg_define[index].request>0&&_reg_define[index].request!=id) {
			//WaW hazard
			isHazard = HS_REG_WAW;
		}
		else if(_reg_use[index].request>0&&_reg_use[index].request!=id) {
			//WaR hazard
			if(_reg_use[index].request!=_reg_use[index].lock) {
				isHazard = HS_REG_WAR;			
			}
		}
	
	  // CP2 WaW is non-hazard
	  /*
		if(isHazard!=TRUE) {
			//check pipe data hazard
			if(_reg_status[index].cycle>PIPE_CP2_RF_READY) {
				_wawCnt++;
				isHazard = TRUE;
			}		
		}
		*/
	}
	
	_update_write_request(index, id);
	return isHazard;
}

ES_HAZARD_STATUS PerfDataDep::_reg_cp2_read_isHazard(INT index, UINT id) {
	ES_HAZARD_STATUS isHazard = HS_NONE;
	if(index==0) //ignore regsiter 0
		return HS_NONE;
	if(_reg_status[index].type==RS_CP2_MACRO_LOCK) {
		isHazard = HS_REG_MACRO;
	}	
	else {	
		if(_reg_define[index].request>0&&_reg_define[index].request!=id) {
			//RaW hazard
			isHazard = HS_REG_RAW;
		}
	
		if(isHazard==HS_NONE) {
			//check pipe data hazard
			switch(_reg_status[index].type) {
				case RS_CP2_MACRO_LOCK:
					isHazard = HS_REG_MACRO;
					break;
				case RS_CP2_RF:
					if(_reg_status[index].cycle>PIPE_CP2_RF_READY){
						isHazard = HS_REG_RAW;
					}
					break;
				case RS_CP2_EX0:
					if(_reg_status[index].cycle>PIPE_CP2_EX0_READY){
						isHazard = HS_REG_RAW;
					}
					break;				
				case RS_CP2_EX1:
					if(_reg_status[index].cycle>PIPE_CP2_EX1_READY){
						isHazard = HS_REG_RAW;
					}
					break;				
				case RS_CP2_WB:
					if(_reg_status[index].cycle>PIPE_CP2_WB_READY){
						isHazard = HS_REG_RAW;
					}
					break;				
				case RS_CP2_DONT_CARE:
				case RS_READY:
					break;
				default:	
					IsTrue((0), ("Datadep (_reg_cp2_read_isHazard): Undefined CP2 register access type (%d).\n", _reg_status[index].type));
					break;
			}
		}
	}
	
	_update_read_request(index, id);
	return isHazard;
}

void PerfDataDep::_lock_core_reg(IWinItemsIter iwin_iter) {
	UINT id = iwin_iter->perf_id();
	EU_INSTR_GROUP unit = iwin_iter->unit();
	RegVecIter regIter = iwin_iter->dest_reg.begin();
	RegVecIter endIter = iwin_iter->dest_reg.end();
	while (regIter!=endIter) {
		INT index = *regIter;
		ES_REG_STATUS type = _core_reg_access_type(unit, id);
		_reg_status[index].type = type;
		_reg_status[index].cycle = PIPE_CORE_CYCLE;
		_update_write_lock(index, id);
		regIter++;
	}
	
	//Src dependence check
	regIter = iwin_iter->src_reg.begin();
	endIter = iwin_iter->src_reg.end();			
	while (regIter!=endIter) {
		INT index = *regIter;
		_update_read_lock(index, id);
		regIter++;
	}

}

void PerfDataDep::_lock_cp1_reg(IWinItemsIter iwin_iter) {
	UINT id = iwin_iter->perf_id();
	EU_INSTR_GROUP unit = iwin_iter->unit();
	RegVecIter regIter = iwin_iter->dest_reg.begin();
	RegVecIter endIter = iwin_iter->dest_reg.end();
	while (regIter!=endIter) {
		INT index = *regIter;
		ES_REG_STATUS type = _core_reg_access_type(unit, id);
		_reg_status[index].type = type;		
		_reg_status[index].cycle = PIPE_CP1_CYCLE;
		_update_write_lock(index, id);
		regIter++;
	}
	//Src dependence check
	regIter = iwin_iter->src_reg.begin();
	endIter = iwin_iter->src_reg.end();			
	while (regIter!=endIter) {
		INT index = *regIter;
		_update_read_lock(index, id);
		regIter++;
	}	
}

void PerfDataDep::_lock_cp2_reg(IWinItemsIter iwin_iter) {
	UINT id = iwin_iter->perf_id();
	BOOL isMacro = (iwin_iter->type()==ISSUE_TYPE_MACRO);
	EU_INSTR_GROUP unit = iwin_iter->unit();
	RegVecIter regIter = iwin_iter->dest_reg.begin();
	RegVecIter endIter = iwin_iter->dest_reg.end();
	while (regIter!=endIter) {
		INT index = *regIter;
		if(isMacro==FALSE) {
			ES_REG_STATUS type = _cp2_reg_access_type(unit, id, index);
			_reg_status[index].type = type;		
			_reg_status[index].cycle = PIPE_CP2_CYCLE;
		}
		else {
			_reg_status[index].type = RS_CP2_MACRO_LOCK;
			_reg_status[index].cycle = PIPE_LOCKED;
		}
		
		_update_write_lock(index, id);
		regIter++;
	}
	/*
	#ifdef _INC_MACRO_REG
		if(isMacro==TRUE) {
			IWinItemsIter ucode_iter = iwin_iter->macroInstr.begin();
			IWinItemsIter ucode_iter_end = iwin_iter->macroInstr.end();	
			while(ucode_iter!=ucode_iter_end) {
				RegVecIter regIter = ucode_iter->dest_reg.begin();
				RegVecIter endIter = ucode_iter->dest_reg.end();
				while (regIter!=endIter) {
					INT index = *regIter;	
					_reg_status[index].cycle -= 1;		
					regIter++;
				}
				ucode_iter++;
			}
		}
	#endif
	*/
	//Src dependence check
	regIter = iwin_iter->src_reg.begin();
	endIter = iwin_iter->src_reg.end();			
	while (regIter!=endIter) {
		INT index = *regIter;
		_update_read_lock(index, id);
		regIter++;
	}		
}

void PerfDataDep::unlock(IWinItemsIter iwin_iter) {
	//#ifndef _INC_MACRO_REG
	//	int size = iwin_iter->macroInstr.size();
		IsTrue((iwin_iter->type()==ISSUE_TYPE_MACRO), ("Not a Macro instruction"));
		 IsTrue((iwin_iter->macroInstr.size()<=1), ("Macro uCodeList size != 0"));
		RegVecIter regIter = iwin_iter->dest_reg.begin();
		RegVecIter endIter = iwin_iter->dest_reg.end();
		while (regIter!=endIter) {
			INT index = *regIter;
			IsTrue((_reg_status[index].cycle==0), ("Invalid register reserve cycle %d.\n", _reg_status[index].cycle));
			_reg_status[index].type = RS_READY;
			regIter++;
		}
	//#endif
}

void PerfDataDep::releaseMacroLock(IWinItemsIter iwin_iter) {
	#ifdef _INC_MACRO_REG
		_DEBUG_IWIN_ITER
		IsTrue((iwin_iter->type()==ISSUE_TYPE_UCODE), ("Not an uCode instruction"));
		RegVecIter regIter = iwin_iter->dest_reg.begin();
		RegVecIter endIter = iwin_iter->dest_reg.end();
		while (regIter!=endIter) {
			INT index = *regIter;
			IsTrue((_reg_status[index].type==RS_CP2_MACRO_LOCK), ("Not a Macro lock register"));
			_reg_status[index].cycle += 1;
			if(_reg_status[index].cycle==0) {
				_reg_status[index].type = RS_READY;
			}
			regIter++;
		}	
	#endif
}

void PerfDataDep::lock(IWinItemsIter iwin_iter, EP_PROCESSOR cp) {
	switch (cp) {
		case PROCESSOR_CORE:
			_lock_core_reg(iwin_iter);
			_port.lock(PROCESSOR_CORE);
			break;
		case PROCESSOR_CP1:
			_lock_cp1_reg(iwin_iter);
			break;		
		case PROCESSOR_CP2:
			_lock_cp2_reg(iwin_iter);
			_port.lock(PROCESSOR_CP2);
			break;
		default:
			IsTrue((0), ("Undefined processor.\n"));		
	}	
}

