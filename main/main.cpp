#include <stdio.h>
#include "defs.h"
#include "util/messg.h"
#include "regdefs.h"
#include "perfmain.h"
#include "perfdispatch.h"
#include "perffetch.h"
#include "perfiwin.h"
#include "perfprofiler.h"
#include "perforder.h"
#include "perfcache.h"
#include "time.h"

#ifdef _INCLUDE_PROFILER
	#define PROFILER_NOT_ENABLE_WARMING(str)
#else
	#define PROFILER_NOT_ENABLE_WARMING(str) fprintf(stdout, "No effect profiler-built option (%s)\n"); 
#endif

void printUsage(void)
{
	printf("\npsim <<trace_file>> [options.....]\n");
	printf("\nOptions:\n");
	printf("-w (Int)iwin_size: define the instr win size by \"iwin_size\".\n");
	printf("-b (Int)instr_buf_size:	define the instr buffer size.\n");
	printf("-f (Int)instr_fetch_per_cyc: define instr fetch speed.\n");
	printf("--bcspeed (Int)speed_per_cyc: Define bytecode translation rate (Java only).\n");
	printf("--brpredict: Include branch miss predict penalty to fetch unit.\n");
	printf("--nocache: Do not simulate cache.\n");
	printf("--noicache: Exclude icache penalty.\n");
	printf("--icachecyc (Int)cycle: Set i-cache miss penalty cycle.\n");
	printf("--nocp2order: ingnore CP2 instructions pipe order (CP2 only).\n");
	printf("--noxfer: Skip CPs xfer/LS instructions synchronization.\n");
	printf("--nobr: Ignore branch miss-predict penalty.\n");
	printf("--nojr: Ignore jump penalty.\n");
	printf("--nols: Ignore L/S order.\n");	
	printf("--nosynls: Ignore CP-Core L/S synchronization.\n");	
	printf("--nospeculated: Ignore speculated instruction order.\n");	
	printf("--brcyc (Int)cycle: Set branch miss-predict penalty cycle.\n");
	printf("--jrcyc (Int)cycle: Set jump penalty cycle.\n");
	printf("--resolvecyc (Int)cycle: Set branch/jump resolve cycle.\n");
	printf("--withnop: Don't discard NOP (count as an ALU instruction).\n");
	
	#ifdef _INCLUDE_PROFILER
	printf("--printfull: Print full detail of issued instruction.\n");	
	printf("--printlite: Print issued instruction sequence.\n");	
	printf("--lineperprint (Int)line_per_print: Line per issued instruction printing.\n");	
	printf("--iwinprofiling : Enable iwin profiling.\n");	
	printf("--fetchprofiling : Enable fetch profiling.\n");	
	printf("--iwinfullanalysis : Analyse the iwin full situation.\n");	
	printf("--issueanalysis : Analyse the instruction issuing.\n");	
	printf("--icachehiddencyc : Count the hidden cycles of i-cache miss penalty.\n");	
	printf("--psiminstrmode : Print instruction in psim format.\n");	
	#endif
	
	printf("--instrdist: Print instruction distribution in report.\n");	
	printf("--reportfull: Print full detail of report.\n");	
	printf("--reportline (String)file_name: Print line report to file.\n");
	printf("--silent: Silent mode.\n");
	printf("--exetime: Show psim execution time.\n");
	printf("--startpc (Int)pc: Start psim when (pc) has been found.\n");
	printf("--startcnt (Int)cnt: Start psim after #(cnt) instructions.\n");
	printf("--endcnt (Int)cnt: End psim after #(cnt) instructions.\n");
	printf("\n");
}

void printRegIndex(FILE* out) {
	fprintf(out, "\n******************************\n");
	fprintf(out, "Register index base:\n");
	fprintf(out, "Core GPR	= %3d - %3d\n", REG_GPR, REG_GPR+MAX_REG_GPR-1);
	fprintf(out, "CP1 GPR		= %3d - %3d\n", REG_CP1_GPR, REG_CP1_GPR+MAX_REG_CP1_GPR-1);
	fprintf(out, "CP1 Control	= %3d - %3d\n", REG_CP1_CTRL, REG_CP1_CTRL+MAX_REG_CP1_CTRL-1);
	fprintf(out, "CP2 SIMD	= %3d - %3d\n", REG_CP2_SIMD, REG_CP2_SIMD+MAX_REG_CP2_SIMD-1);
	fprintf(out, "CP2 SCALAR	= %3d - %3d\n", REG_CP2_SCALAR, REG_CP2_SCALAR+MAX_REG_CP2_SCALAR-1);
	fprintf(out, "CP2 ACC		= %3d - %3d\n", REG_CP2_ACC, REG_CP2_ACC+MAX_REG_CP2_ACC-1); 
	fprintf(out, "CP2 Control	= %3d - %3d\n", REG_CP2_CTRL, REG_CP2_CTRL+MAX_REG_CP2_CTRL-1);
	fprintf(out, "CP3 Status	= %3d - %3d\n", REG_CP3_SR, REG_CP3_SR+MAX_REG_CP3_SR-1);
	fprintf(out, "JVSR		= %3d - %3d\n", REG_JVSR, REG_JVSR+MAX_REG_JVSR-1);
	fprintf(out, "Mem VBuf	= %3d - %3d\n", REG_MEM_VBUF, REG_MEM_VBUF+MAX_REG_MEM_VBUF-1);
	fprintf(out, "Mem SBuf	= %3d - %3d\n", REG_MEM_SBUF, REG_MEM_SBUF+MAX_REG_MEM_SBUF-1);	
	fprintf(out, "******************************\n");	
}

void printSetting(FILE* out, char* src, INT meta, PerfOrder& order, PerfBranch& branch, PerfEngine& engine, 
					 PerfIWin& iwin, PerfDataDep& dataCheck, PerfDispatch& dispatch,
					 PerfFetch& fetch, PerfCache& cache) {
	fprintf(out, "\n######## Setting ########\n");
	fprintf(out, "Source File: %s\n", src);
	fprintf(out, "Fields per line: %d (metadata %s)\n", meta, meta==2?"excluded":"included");
	fprintf(out, "Profiler build: %s\n",_HAS_PROFILER==1?"on":"off");
	fprintf(out, "Core L/S order included: %s\n", order.incLSorder()==TRUE?"true":"false");
	fprintf(out, "CP2 pipe order included: %s\n", order.incCP2order()==TRUE?"true":"false");
	fprintf(out, "Total mips exec units: %d\n", CORE_MAX_ISSUE);	
	fprintf(out, "Total cp1 exec units: %d\n", CP1_MAX_ISSUE);
	fprintf(out, "Total cp2 exec units: %d\n", CP2_MAX_ISSUE);	
	//fprintf(out, "CP2 SIMD regsiter read/write ports:	%d/%d\n", MAX_SIMD_READ, MAX_SIMD_WRITE);	
	//fprintf(out, "CP2 Scalar regsiter read/write ports:	%d/%d\n", MAX_SCALAR_READ, MAX_SCALAR_WRITE);
	fprintf(out, "Instr buffer size: %d\n", fetch.bufSize());
	fprintf(out, "Core iwin size: %d\n", iwin.coreIwinMaxSize());
	fprintf(out, "CP1 iwin size: %d\n", iwin.cp1IwinMaxSize());
	fprintf(out, "CP2 iwin size: %d\n", iwin.cp2IwinMaxSize());
	fprintf(out, "Instr fetch per cycle: %d\n", fetch.instrFetchSize());
	fprintf(out, "Include cache simulation: %s\n", cache.incCache()==TRUE?"true":"false");
	fprintf(out, "Include icache penalty: %s (%u cycle)\n", cache.incICachePenalty()==TRUE?"true":"false", cache.icacheMissCyc());
	fprintf(out, "Include dcache penalty: %s (%u cycle)\n", cache.incDCachePenalty()==TRUE?"true":"false", cache.dcacheMissCyc());
	fprintf(out, "Speculated instruction order included: %s (%u resolve cycle)\n", branch.incSpeculated()==TRUE?"true":"false", branch.basicReolveCyc());
	fprintf(out, "Branch miss penalty: %s (%d cycle)\n", branch.incBrPenalty()==TRUE?"true":"false", branch.brCyc());
	fprintf(out, "Jump penalty: %s (%u cycle)\n", branch.incJrPenalty()==TRUE?"true":"false", branch.jrCyc());
	fprintf(out, "Java Exception penalty: %s (%u cycle)\n", branch.incExcepPenalty()==TRUE?"true":"false", branch.excepCyc());
		 	
}

int main(INT argc, char **argv)
{  
	FILE* out = stdout;
	FILE* inStream = fopen64(argv[1], "r");
	UINT cyc = 0;
	BOOL show_time = FALSE;
	BOOL full_report = FALSE;
	BOOL line_report = FALSE;
	BOOL silent = FALSE;
	FILE* line_report_fp = NULL;
	PerfOrder order;
	PerfBranch branch;
	PerfEngine engine;
	PerfCache cache;
	PerfDataDep dataCheck;
	PerfIWin iwin(branch, dataCheck);
	PerfDispatch dispatch(iwin, dataCheck, branch, engine, order);
	PerfFetch fetch(iwin, branch, cache);
	PerfMain perfmain(fetch, iwin, dispatch);
	if (argc < 2) {
    	printUsage();
    	exit(0);
  	}

	if (argc > 2)
	{
		int ArgCount = 2;
		while(ArgCount < argc){
			if(strncmp(argv[ArgCount] , "-w", 2) == 0 ){
				if((ArgCount+1)<argc&&strlen(argv[ArgCount+1])>0) {
					INT size = atoi(argv[ArgCount+1]);
					iwin.coreIwinMaxSize(size);
					iwin.cp2IwinMaxSize(size);
					ArgCount += 2;
				}
			}
			else if(strncmp(argv[ArgCount] , "-b", 2) == 0 ){
				if((ArgCount+1)<argc&&strlen(argv[ArgCount+1])>0) {
					INT size = atoi(argv[ArgCount+1]);
					fetch.bufSize(size);
					ArgCount += 2;
				}
			}
			else if(strncmp(argv[ArgCount] , "-f", 2) == 0 ){
				if((ArgCount+1)<argc&&strlen(argv[ArgCount+1])>0) {
					INT size = atoi(argv[ArgCount+1]);
					fetch.instrFetchSize(size);
					ArgCount += 2;
				}				
			}
			else if(strncmp(argv[ArgCount] , "--bcspeed", 9) == 0 ){
				if((ArgCount+1)<argc&&strlen(argv[ArgCount+1])>0) {
					INT size = atoi(argv[ArgCount+1]);
					fetch.bcFetchSize(size);
					ArgCount += 2;
				}
				else {
					fprintf(stdout, "Missing argument (%s)\n", argv[ArgCount]);
					printUsage();
					exit(0);					
				}				
			}
			else if(strncmp(argv[ArgCount] , "--nocache", 9) == 0 ){
				cache.incCache(FALSE);
				ArgCount += 1;
			}		
			else if(strncmp(argv[ArgCount] , "--noicache", 9) == 0 ){
				cache.incICachePenalty(FALSE);
				ArgCount += 1;
			}				
			else if(strncmp(argv[ArgCount] , "--icachecyc", 11) == 0 ){
				if((ArgCount+1)<argc&&strlen(argv[ArgCount+1])>0) {
					INT size = atoi(argv[ArgCount+1]);
					cache.icacheMissCyc(size);
					ArgCount += 2;
				}
				else {
					fprintf(stdout, "Missing argument (%s)\n", argv[ArgCount]);
					printUsage();
					exit(0);					
				}				
			}								
			else if(strncmp(argv[ArgCount] , "--noxfer", 8) == 0 ){
				fetch.ignoreXfer(TRUE);
				ArgCount += 1;
			}	
			else if(strncmp(argv[ArgCount] , "--nobr", 6) == 0 ){
				branch.incBrPenalty(TRUE);
				ArgCount += 1;
			}	
			else if(strncmp(argv[ArgCount] , "--nojr", 6) == 0 ){
				branch.incJrPenalty(TRUE);
				ArgCount += 1;
			}			
			else if(strncmp(argv[ArgCount] , "--nols", 6) == 0 ){
				order.incLSorder(FALSE);
				ArgCount += 1;
			}	
			else if(strncmp(argv[ArgCount] , "--nosynls", 9) == 0 ){
				order.incSynLSorder(FALSE);
				ArgCount += 1;
			}				
			else if(strncmp(argv[ArgCount] , "--nocp2order", 12) == 0 ){
				order.incCP2order(FALSE);
				ArgCount += 1;
			}	
			else if(strncmp(argv[ArgCount] , "--withnop", 9) == 0 ){
				fetch.includeNOP(TRUE);
				ArgCount += 1;
			}			
			else if(strncmp(argv[ArgCount] , "--nospeculated", 14) == 0 ){
				branch.incSpeculated(FALSE);
				ArgCount += 1;
			}	
			else if(strncmp(argv[ArgCount] , "--brcyc", 6) == 0 ){
				if((ArgCount+1)<argc&&strlen(argv[ArgCount+1])>0) {
					INT size = atoi(argv[ArgCount+1]);
					branch.brCyc(size);
					ArgCount += 2;
				}	
				else {
					fprintf(stdout, "Missing argument (%s)\n", argv[ArgCount]);
					printUsage();
					exit(0);					
				}				
			}	
			else if(strncmp(argv[ArgCount] , "--jrcyc", 6) == 0 ){
				if((ArgCount+1)<argc&&strlen(argv[ArgCount+1])>0) {
					INT size = atoi(argv[ArgCount+1]);
					branch.jrCyc(size);
					ArgCount += 2;
				}	
				else {
					fprintf(stdout, "Missing argument (%s)\n", argv[ArgCount]);
					printUsage();
					exit(0);					
				}				
			}				
			else if(strncmp(argv[ArgCount] , "--jrcyc", 6) == 0 ){
				if((ArgCount+1)<argc&&strlen(argv[ArgCount+1])>0) {
					INT size = atoi(argv[ArgCount+1]);
					branch.jrCyc(size);
					ArgCount += 2;
				}	
				else {
					fprintf(stdout, "Missing argument (%s)\n", argv[ArgCount]);
					printUsage();
					exit(0);					
				}				
			}			
			else if(strncmp(argv[ArgCount] , "--resolvecyc", 12) == 0 ){
				if((ArgCount+1)<argc&&strlen(argv[ArgCount+1])>0) {
					INT size = atoi(argv[ArgCount+1]);
					branch.basicReolveCyc(size);
					ArgCount += 2;
				}	
				else {
					fprintf(stdout, "Missing argument (%s)\n", argv[ArgCount]);
					printUsage();
					exit(0);					
				}				
			}		
			else if(strncmp(argv[ArgCount] , "--exetime", 9) == 0 ){
				show_time = TRUE;
				ArgCount += 1;
			}		
			else if(strncmp(argv[ArgCount] , "--printfull", 11) == 0 ){
				PROFILER_NOT_ENABLE_WARMING(argv[ArgCount])
				PROFILER_PRINT_FULL(TRUE)
				ArgCount += 1;
			}	
			else if(strncmp(argv[ArgCount] , "--printlite", 11) == 0 ){
				PROFILER_NOT_ENABLE_WARMING(argv[ArgCount])
				PROFILER_PRINT_LITE(TRUE)
				ArgCount += 1;
			}	
			else if(strncmp(argv[ArgCount] , "--lineperprint", 14) == 0 ){
				PROFILER_NOT_ENABLE_WARMING(argv[ArgCount])
				if((ArgCount+1)<argc&&strlen(argv[ArgCount+1])>0) {
					INT line = atoi(argv[ArgCount+1]);
					PROFILER_LINE_PER_PRINT(line);
					ArgCount += 2;
				}		
				else {
					fprintf(stdout, "Missing argument (%s)\n", argv[ArgCount]);
					printUsage();
					exit(0);					
				}						
			}				
			else if(strncmp(argv[ArgCount] , "--reportfull", 12) == 0 ){
				full_report = TRUE;
				ArgCount += 1;
			}	
			else if(strncmp(argv[ArgCount] , "--reportline", 12) == 0 ){
				if((ArgCount+1)<argc&&strlen(argv[ArgCount+1])>0) {
					line_report_fp = fopen64(argv[ArgCount+1], "a");
					if(line_report_fp!=NULL) {
						line_report = TRUE;
						ArgCount += 2;
					}
					else {
						fprintf(stderr, "Invalid line report file (%s)\n", argv[ArgCount+1]);
					}
				}	
				else {
					fprintf(stdout, "Missing argument (%s)\n", argv[ArgCount]);
					printUsage();
					exit(0);					
				}				
			}	
			else if(strncmp(argv[ArgCount] , "--instrdist", 11) == 0 ){
				PROFILER_PRINT_INSTR_DIST(TRUE)
				ArgCount += 1;
			}
			else if(strncmp(argv[ArgCount] , "--silent", 8) == 0 ){
				out = fopen("/dev/null", "w");
				silent = TRUE;
				if(out==NULL) {
					fprintf(stderr, "Cannot redirect output to /dev/null\n");
					exit(0);
				}
				ArgCount += 1;
			}	
			else if(strncmp(argv[ArgCount] , "--startpc", 9) == 0 ){
				if((ArgCount+1)<argc&&strlen(argv[ArgCount+1])>0) {
					ADDR pc = atoi(argv[ArgCount+1]);
					fetch.startPC(pc);
					ArgCount += 2;
				}	
				else {
					fprintf(stdout, "Missing argument (%s)\n", argv[ArgCount]);
					printUsage();
					exit(0);					
				}							
			}
			else if(strncmp(argv[ArgCount] , "--startcnt", 10) == 0 ){
				if(strlen(argv[ArgCount+1])>0) {
					UINT cnt = atoi(argv[ArgCount+1]);
					fetch.startCnt(cnt);
					ArgCount += 2;
				}	
				else {
					fprintf(stdout, "Missing argument (%s)\n", argv[ArgCount]);
					printUsage();
					exit(0);					
				}							
			}
			else if(strncmp(argv[ArgCount] , "--endcnt", 8) == 0 ){
				if(strlen(argv[ArgCount+1])>0) {
					UINT cnt = atoi(argv[ArgCount+1]);
					fetch.endCnt(cnt);
					ArgCount += 2;
				}	
				else {
					fprintf(stdout, "Missing argument (%s)\n", argv[ArgCount]);
					printUsage();
					exit(0);					
				}							
			}	
			else if(strncmp(argv[ArgCount] , "--iwinprofiling", 15) == 0 ){
				PROFILER_NOT_ENABLE_WARMING(argv[ArgCount])
				PROFILER_IWIN_PROFILING(TRUE)
				ArgCount += 1;				
	
			}
			else if(strncmp(argv[ArgCount] , "--fetchprofiling", 16) == 0 ){
				PROFILER_NOT_ENABLE_WARMING(argv[ArgCount])
				PROFILER_FETCH_PROFILING(TRUE)
				ArgCount += 1;				
			}
			else if(strncmp(argv[ArgCount] , "--iwinfullanalysis", 18) == 0 ){
				PROFILER_IWIN_FULL(TRUE)
				ArgCount += 1;				
			}	
			else if(strncmp(argv[ArgCount] , "--issueanalysis", 15) == 0 ){
				PROFILER_ISSUE_ANALYSIS(TRUE)
				ArgCount += 1;				
			}					
			else if(strncmp(argv[ArgCount] , "--icachehiddencyc", 17) == 0 ){
				PROFILER_FETCH_HIDDEN_CYC(TRUE)
				ArgCount += 1;
			}			
			else if(strncmp(argv[ArgCount] , "--psiminstrmode", 15) == 0 ){
				PROFILER_PSIM_INSTR_MODE(TRUE)
				ArgCount += 1;
			}	
			else {
				fprintf(stderr, "Invalid option (%s)\n", argv[ArgCount]);
				printUsage();
				exit(0);
			}												
		}
	}

	#if defined _PERFBUILD
		fprintf(out, "\n=======================================\n");
		fprintf(out, " Javi Performance Model ver 3.0\n");
		fprintf(out, "=======================================\n");
	#else
		fprintf(stderr, "ERROR: non-PERBUILD version\n");		
		exit(1);
	#endif

  	if(inStream!=NULL) {
  		char str[100];
  		if(fscanf(inStream, " %[^\n]", str)==1) {
  			INT i = 0;
  			STRING tok = strtok(str, " ");
  			while(tok!=NULL) {
				tok = strtok(NULL, " ");
				i++;
			} 
			if(i==2) {
				fetch.includeMeta(FALSE);
			} 			
			else if(i==3) {
				fetch.includeMeta(TRUE);
			}
			else {
				printf("Invalid trace file format (fields per line = %d.\n", i);
				exit(0);
			}
			printSetting(out, argv[1], i, order, branch, engine, iwin, dataCheck, dispatch, fetch, cache);
  		}
  		fclose(inStream);
  	}
  	else {
		fprintf(stderr, "File (%s) not found.\n", argv[1]);
		exit(0);  		
  	}
	inStream = fopen64(argv[1], "r");
  	if(inStream!=NULL) {
  		time_t t1;
  		time_t t2;
		PROFILER_INIT(iwin, fetch)
		fetch.streamFP(inStream);
		printRegIndex(out);
		
  		fprintf(out, "\nInstruction issuing...........\n");	
  		PROFILER_PRINT_OUT(out)
  		t1 = time(NULL);
  		cyc = perfmain.execute();
  		t2 = time(NULL) - t1;
  		if(silent==FALSE) {
	  		if(full_report==TRUE&&_HAS_PROFILER==1) {
	  			PROFILER_PRINT_FULL_REPORT(cyc, order, branch, engine, iwin, dataCheck, dispatch, fetch, cache)
	  		}
	  		else {
	  			fprintf(out, "\n######## Overall results ########\n");
				fprintf(out, "Total instructions fetched = %u\n", fetch.totalcnt());
				fprintf(out, "Total instructions issued = %u\n", dispatch.totalIssued());	
				fprintf(out, "Total execution cycle = %u\n", cyc);	
				fprintf(out, "Overall issued rate= %0.4f\n", ((float) dispatch.totalIssued())/cyc);	
	  		}
	  		if(line_report==TRUE&&_HAS_PROFILER==1) {
	  			PROFILER_PRINT_LINE_REPORT(line_report_fp, cyc, order, branch, engine, iwin, dataCheck, dispatch, fetch, cache)
	  			fclose(line_report_fp);
	  		}
	  		PROFILER_PRINT_IWIN_FULL(cyc)
	  		PROFILER_PRINT_ISSUE_ANALYSIS(cyc)	  		
  		}
		else {
			PROFILER_PRINT_OUT(stdout)
			fprintf(stdout, "%u,%u,%u,%0.4f\n", fetch.totalcnt(), dispatch.totalIssued(), cyc, ((float) dispatch.totalIssued())/cyc);
	  		PROFILER_PRINT_FETCH_HIDDEN_CYC_LINE(cache, cyc)
	  		PROFILER_PRINT_IWIN_FULL_LINE(cyc)
	  		PROFILER_PRINT_ISSUE_ANALYSIS_LINE(cyc)	
	  		PROFILER_PRINT_OUT(out)			
		}
  		if(show_time==TRUE) {
  			fprintf(out, "\nTotal psim execution time = %us (%.1f cycle per sec)\n", t2, ((float)cyc)/t2);
  		}
  		fclose(inStream);
  		PROFILER_CLEAR
  	}
  	if(silent==TRUE) {
  		fclose(out);
  		out = stdout;
  	}
  	return cyc;
 	
}

