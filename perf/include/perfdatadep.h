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


#ifndef PERDATADEP_H_
#define PERDATADEP_H_

#include "perfdefs.h"
#include "perfiwinitem.h"
#include "perfport.h"

class PerfDataDep {
	private:
		PerfPort _port;
		RegItem _reg_use[MAX_REG_INDEX];
		RegItem _reg_define[MAX_REG_INDEX];
		RegStatus _reg_status[MAX_REG_INDEX];
		
		INT _cp2SIMDRead;
		INT _cp2SIMDWrite;
		INT _cp2ScalarRead;
		INT _cp2ScalarWrite;		
		
		UINT _wawCnt;
		UINT _rawCnt;
		UINT _warCnt;
		
	private:
		ES_HAZARD_STATUS _regHazardCheckCP2(IWinItemsIter iwin_iter);
		ES_HAZARD_STATUS _regHazardCheckCore(IWinItemsIter iwin_iter);
		ES_HAZARD_STATUS _regHazardCheckCP1(IWinItemsIter iwin_iter);
		ES_HAZARD_STATUS _reg_core_write_isHazard(INT index, UINT id);
		ES_HAZARD_STATUS _reg_core_read_isHazard(INT index, UINT id);
		BOOL _reg_cp1_isHazard(INT index, UINT id);
		ES_HAZARD_STATUS _reg_cp2_write_isHazard(INT index, UINT id);
		ES_HAZARD_STATUS _reg_cp2_read_isHazard(INT index, UINT id);				
		void _lock_core_reg(IWinItemsIter iwin_iter);
		void _lock_cp1_reg(IWinItemsIter iwin_iter);
		void _lock_cp2_reg(IWinItemsIter iwin_iter);
		void _update_read_request(INT index, UINT id) {
			_reg_use[index].request = id;	
		}
		void _update_write_request(INT index, UINT id) {
			_reg_define[index].request = id;	
		}
		void _update_read_lock(INT index, UINT id) {
			_reg_use[index].lock = id;	
		}
		void _update_write_lock(INT index, UINT id) {
			_reg_define[index].lock = id;	
		}		
		ES_REG_STATUS _core_reg_access_type(EU_INSTR_GROUP, UINT); //Core + CP1
		ES_REG_STATUS _cp2_reg_access_type(EU_INSTR_GROUP, UINT, INT); //CP2
		
	public:		
		PerfDataDep();
		void updateCoreXferReg(IWinItemsIter iwin_iter);
		ES_HAZARD_STATUS check(IWinItemsIter iwin_iter, EP_PROCESSOR cp);
		void reset();
		void xferReset();
		void updatePipe();
		void lock(IWinItemsIter iwin_iter, EP_PROCESSOR cp);
		void unlock(IWinItemsIter iwin_iter);
		void releaseMacroLock(IWinItemsIter iwin_iter);
	
};

#endif /*PERDATADEP_H_*/
