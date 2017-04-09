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


#include "perfcom.h"
#include <stdio.h>

ER_REG_SET findRegSet(INT reg_index) {
	if(reg_index>=REG_GPR&&reg_index<(REG_GPR+MAX_REG_GPR))	
		return REG_SET_GPR;
	else if (reg_index>=REG_CP1_GPR&&reg_index<(REG_CP1_GPR+MAX_REG_CP1_GPR))
		return REG_SET_CP1_GPR;
	else if (reg_index==REG_CP1_CTRL)
		return REG_SET_CP1_CTRL;				
	else if (reg_index>=REG_CP2_SIMD&&reg_index<(REG_CP2_SIMD+MAX_REG_CP2_SIMD))
		return REG_SET_CP2_SIMD;
	else if (reg_index>=REG_CP2_SCALAR&&reg_index<(REG_CP2_SCALAR+MAX_REG_CP2_SCALAR))
		return REG_SET_CP2_SCALAR;
	else if (reg_index>=REG_CP2_ACC&&reg_index<(REG_CP2_ACC+MAX_REG_CP2_ACC))
		return REG_SET_CP2_ACC;		
	else if (reg_index>=REG_CP2_CTRL&&reg_index<(REG_CP2_CTRL+MAX_REG_CP2_CTRL))
		return REG_SET_CP2_CTRL;
	else if (reg_index>=REG_CP3_SR&&reg_index<(REG_CP3_SR+MAX_REG_CP3_SR))
		return REG_SET_CP3_SR;
	else if (reg_index>=REG_MEM_VBUF&&reg_index<(REG_MEM_SBUF+MAX_REG_MEM_SBUF))
		return REG_SET_MEM;				
	else 
		IsTrue((0), ("Undefined register index %d.\n", reg_index));
	return REG_UNDEFINED;
}

EP_PROCESSOR findRegUnit(INT reg_index) {
	ER_REG_SET set = findRegSet(reg_index);
	switch(set) {
		case REG_SET_GPR:
		case REG_SET_CP3_SR:
		case REG_SET_MEM:
			return PROCESSOR_CORE;
		case REG_SET_CP1_GPR:
		case REG_SET_CP1_CTRL:
			return PROCESSOR_CP1;
		case REG_SET_CP2_SIMD:
		case REG_SET_CP2_SCALAR:
		case REG_SET_CP2_ACC:
		case REG_SET_CP2_CTRL:
			return PROCESSOR_CP2;
		default:
			IsTrue((0), ("Undefined register index %d.\n", reg_index));
	}
	return PROCESSOR_UNDEFINED;
}

ELS_LS_INSTR checkLS(UINT16 id) {
	switch(id) {
		case core_sb_id:
		case core_sh_id:
		case core_swl_id:
		case core_sw_id:	
		case core_swr_id:
		case cp3_stfield_id:
			return LS_CORE_STORE;
		case core_swc1_id:
			return LS_CP1_STORE;
		case cp2_swc2_id:	
		case cp2_swc2_temp_id:
			return LS_CP2_STORE;
		case cp2_vbs_id:
			return LS_VBUF_STORE;
		case cp2_sbs_id:
			return LS_SBUF_STORE;
			
		case core_lb_id:
		case core_lh_id:
		case core_lw_id:	
		case core_lwl_id:	
		case core_lwr_id:
		case core_lbu_id:
		case core_lhu_id:	
		case cp3_ldfield_id:
			return LS_CORE_LOAD;
		case core_lwc1_id:
			return LS_CP1_LOAD;
		case cp2_lwc2_id:	
		case cp2_lwc2_temp_id:
			return LS_CP2_LOAD;
		case cp2_vbl_id:
			return LS_VBUF_LOAD;
		case cp2_sbls_id:	
		case cp2_sblc_id:
			return LS_SBUF_LOAD;			
		default:
			return LS_NON_LS;
	}			
}

BOOL isDiv(UINT16 id) {
	switch (id) {
		case core_div_id:
		case core_divu_id:
			return TRUE;
		default:
			return FALSE;
	}
}
		