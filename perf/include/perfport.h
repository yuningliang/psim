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


#ifndef PERFPORT_H_
#define PERFPORT_H_

#include "perfdefs.h"
#include "regdefs.h"
#include "perfcom.h"

class PerfPort {
	private:
		UINT _regCurRead[REG_SET_REG_ONLY_MAX];
		UINT _regCurWrite[REG_SET_REG_ONLY_MAX];

		UINT _regRead[REG_SET_REG_ONLY_MAX];
		UINT _regWrite[REG_SET_REG_ONLY_MAX];

		UINT _regMaxRead[REG_SET_REG_ONLY_MAX];
		UINT _regMaxWrite[REG_SET_REG_ONLY_MAX];
									
	public:
		PerfPort();
		void reset();
		void clear();
		void addReadRequest(UINT index) {
			_regCurRead[findRegSet(index)]+=1;
		}
		void addWriteRequest(UINT index) {
			_regCurWrite[findRegSet(index)]+=1;
		}
		BOOL isPortJam(EP_PROCESSOR cp);
		void lock(EP_PROCESSOR cp);
				
};

#endif /*PERFPORT_H_*/
