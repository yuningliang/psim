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


#include "perfcache.h"
#include "perfprofiler.h"


PerfCache::PerfCache() {
		incICachePenalty(TRUE);
		incDCachePenalty(FALSE);
		incCache(TRUE);
		icacheMissCyc(DEFAULT_ICACHE_MISS_CYC);
		dcacheMissCyc(DEFAULT_DCACHE_MISS_CYC);
		_icacheMissed = 0;
		_dcacheMissed = 0;	
		_icacheAccess = 0;
		_dcacheAccess = 0;
		_lastICacheAccess = 0;
		init();
}

std::pair<ADDR, ADDR> PerfCache::getLastTag(ADDR addr) {
	std::pair<ADDR, ADDR> lastTag;
	INT cache_index = ((addr&INSTR_CACHE_MASK)>>INSTR_CACHE_LINE_LOG_SIZE);
	lastTag.first = _icache[cache_index].tag[0];
	lastTag.second = _icache[cache_index].tag[1];
	return lastTag;
}

std::pair<UINT, ADDR> PerfCache::getRepTag(UINT index) {
	std::pair<UINT, ADDR> repTag;
	repTag.first = _icache[index].rep_index;
	repTag.second = _icache[index].tag[repTag.first];
	return repTag;	
}

void PerfCache::init() {
	INT i, j;
	for(i = 0; i<(INSTR_CACHE_SIZE/INSTR_CACHE_LINE_SIZE); i++) {
		for(j = 0; j < INSTR_CACHE_WAY; j++) {
			_icache[i].tag[j] = CACHE_INIT_FILL;
		}
		_icache[i].rep_index = 0;
	}
}

INT PerfCache::checkICache(ADDR addr) {
	UINT cyc = 0;
	if(incCache()==TRUE) {
		if(addr!=0) {
			INT cache_index = ((addr&INSTR_CACHE_MASK)>>INSTR_CACHE_LINE_LOG_SIZE);
			INT cache_tag = (addr&INSTR_CACHE_OFFSET_MASK);
			_icacheAccess++;
			if(_hasTag(cache_tag, cache_index)==FALSE) {
				_icacheMissed++;
				cyc = icacheMissCyc();
			}
			_lastICacheAccess = addr;
		}
		return cyc;
	}
	else {
		return cyc;
	}
}

INT PerfCache::checkDCache(ADDR addr) {
	return 0;
}
