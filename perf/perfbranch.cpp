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


#include "perfbranch.h"
#include "javacyc.h"
#include "h264cyc.h"
#include "tool/profiler.h"

#ifdef _DEBUG_BRANCH
	#include "tool/prompt.h"
#endif	

PerfBranch::PerfBranch() {
	_missBr = 0;
	_totalBr = 0;
	_chmodCnt = 0;	
	_callCnt = 0;	
	_otherJr = 0;	
	_excepCnt = 0;
	
	incBrPenalty(TRUE);
	incJrPenalty(TRUE);
	incSpeculated(TRUE);
	incExcepPenalty(TRUE);

	_lastBrID = 1;	

	jrCyc(BR_JUMP_PENALTY);
	brCyc(BR_MISS_PENALTY);
	chmodJCyc(BR_CHMOD_J_PENALTY);
	chmodNCyc(BR_CHMOD_N_PENALTY);
	excepCyc(BR_JAVA_EXCEP_PENALTY);
	basicReolveCyc(BR_BASIC_RESOLVE);
}

void PerfBranch::branchLatency(PerfIWinItem* item) {
	if(incSpeculated()==TRUE) {
		EU_INSTR_GROUP unit = item->unit();
		if(unit==INSTR_RISC_BR||unit==INSTR_CP1_BR||unit==INSTR_CP2_BR||unit==INSTR_CP3_BR) {
			switch(item->instr_id()) {
				case core_jalr_id:
				case core_jal_id:
				case core_jr_id:
					if(incJrPenalty()==TRUE) {
						item->latency(jrCyc());	
					}
					else{
						item->latency(basicReolveCyc());	
					}
					break;
				case core_syscall_id:
				case core_j_id:
					item->latency(basicReolveCyc());	
					break;
				case cp3_chmod_id:
					if(incJrPenalty()==TRUE) {
						item->latency((item->latency()==((INT)cp3_chmod_n_cyc))?chmodNCyc():chmodJCyc());
					}
					else{
						item->latency(basicReolveCyc());	
					}					
					break;
				case cp3_chkidiv_id:
				case cp3_chkirem_id:
				case cp3_excepeq_id:
				case cp3_excepne_id:
				case cp3_exceplt_id:
				case cp3_exceple_id:
					if(incExcepPenalty()==TRUE) {
						item->latency(excepCyc());
					}
					else{
						item->latency(basicReolveCyc());	
					}				
					break;
				default:
					if(incBrPenalty()==TRUE) {
						if(item->metaData()==BR_MISS) {
							_missBr++;
							item->latency(brCyc());
						}
						else {
							item->latency(basicReolveCyc());
						}						
					}
					else{
						item->latency(basicReolveCyc());	
					}					
					break;
			}
		}
	}
}

void PerfBranch::lastBrID(IWinItemsIter iwin_iter) {
	EU_INSTR_GROUP unit = iwin_iter->unit();
	if(unit==INSTR_RISC_BR||unit==INSTR_CP1_BR||unit==INSTR_CP2_BR||unit==INSTR_CP3_BR) {
		UINT id = iwin_iter->instr_id();
		switch(id) {
			case core_syscall_id:
			case core_j_id:
				break;
			case core_jalr_id:
			case core_jal_id:
			case cp3_chmod_id:
			default:
				_lastBrID = iwin_iter->perf_id();
				break;
		}
	}
}

INT PerfBranch::speculatedID(BufItem item, UINT id, EINSTR_TYPE type) {
	IsTrue(((id!=0)&&(type==EI_br||type==EI_fbr||type==EI_cp2_br||type==EI_cp3_br)),
			("Not a branch (%x).", item.perfID)); 
	if(incSpeculated()==TRUE) {
		switch(id) {
			case core_jalr_id:
			case core_jal_id:
			case core_jr_id:
				_callCnt++;
				return item.perfID;
			case core_syscall_id:
			case core_j_id:
				_otherJr++;
				return item.specID;
			case cp3_chmod_id:
				_chmodCnt++;
				return item.perfID;
			case cp3_chkidiv_id:
			case cp3_chkirem_id:
			case cp3_excepeq_id:
			case cp3_excepne_id:
			case cp3_exceplt_id:
			case cp3_exceple_id:	
				_excepCnt++;
				return item.perfID;						
			default:
				_totalBr++;
				return item.perfID;
		}
	}
	return item.specID;
}
