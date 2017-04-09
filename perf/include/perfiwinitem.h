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


#ifndef PERFIWINITEM_H_
#define PERFIWINITEM_H_

#include "perfdefs.h"

class PerfIWinItem;

typedef std::list<PerfIWinItem> IWinItems;
typedef std::list<PerfIWinItem>::iterator IWinItemsIter;

class PerfIWinItem {
	private:
		UINT 			_perf_id;
		UINT 			_sub_id;
		ADDR			_pc;
		WORD			_raw;	
		UINT16  		_instr_id;
		EU_INSTR_GROUP	_unit;
		EINSTR_TYPE		_instr_type;
		UINT			_xfer_id;
		UINT			_spec_id;
		IW_ITEM_STATUS	_status;
		EI_ISSUE_TYPE	_type;
		INT				_metaData;
		INT				_latency;
		UINT			_fetchCyc;
		EP_PROCESSOR 	_cp;
		ES_HAZARD_STATUS _hazard;
		EI_INSTR_INFO	 _info;
			
	public:
		RegVec		src_reg;
		RegVec		dest_reg;
		IWinItems	macroInstr;

	
	public:
		PerfIWinItem(ExpInstr *instr, FetchBufIter bufIter, EP_PROCESSOR cp, UINT cyc);
		
		UINT perf_id(void) { return _perf_id; }
		void perf_id(UINT id) { _perf_id = id; }
		UINT sub_id(void) { return _sub_id; }
		void sub_id(UINT id) { _sub_id = id; }		
		WORD raw(void) { return _raw; }
		void raw(WORD r) { _raw = r; }
		WORD pc(void) { return _pc; }
		void pc(ADDR pc) { _pc = pc; }		
		UINT16 instr_id(void) { return _instr_id; }
		void instr_id(UINT16 id) { _instr_id = id; }
		EU_INSTR_GROUP	unit(void) { return _unit; }
		void unit(EU_INSTR_GROUP u) { _unit = u; }
		EINSTR_TYPE	instr_type(void) { return _instr_type; }
		void instr_type(EINSTR_TYPE t) { _instr_type = t; }		
		UINT xfer_id(void) { return _xfer_id; }
		void xfer_id(UINT id) { _xfer_id = id; }	
		UINT spec_id(void) { return _spec_id; }
		void spec_id(UINT id) { _spec_id = id; }	
		IW_ITEM_STATUS status(void) { return _status; }
		void status(IW_ITEM_STATUS s) { _status = s; }	
		EI_ISSUE_TYPE type(void) { return _type; }
		void type(EI_ISSUE_TYPE t) { _type = t; }
		INT	metaData(void) { return _metaData; }
		void metaData(INT m) { _metaData = m; }
		INT	latency(void) { return _latency; }
		void latency(INT l) { _latency = l; }
		void dec_latency(void) { _latency--; }
		void fetchCyc(UINT c) { _fetchCyc = c; }
		UINT fetchCyc(void) { return _fetchCyc; }							
		WORD cp(void) { return _cp; }
		void cp(EP_PROCESSOR p) { _cp = p; }	
		ES_HAZARD_STATUS hazard(void) { return _hazard; }
		void hazard(ES_HAZARD_STATUS h) { _hazard = h; }	
		EI_INSTR_INFO info(void) { return _info; }
		void info(EI_INSTR_INFO i) { _info = i; }			
};

#endif /*PERFIWINITEM_H_*/
