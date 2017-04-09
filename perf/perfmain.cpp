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


#include "perfmain.h"
#include "perfdatadep.h"
#include "perfdispatch.h"
#include "perfengine.h"
#include "perfiwin.h"
#include "perfprofiler.h"

PerfMain::PerfMain(PerfFetch& fetch, PerfIWin& iwin, PerfDispatch& dispatch) : 
									_fetch(fetch), 
									_iwin(iwin),
									_dispatch(dispatch)
{
}

UINT PerfMain::execute() {
	UINT cycle = 1;
	BOOL noErr = _fetch.init();
	if(noErr==TRUE) {
		while(_isEnd()==FALSE) {
			//----------fetch instr------------
	
	/*
			if(cycle==1500) {
				printf("Got it\n");
			}
	*/
			_dispatch.dispatch(cycle);
			_fetch.fillWin(cycle);
			_fetch.fillBuf();
			
			PROFILER_DEBUG_FETCH(cycle, _fetch)
			PROFILER_CHECK_FETCH_HIDDEN_CYC(_fetch)
			PROFILER_PRINT_ISSUE(cycle)
			PROFILER_CHECK_LOOP(cycle, _iwin, _fetch)
			PROFILER_RESET
			//_fetch.drinkBuf();
			//_iwin.drinkBuf();
			cycle++;
		}
		_totalCycle = cycle;
		return cycle;
	}
	else {
		fprintf(stderr, "####Setting Error: psim terminated.\n");
		return 0;
	}
}

