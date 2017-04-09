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


#ifndef PERFDEFS_H_
#define PERFDEFS_H_

#include "expinstr.h"
#include "mipsid.h"
#include "h264id.h"
#include "javaid.h"
#include <list>

#ifdef _DEBUG
	#include <assert.h>
	#define IsTrue(a,b) assert(a)
	#define INT int
#endif

//#define _DEBUG_FETCH 1
#define _DEBUG_DEPENDENCE 1
#define _DEBUG_LOOP_CNT 1
#define _INC_MACRO_REG 1
#define _EXPAND_UCODE 1
//#define _DEBUG_IWIN 1
//#define _DEBUG_BUF 1
//#define _DEBUG_FETCH_SPEED 1
#define _DEBUG_IWIN_ITER PerfIWinItem item = *iwin_iter;
//#define _DEBUG_IWIN_ITER
#define MAX_LOOP_CNT		40

//Define constants
#define INSTR_CACHE_LINE_LOG_SIZE	5 //32 byte
#define INSTR_CACHE_LINE_SIZE		(1<<INSTR_CACHE_LINE_LOG_SIZE) //8 words
#define INSTR_CACHE_LOG_SIZE		14 //16k byte, 2-way associative
#define INSTR_CACHE_WAY				2 //2-way associative
#define INSTR_CACHE_SIZE 			(1<<INSTR_CACHE_LOG_SIZE) //in byte
#define INSTR_CACHE_MASK 			(INSTR_CACHE_SIZE-1) 
#define INSTR_CACHE_OFFSET_MASK 	(~INSTR_CACHE_MASK) 
#define DATA_CACHE_LINE_LOG_SIZE	0
#define DATA_CACHE_LINE_SIZE		(1<<DATA_CACHE_LINE_LOG_SIZE) 
#define DATA_CACHE_LOG_SIZE			0 
#define DATA_CACHE_SIZE 			(1<<DATA_CACHE_LOG_SIZE) 
#define DATA_CACHE_MASK 			(DATA_CACHE_SIZE-1) 
#define DATA_CACHE_OFFSET_MASK 		(~DATA_CACHE_MASK) 
#define DEFAULT_ICACHE_MISS_CYC		25
//#define DEFAULT_ICACHE_MISS_CYC		8
#define DEFAULT_DCACHE_MISS_CYC		35
#define CACHE_INIT_FILL				0

#define DEFAULT_CORE_IWIN_SIZE		6
#define DEFAULT_CP1_IWIN_SIZE		1
#define DEFAULT_CP2_IWIN_SIZE		6
#define DEFAULT_BUF_SIZE			8
#define DEFAULT_FETCH_SPEED			8
#define DEFAULT_BC_SPEED			1
#define CORE_MAX_ISSUE				3
#define CP1_MAX_ISSUE				1
#define CP2_MAX_ISSUE				3
#define MAX_BUF_FETCH				8
#define MAX_PREREAD_BUF				4000000
#define MAX_FUNC_NAME_LEN			50
#define MAX_UCODE_SIZE				31

#define DEFAULT_DONT_CARE_PORT		10000000
#define DEFAULT_CORE_READ_PORT		6
#define DEFAULT_CORE_WRITE_PORT		3
#define DEFAULT_CP2SC_READ_PORT		4
#define DEFAULT_CP2SC_WRITE_PORT	3
#define DEFAULT_CP2RF_READ_PORT		4
#define DEFAULT_CP2RF_WRITE_PORT	3


#define BR_BASIC_RESOLVE			1
#define BR_MISS_PENALTY				6
//#define BR_MISS_PENALTY				2
#define BR_JUMP_PENALTY				6
#define BR_CHMOD_N_PENALTY			6
#define BR_CHMOD_J_PENALTY			5
#define BR_JAVA_EXCEP_PENALTY		3

enum EP_PROCESSOR {
	PROCESSOR_CORE = 0,
	PROCESSOR_CP1 = 1,
	PROCESSOR_CP2 = 2,
	PROCESSOR_CP2_UCODE = 3,
	PROCESSOR_UNDEFINED = -1,
};


enum EU_CPU_UNIT {
	UNIT_UNDEFINED = 0,	
	RISC_ALU = 0x1,	
	RISC_ALU_LS = 0x2,
	RISC_LS_BR = 0x4,
	CP1_FALU = 0x8,
	CP2_SIMD = 0x10,	
	CP2_XFER = 0x20,
	CP2_SCALAR_SUM4 = 0x40,
	CP2_SCALAR_MAD = 0x80,
	CP2_SCALAR_BIT = 0x100,
	CP2_SCALAR_SCAN = 0x200,
	CP2_MACRO = 0x400,
	UNIT_PROCESSOR_FULL = 0xf001,
	UNIT_NOT_READY = 0xf002,
};


enum EP_PIPE_ORDER {
	//LS order
	GROUP_RISC_LS1 = 0,
	GROUP_RISC_LS2 = 1,
	//CP2 order
	GROUP_CP2_SIMD = 2,	
	GROUP_CP2_XFER = 3,
	GROUP_CP2_SCALAR_SUM4 = 4,
	GROUP_CP2_SCALAR_MAD = 5,
	GROUP_CP2_SCALAR_BIT = 6,
	GROUP_CP2_SCALAR_SCAN = 7,
	MAX_PIPE_GROUP = 8,
};

enum EP_PIPE_CYCLE {
	PIPE_LOCKED = 0,
	PIPE_CORE_CYCLE = 4,//ID->RF->EX->WB
	PIPE_CP1_CYCLE = 5,//ID->RF->EX1->EX2->WB
	PIPE_CP2_CYCLE = 6, //ID->RF->EX0->EX1->EX2->WB
	PIPE_CORE_WB_READY = 2, //with register by-pass
	PIPE_CORE_WAW_READY = 1, 
	//PIPE_CORE_WAW_READY = 3, //WaW non-hazard
	//without register by-pass, otherwise +1
	PIPE_CP2_RF_READY = 5, 
	PIPE_CP2_EX0_READY = 4,
	PIPE_CP2_EX1_READY = 3,
	PIPE_CP2_WB_READY = 1,
	PIPE_ALL_READY = 1,
};

enum ES_REG_STATUS {
	RS_READY = 0,
	RS_CORE_NORMAL = 1, //RaW ready in all stage 
	RS_CORE_WB = 2, //RaW ready in WB stage only
	RS_CP1_NORMAL = 3, //RaW ready in all stage 
	RS_CP2_NORMAL = 4, //RaW ready in all stage 
	RS_CP2_MACRO_LOCK = 5,
	RS_CP2_RF = 6,
	RS_CP2_EX0 = 7,
	RS_CP2_EX1 = 8,
	RS_CP2_WB = 9,
	RS_CP2_DONT_CARE = 10,
	RS_ERROR = 0xffff,
};

enum EI_ISSUE_TYPE {
	ISSUE_TYPE_NORMAL = 0,
	ISSUE_TYPE_MACRO = 1,
	ISSUE_TYPE_UCODE = 2,
};

enum EU_INSTR_GROUP {
	INSTR_UNDEFINED = 0,
	INSTR_RISC_ALU = 1,
	INSTR_RISC_LS = 2,
	INSTR_RISC_BR = 3,
	INSTR_RISC_CP1_LS = 4,
	INSTR_RISC_CP1_XFER = 5,
	INSTR_RISC_CP2_LS = 6,
	INSTR_RISC_CP2_XFER = 7,
		
	INSTR_CP1_ALU = 8,
	INSTR_CP1_LS = 9,
	INSTR_CP1_XFER = 10,
	INSTR_CP1_BR = 11,
	
	INSTR_CP2_SIMD = 12,
	INSTR_CP2_SCALAR_SUM4 = 13,
	INSTR_CP2_SCALAR_MAD = 14,
	INSTR_CP2_SCALAR_BIT = 15,
	INSTR_CP2_SCALAR_SCAN = 16,
	INSTR_CP2_LS = 17,
	INSTR_CP2_XFER_EXT = 18,
	INSTR_CP2_XFER_INT = 19,
	INSTR_CP2_BR = 20,
	
	INSTR_CP3_BR = 21,
	INSTR_CP3_LS = 22,
	INSTR_CP3_XFER = 23,	

	INSTR_CP2_MACRO = 24,	
	INSTR_MAX = 25,
};

const STRING instr_group_long_string[] = {
	{"Total"},
	{"Core ALU"},
	{"Core L/S"},
	{"Core BR"},
	{"Core-CP1 L/S"},
	{"Core-CP1 XFER"},
	{"Core-CP2 L/S"},
	{"Core-CP2 XFER"},
		
	{"CP1 ALU"},
	{"CP1 L/S"},
	{"CP1 XFER"},
	{"CP1 BR"},
	
	{"CP2 SIMD"},
	{"CP2 SUM4"},
	{"CP2 SMAD"},
	{"CP2 SBIT"},
	{"CP2 SCAN"},
	{"CP2 L/S"},
	{"CP2 EXT XFER"},
	{"CP2 INT XFER"},
	{"CP2 BR  "},
	
	{"CP3 BR  "},
	{"CP3 L/S"},
	{"CP3 XFER"},

	{"CP2 MACRO"},	
	{"MAX"},
};

const STRING instr_group_string[] = {
	{"---"},
	{"AL0"},
	{"LS0"},
	{"BR0"},
	{"LS1"},
	{"XF1"},
	{"LS2"},
	{"XF2"},
	
	{"AL1"},
	{"LS1"},
	{"XF1"},
	{"BR1"},
		
	{"SIM"},
	{"SUM"},
	{"MAD"},
	{"BIT"},
	{"SCN"},
	{"LS2"},
	{"XF2"},
	{"XFI"},
	{"BR2"},

	{"BR3"},		
	{"LS3"},
	{"XF3"},

	{"MAC"},	
	{"###"},
};

enum IW_ITEM_STATUS {
	IW_NOT_READY = 0,
	IW_READY = 1,
	IW_XFER_READY = 2,
	IW_ISSUED = 3,
	IW_COMPLETED = 4,
	IW_UNDEFINED = 0xffff,
};

enum ES_HAZARD_STATUS {
	HS_UNDEFINED = -1,
	HS_NONE = 0,
	HS_ENGINE_NOT_READY = 1,
	HS_ENGINE_FULL = 2,
	HS_INSTR_ORDER = 3,
	HS_XFER_PIPE_NOT_CLEAR = 4,
	HS_XFER_SYN = 5,
	HS_BR_STALL = 6, 
	HS_PORT_JAM = 7,
	HS_REG_MACRO = 8,
	HS_REG_WAW = 9,
	HS_REG_RAW = 10,
	HS_REG_WAR = 11,
	HS_IWIN_FULL_MAX = 12
};

enum ELS_LS_INSTR {
	LS_NON_LS = 0,
	LS_CORE_LOAD = 1,
	LS_CORE_STORE = 2,
	LS_CP1_LOAD = 3,
	LS_CP1_STORE = 4,
	LS_CP2_LOAD = 5,
	LS_CP2_STORE = 6,		
	LS_VBUF_LOAD = 7,
	LS_VBUF_STORE = 8,		
	LS_SBUF_LOAD = 9,
	LS_SBUF_STORE = 10,
	LS_UNDEFINED = 0xffff,
};

enum EI_INSTR_INFO {
	INSTR_INFO_NIL = 0,
	INSTR_INFO_MACRO_START = 1,
	INSTR_INFO_MACRO_END = 2,
};

typedef struct buf_Item {ADDR pc; WORD raw; INT meta; UINT perfID; UINT xferID; UINT specID;} BufItem;
typedef struct reg_Item {UINT request; UINT lock;} RegItem;
typedef struct reg_status {ES_REG_STATUS type; INT cycle;} RegStatus;
typedef struct order_status {BOOL access; UINT cycle;} OrderStatus;
typedef struct icache_Item {UINT tag[INSTR_CACHE_WAY]; UINT rep_index;} ICacheItem;
typedef std::list<BufItem> FetchBuf;
typedef std::list<BufItem>::iterator FetchBufIter;
typedef std::list<INT> RegVec;
typedef std::list<INT>::iterator RegVecIter;

#endif /*PERFDEFS_H_*/
