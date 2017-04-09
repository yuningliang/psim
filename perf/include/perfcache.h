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


#ifndef PERFCACHE_H_
#define PERFCACHE_H_

#include "perfdefs.h"

class PerfCache {
	private:
		ICacheItem _icache[(INSTR_CACHE_SIZE/INSTR_CACHE_LINE_SIZE)];
		BOOL _incICachePenalty;
		BOOL _incDCachePenalty;
		BOOL _incCache;
		ADDR _lastICacheAccess;
		UINT _icacheMissCyc;
		UINT _dcacheMissCyc;
		UINT _icacheMissed;
		UINT _dcacheMissed;
		UINT _icacheAccess;
		UINT _dcacheAccess;		
	
	private:
		BOOL _hasTag(UINT tag, UINT index);
		
	public:
		PerfCache();
		void init();
		INT checkICache(ADDR);
		INT checkDCache(ADDR);
		BOOL incICachePenalty() { return _incICachePenalty; }
		void incICachePenalty(BOOL t) { _incICachePenalty = t; }
		BOOL incDCachePenalty() { return _incDCachePenalty; }
		void incDCachePenalty(BOOL t) { _incDCachePenalty = t; }
		BOOL incCache() { return _incCache; }
		void incCache(BOOL t) { _incCache = t; }
		UINT icacheMissCyc() { return (incICachePenalty()==TRUE?_icacheMissCyc:0); }
		void icacheMissCyc(UINT c) { _icacheMissCyc = c; }
		UINT dcacheMissCyc() { return (incDCachePenalty()==TRUE?_dcacheMissCyc:0); }		
		void dcacheMissCyc(UINT c) { _dcacheMissCyc = c; }
		UINT icacheAccess() { return _icacheAccess; }
		UINT dcacheAccess() { return _dcacheAccess; }		
		UINT icacheMissed() { return _icacheMissed; }
		UINT dcacheMissed() { return _dcacheMissed; }		
		ADDR getTag(ADDR addr) { return addr&INSTR_CACHE_OFFSET_MASK; }
		UINT getIndex(ADDR addr) { return (UINT)((addr&INSTR_CACHE_MASK)>>INSTR_CACHE_LINE_LOG_SIZE); }
		ADDR getRepIndex(UINT index) { return _icache[index].rep_index; }
		std::pair<ADDR, ADDR> getLastTag(ADDR addr);
		std::pair<UINT, ADDR> getRepTag(UINT index);
};

_INLINE BOOL PerfCache::_hasTag(UINT tag, UINT index) {
	BOOL t = FALSE;
	for(INT j = 0; j < INSTR_CACHE_WAY; j++) {
		if(_icache[index].tag[j]==tag) {
			t = TRUE;
			_icache[index].rep_index = (++j)%INSTR_CACHE_WAY;
			break;
		}
	}
	if(t==FALSE) {
		UINT repl_index = _icache[index].rep_index;
		_icache[index].tag[repl_index] = tag;
		_icache[index].rep_index = (++repl_index)%INSTR_CACHE_WAY;
	}
	
	return t;
}

#endif /*PERFCACHE_H_*/
