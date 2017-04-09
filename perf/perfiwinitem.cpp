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


#include "perfiwinitem.h"
#include "perfcom.h"
#include "decode.h"
#include "javacyc.h"
#include <stdio.h>

PerfIWinItem::PerfIWinItem(ExpInstr *instr, FetchBufIter bufIter, EP_PROCESSOR pro, UINT cyc) {
	Decoder& decoder = DecoderUnit::getInstance();
	ELS_LS_INSTR ls = LS_NON_LS;
	fmt_map* form = instr->fmtMap();
	INT cycle = form->cycle();
	perf_id(bufIter->perfID);
	sub_id(0);
	pc(bufIter->pc);
	raw(bufIter->raw);	
	instr_id(form->id());
	xfer_id(bufIter->xferID);
	spec_id(bufIter->specID);
	metaData(bufIter->meta);
	fetchCyc(cyc);
	cp(pro);
	status(IW_NOT_READY);
	instr_type(form->type());
	hazard(HS_BR_STALL); //default: instruction is speculated before 1st checking
	info(INSTR_INFO_NIL);
	
	if(pro==PROCESSOR_CP2_UCODE) {
		type(ISSUE_TYPE_UCODE);
	}
	else {
		type(ISSUE_TYPE_NORMAL);
	}
	
	switch(instr_type())
	{
		case EI_ls:
			ls = checkLS(instr_id());
			unit(INSTR_RISC_LS);
			break;		
		case EI_alu:
			unit(INSTR_RISC_ALU);
			break;
		case EI_br:
			unit(INSTR_RISC_BR);
			break;				
		case EI_falu:
			unit(INSTR_CP1_ALU);
			break;
		case EI_fls:
			if(pro==((EP_PROCESSOR)PROCESSOR_CORE)) {
				ls = checkLS(instr_id());
				unit(INSTR_RISC_CP1_LS);
			}
			else {
				unit(INSTR_CP1_LS);
			}
			break;				
		case EI_fxfer:
			if(pro==((EP_PROCESSOR)PROCESSOR_CORE)) {
				unit(INSTR_RISC_CP1_XFER);
			}
			else {
				unit(INSTR_CP1_XFER);
			}
			break;	
		case EI_fbr:
			unit(INSTR_CP1_BR);
			break;						
		case EI_cp2_scalar:
			//obtain row index
			decoder.decodeOpdMode(instr, metaData());
			switch(form->subtype()) {
				case EI_cp2_scalar_sum4:
					unit(INSTR_CP2_SCALAR_SUM4);
					break;
				case EI_cp2_scalar_mad:
					unit(INSTR_CP2_SCALAR_MAD);
					break;					
				case EI_cp2_scalar_bit:
					unit(INSTR_CP2_SCALAR_BIT);
					break;	
				case EI_cp2_scalar_scan:
					unit(INSTR_CP2_SCALAR_SCAN);	
					break;
				default:
					IsTrue((0), ("Wrong Scalar instr sub-type %d %d.\n", instr_id(), unit()));
			}
			break;	
		case EI_cp2_br:
			unit(INSTR_CP2_BR);
			break;		
		case EI_cp2_simd:
			unit(INSTR_CP2_SIMD);
			break;
		case EI_cp2_xfer:
			if(form->subtype()==EI_cp2_xfer_ext) {
				if(pro==PROCESSOR_CORE) {
					unit(INSTR_RISC_CP2_XFER);
				}
				else {
					unit(INSTR_CP2_XFER_EXT);
				}
			}
			else
				unit(INSTR_CP2_XFER_INT);
			break;
		case EI_cp2_ls:
			if(pro==PROCESSOR_CORE) {
				ls = checkLS(instr_id());
				unit(INSTR_RISC_CP2_LS);				
			}
			else {
				if(type()==ISSUE_TYPE_NORMAL) {
					ELS_LS_INSTR cp2_ls = checkLS(instr_id());
					if(cp2_ls==LS_CP2_LOAD||cp2_ls==LS_CP2_STORE||cp2_ls==LS_VBUF_LOAD||cp2_ls==LS_VBUF_STORE) {
						if(instr->modf1()==1) {
							type(ISSUE_TYPE_MACRO);
							if(cycle<=0) {
								IsTrue((0), ("Macro instruction build routine return 0.\n"));
							}					
						}
					}
				}
				unit(INSTR_CP2_LS);	
			}
			break;			
		case EI_cp2_macro:
			type(ISSUE_TYPE_MACRO);
			if(cycle<=0) {
				IsTrue((0), ("Macro instruction build routine return 0.\n"));
			}
			break;			
		case EI_cp3_xfer:
			unit(INSTR_CP3_XFER);
			break;
		case EI_cp3_ls:
			ls = checkLS(instr_id());
			unit(INSTR_CP3_LS);	
			break;
		case EI_cp3_br:
			unit(INSTR_CP3_BR);
			if((instr_id()-cp3_chmod_id)==0) {
				if(instr->immediate()==0) {
					cycle = cp3_chmod_n_cyc;
				}
				else {
					cycle = cp3_chmod_j_cyc;
				}
			}
			break;
		default:
			IsTrue((0), ("Unsupported instr %x %d %d.\n", raw(), instr_id(), unit()));
	}		
	#ifdef _INC_MACRO_REG
		if(type()==ISSUE_TYPE_UCODE) {
			switch(pro) {
				case PROCESSOR_CP2_UCODE:
					pro = PROCESSOR_CP2;
					break;
				default:
					break;
			}
		}
	#else
	if(type()!=ISSUE_TYPE_UCODE) {
	#endif
		//build register table
		INT src1 = instr->srcReg1().index;
		INT src2 = instr->srcReg2().index;
		INT src3 = instr->srcReg3().index;
		INT src4 = instr->srcReg4().index;
		INT src5 = instr->srcReg5().index;
		INT dest1 = instr->destReg1().index;
		INT dest2 = instr->destReg2().index;
		INT dest3 = instr->destReg3().index;
		INT dest4 = instr->destReg4().index;
			
		if(dest1>0) {
			if(findRegUnit(dest1)==pro) 
				dest_reg.push_back(dest1);
		}
		if(dest2>0) {
			if(!(dest2==dest1)&&(findRegUnit(dest2)==pro))
				dest_reg.push_back(dest2);
		}
		if(dest3>0) {
			if(!(dest3==dest1||dest3==dest2)&&findRegUnit(dest3)==pro) 
				dest_reg.push_back(dest3); 
		}			
		if(dest4>0) {
			if(!(dest4==dest1||dest4==dest2||dest4==dest3)&&findRegUnit(dest4)==pro) 
				dest_reg.push_back(dest4); 
		}		
		if(src1>0) {
			if(findRegUnit(src1)==pro)
				src_reg.push_back(src1);
		}		
		if(src2>0) {
			if(!(src2==src1)&&findRegUnit(src2)==pro)
				src_reg.push_back(src2);
		}			
		if(src3>0) {
			if(!(src3==src1||src3==src2)&&findRegUnit(src3)==pro)
				src_reg.push_back(src3); 
		}	
		if(src4>0) {
			if(!(src4==src1||src4==src2||src4==src3)&&findRegUnit(src4)==pro) 
				src_reg.push_back(src4); 
		}	
		if(src5>0) {
			if(!(src5==src1||src5==src2||src5==src3||src5==src4)&&findRegUnit(src5)==pro) 
				src_reg.push_back(src5); 
		}	
	#ifndef _INC_MACRO_REG
	}		
	#endif
	
	latency(cycle);	
	#ifdef _DEBUG_PERF
		fprintf(stdout, "%s ", instr->fmtMap()->mn());
		fprintf(stdout, "%d %d, %d %d %d\n", dest1, dest2, src1, src2, src3);
	#endif
}
