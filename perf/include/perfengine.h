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


#ifndef PERFENGINE_H_
#define PERFENGINE_H_

#include "perfdefs.h"
#include "perfiwin.h"
#include "perfcom.h"

class PerfEngine {
	private:
		unsigned short _units;
		char _coreUsedUnits;	
		char _cp1UsedUnits;
		char _cp2UsedUnits;
		UINT _totalEngineLocked;
		
	private:	
		BOOL _isReady(EU_CPU_UNIT unit) {
			return ((_units&unit)==0);
		}	
		void _lockUnit(EU_CPU_UNIT unit) {
			_units |= unit;
		}	
	
	public:	
		PerfEngine();
		void reset();
		EU_CPU_UNIT checkReadyUnit(EU_INSTR_GROUP unit, UINT id, EP_PROCESSOR cp);
		void lockEngine(EU_CPU_UNIT unit, EP_PROCESSOR cp);
		UINT totalEngineLocked() { return _totalEngineLocked; }
	
};


#endif /*PERFENGINE_H_*/
