/*
 * PapiProfiler.cpp
 *
 *  Created on: 9 Jun 2011
 *      Author: nb605
 */

#include "PapiProfiler.h"
#include <papi.h>

#include <cstdlib>
#include <sstream>
#include <cstdio>
#include <string>
#include <iostream>
#include <syscall.h>
#include <cstring>
#include <fstream>

#include <sys/time.h>
int TESTS_QUIET = 0;
static int TEST_FAIL = 0;

using namespace std;

__thread long long int PapiProfiler::startingThreadCpuTime = 0;
__thread long long int PapiProfiler::startingThreadRealTime = 0;
__thread int PapiProfiler::eventSet = 0;
__thread struct rusage PapiProfiler::resourceUsage;

#define ALL_COUNTERS 20

//const char *counterNames[ALL_COUNTERS] = {"Total cycles (millions)", "Instructions completed (millions)" , "L2 access" , "L2 misses", "utime [ms]","stime [ms]","maxrss","ixrss","idrss","isrss","minflt","majflt","nswap","inblock","oublock","msgsnd","msgrcv","nsignals","nvcsw","nivcsw"};
const char *counterNames[ALL_COUNTERS] = {"Total cycles (millions)", "Instructions completed (millions)" ,
		"L2 misses" , "L2 accesses", "User time [ms]","Kernel time [ms]",
		"Max mem resident set pages","Text segment mem size x exec_ticks[kB]","Data segment mem size x exec_ticks[kB]","Stack mem size x exec_ticks[kB]",
		"Page faults without I/O (min)","Page faults with I/O (maj)","Process swap to mem times","File system inputs for read reqs","File system outputs for write reqs",
		"IPC msgs sent","IPC msgs received","Signals received","Voluntary context switches","Involuntary context switches"};

long maxrss;       /* maximum resident set size */
long ixrss;        /* integral shared memory size */
long idrss;        /* integral unshared data size */
long isrss;        /* integral unshared stack size */
long minflt;       /* page reclaims or soft faults */
long majflt;       /* page faults or hard faults */

long nswap;        /* swaps */

long inblock;      /* block input operations */
long oublock;      /* block output operations */

long msgsnd;       /* messages sent */
long msgrcv;       /* messages received */
long nsignals;     /* signals received */

long nvcsw;        /* voluntary context switches */
long nivcsw;       /* involuntary context switches */
void test_fail(const char *file, int line, const char *call, int retval )
{
	if ( TEST_FAIL )		 //Prevent duplicate output
		return;

	char buf[128];
	memset( buf, '\0', sizeof ( buf ) );
	fprintf( stdout, "%-40s FAILED\nLine # %d\n", file, line );

	if ( retval == PAPI_ESYS ) {
		sprintf( buf, "System error in %s", call );
		perror( buf );
	} else if ( retval > 0 ) {
		fprintf( stdout, "Error: %s\n", call );
	} else if ( retval == 0 ) {
#if defined(sgi)
		fprintf( stdout, "SGI requires root permissions for this test\n" );
#else
		fprintf( stdout, "Error: %s\n", call );
#endif
	} else {
		char errstring[PAPI_MAX_STR_LEN];
		PAPI_perror(errstring);
		fprintf( stdout, "Error in %s: %s\n", call, errstring );
	}

	fprintf( stdout, "\n" );
	TEST_FAIL = 1;

	/* NOTE: Because test_fail is called from thread functions,
	   calling PAPI_shutdown here could prevent some threads
	   from being able to free memory they have allocated.
	 */
}


void getInfoOf(int event_code) {
/*
	char name[128];
	PAPI_event_code_to_name(event_code, name); 	
	cout << "1 adding " << name;
	if (PAPI_query_event(event_code) == PAPI_OK) {
		cout << " which exists: ";
	} else {
		cout << " which DOES NOT exist: ";
	}
*/
}


void PapiProfiler::add_single_event(const char *eventName, int &EventSet) {
	int retval;
	int EventCode;

	std::string buffer("PAPI failed to during addition of ");
	buffer.append(eventName);

	retval = PAPI_event_name_to_code((char *)eventName, &EventCode);
	if ( retval != PAPI_OK) test_fail( __FILE__, __LINE__, buffer.c_str(), retval );
	retval = PAPI_add_event(EventSet, EventCode) ;
	if ( retval != PAPI_OK) test_fail( __FILE__, __LINE__, buffer.c_str(), retval );
}

int PapiProfiler::add_selected_events( int *num_events, int **evtcodes ) {
	/* query and set up the right event to monitor */
	int EventSet = PAPI_NULL;
	int retval;

	unsigned int counters = (unsigned int ) PAPI_num_hwctrs(  );
	( *evtcodes ) = ( int * ) calloc( counters, sizeof ( int ) );
	retval = PAPI_create_eventset( &EventSet );
	if ( retval != PAPI_OK ) test_fail( __FILE__, __LINE__, "PAPI_create_eventset", retval );

	if (counters >= 5) {
		add_single_event("PAPI_TOT_CYC", EventSet);	 // Total cycles
		add_single_event("PAPI_TOT_INS", EventSet);	 // Instructions completed
//		add_single_event("PAPI_RES_STL", EventSet);	 // Cycles stalled on any resource

		add_single_event("PAPI_L2_TCM", EventSet);     // Level 2 cache misses    
		add_single_event("PAPI_L2_TCA", EventSet);     // Level 2 total cache accesses
				
		//add_single_event("PAPI_HW_INT", EventSet);     	 // Hardware interrupts      		
		//add_single_event("PAPI_L1_TCM", EventSet);     // Level 1 cache misses    
		//add_single_event("PAPI_L1_TCA", EventSet);     // Level 1 total cache accesses   

		//add_single_event("PAPI_TOT_IIS", EventSet);	 // Instructions issued
		//add_single_event("PAPI_L1_DCM", EventSet);     // Level 1 data cache misses   
		//add_single_event("PAPI_L1_ICM", EventSet);     // Level 1 instruction cache misses   
		//add_single_event("PAPI_L2_DCM", EventSet);     // Level 2 data cache misses   
		//add_single_event("PAPI_L2_ICM", EventSet);     // Level 2 instruction cache misses   

		//add_single_event("PAPI_CA_SHR", EventSet);     // Requests for exclusive access to shared cache line
		//add_single_event("PAPI_CA_CLN", EventSet);     // Requests for exclusive access to clean cache line
		//add_single_event("PAPI_CA_ITV", EventSet);     // Requests for cache line intervention   
		//add_single_event("PAPI_TLB_DM", EventSet);     // Data translation lookaside buffer misses   
		//add_single_event("PAPI_TLB_IM", EventSet);     // Instruction translation lookaside buffer misses   
		//add_single_event("PAPI_L1_LDM", EventSet);     // Level 1 load misses    
		//add_single_event("PAPI_L1_STM", EventSet);     // Level 1 store misses    
		//add_single_event("PAPI_L2_LDM", EventSet);     // Level 2 load misses    
		//add_single_event("PAPI_L2_STM", EventSet);     // Level 2 store misses    
		//add_single_event("PAPI_HW_INT", EventSet);     // Hardware interrupts      
		//add_single_event("PAPI_BR_CN", EventSet);     // Conditional branch instructions     
		//add_single_event("PAPI_BR_TKN", EventSet);     // Conditional branch instructions taken    
		//add_single_event("PAPI_BR_NTK", EventSet);     // Conditional branch instructions not taken   
		//add_single_event("PAPI_BR_MSP", EventSet);     // Conditional branch instructions mispredicted    
		//add_single_event("PAPI_BR_PRC", EventSet);     // Conditional branch instructions correctly predicted   
		//add_single_event("PAPI_TOT_IIS", EventSet);     // Instructions issued      
		//add_single_event("PAPI_TOT_INS", EventSet);     // Instructions completed      
		//add_single_event("PAPI_FP_INS", EventSet);     // Floating point instructions     
		//add_single_event("PAPI_BR_INS", EventSet);     // Branch instructions      
		//add_single_event("PAPI_VEC_INS", EventSet);     // Vector/SIMD instructions (could include integer)   
		//add_single_event("PAPI_RES_STL", EventSet);     // Cycles stalled on any resource   
		//add_single_event("PAPI_TOT_CYC", EventSet);     // Total cycles      
		//add_single_event("PAPI_L1_DCH", EventSet);     // Level 1 data cache hits   
		//add_single_event("PAPI_L1_DCA", EventSet);     // Level 1 data cache accesses   
		//add_single_event("PAPI_L2_DCA", EventSet);     // Level 2 data cache accesses   
		//add_single_event("PAPI_L2_DCR", EventSet);     // Level 2 data cache reads   
		//add_single_event("PAPI_L2_DCW", EventSet);     // Level 2 data cache writes   
		//add_single_event("PAPI_L1_ICH", EventSet);     // Level 1 instruction cache hits   
		//add_single_event("PAPI_L2_ICH", EventSet);     // Level 2 instruction cache hits   
		//add_single_event("PAPI_L1_ICA", EventSet);     // Level 1 instruction cache accesses   
		//add_single_event("PAPI_L2_ICA", EventSet);     // Level 2 instruction cache accesses   
		//add_single_event("PAPI_L2_TCH", EventSet);     // Level 2 total cache hits   
		//add_single_event("PAPI_L2_TCR", EventSet);     // Level 2 total cache reads   
		//add_single_event("PAPI_L2_TCW", EventSet);     // Level 2 total cache writes   
		//add_single_event("PAPI_FML_INS", EventSet);     // Floating point multiply instructions    
		//add_single_event("PAPI_FDV_INS", EventSet);     // Floating point divide instructions    
		//add_single_event("PAPI_FP_OPS", EventSet);     // Floating point operations     
		//add_single_event("PAPI_SP_OPS", EventSet);     // Floating point operations; optimized to count scaled single
		//add_single_event("PAPI_DP_OPS", EventSet);     // Floating point operations; optimized to count scaled double
		//add_single_event("PAPI_VEC_SP", EventSet);     // Single precision vector/SIMD instructions    
		//add_single_event("PAPI_VEC_DP", EventSet);     // Double precision vector/SIMD instructions   
	}
	*num_events = 5;
	return EventSet;
}




/* add native events to use all counters */
int PapiProfiler::enum_add_native_events( int *num_events, int **evtcodes ) {
	/* query and set up the right event to monitor */
	int EventSet = PAPI_NULL;
	int i = 0, k, event_code, retval;
	unsigned int counters, event_found = 0;
	PAPI_event_info_t info;
	const PAPI_component_info_t *s = NULL;

	s = PAPI_get_component_info( 0 );
	if ( s == NULL ) test_fail( __FILE__, __LINE__, "PAPI_get_component_info", PAPI_ESBSTR );

	counters = (unsigned int ) PAPI_num_hwctrs(  );
	( *evtcodes ) = ( int * ) calloc( counters, sizeof ( int ) );

	PAPI_set_domain(PAPI_DOM_ALL);
	retval = PAPI_create_eventset( &EventSet );
	if ( retval != PAPI_OK ) test_fail( __FILE__, __LINE__, "PAPI_create_eventset", retval );


	/* For platform independence, always ASK FOR the first event */
	/* Don't just assume it'll be the first numeric value */
	i = 0 | PAPI_NATIVE_MASK;
	PAPI_enum_event( &i, PAPI_ENUM_FIRST );
	PAPI_enum_event( &i, PAPI_ENUM_EVENTS );
	PAPI_enum_event( &i, PAPI_ENUM_EVENTS );
	PAPI_enum_event( &i, PAPI_ENUM_EVENTS );
	PAPI_enum_event( &i, PAPI_ENUM_EVENTS );
	char buffer[256];
	do {
		retval = PAPI_get_event_info( i, &info );

		if ( s->cntr_umasks ) {

			k = i;
			if ( PAPI_enum_event( &k, PAPI_NTV_ENUM_UMASKS ) == PAPI_OK ) {
				do {
					retval = PAPI_get_event_info( k, &info );
					event_code = ( int ) info.event_code;
					cout << event_found << "/" << counters << " " ;
					getInfoOf(event_code);

					if (event_found < 0) {
						cout << " skipped ";
						retval = PAPI_OK;
					} else {
						retval = PAPI_add_event( EventSet, event_code );
					}
					if ( retval == PAPI_OK ) {
						( *evtcodes )[event_found] = event_code;
						event_found++;
						PAPI_event_code_to_name(event_code, buffer);
						fprintf( stdout, "Success adding event %s!\n", buffer);
					} else {
						if ( !TESTS_QUIET ) {
							char errstring[PAPI_MAX_STR_LEN];
							PAPI_perror(errstring);
							PAPI_event_code_to_name(event_code, buffer);
							fprintf( stdout, "Failure adding event %s: %s\n", buffer, errstring);
							papiInitializedSuccess = false;
						}
					}
					cout << endl;
				}
				while ( PAPI_enum_event( &k, PAPI_NTV_ENUM_UMASKS ) == PAPI_OK && event_found < counters );
			} else {
				event_code = ( int ) info.event_code;

				cout << event_found << "/" << counters << " " ;
				getInfoOf(event_code);
				if (event_found < 0) {
					cout << " skipped ";
					retval = PAPI_OK;
				} else {
					retval = PAPI_add_event( EventSet, event_code );
				}

				if ( retval == PAPI_OK ) {
					( *evtcodes )[event_found] = event_code;
					event_found++;
					PAPI_event_code_to_name(event_code, buffer);
					fprintf( stdout, "Success adding event %s!\n", buffer);
				} else {
					char errstring[PAPI_MAX_STR_LEN];
					PAPI_perror(errstring);
					PAPI_event_code_to_name(event_code, buffer);
					fprintf( stdout, "Failure adding event %s: %s\n", buffer, errstring);
					papiInitializedSuccess = false;
				}
				cout << endl;
			}
//			if ( !TESTS_QUIET && retval == PAPI_OK )
//				printf( "\n" );
		} else {

			event_code = ( int ) info.event_code;


//cout << event_found << "/" << counters << " " ;
			getInfoOf(event_code);

			if (event_found < 0) {
				cout << " skipped ";
				retval = PAPI_OK;
			} else {
				retval = PAPI_add_event( EventSet, event_code );
			}
			if ( retval == PAPI_OK ) {
				( *evtcodes )[event_found] = event_code;
				event_found++;
			} else {
				if ( !TESTS_QUIET ) {
					cout << " NOT available";
					char errstring[PAPI_MAX_STR_LEN];
					PAPI_perror(errstring);
					fprintf( stdout, "%s", errstring );
					papiInitializedSuccess = false;
				}
			}
			cout << endl;
		}
	}
	while ( PAPI_enum_event( &i, PAPI_ENUM_EVENTS ) == PAPI_OK && event_found < counters );

	*num_events = ( int ) event_found;
	return ( EventSet );
}

PapiProfiler::PapiProfiler(bool initializeCounters) {
	int retval;
	if (initializeCounters) {
		if((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT ) {
			printf("PAPI library initialization error! \n");
			papiInitializedSuccess = false;
		}

		if((retval = PAPI_thread_init( ( unsigned long ( * )( void ) ) ( pthread_self ) )) != PAPI_OK) {
			printf("PAPI thread init error! \n");
			papiInitializedSuccess = false;
		}

		papiInitializedSuccess = true;
	} else {

		papiInitializedSuccess = true;
	}

	totalThreads = 0;
	totalCpuTime = 0;
	count = 0;

	totalValues = new long long int[ALL_COUNTERS];
	for (int i =0 ; i<ALL_COUNTERS; i++) {
		totalValues[i] = 0;
	}

	pthread_mutex_init(&mutex, NULL);
}


void PapiProfiler::startPapiCounterMeasurement() {
	if (papiInitializedSuccess) {
		addEvents();
		int retval;
		if((retval = PAPI_start(eventSet)) != PAPI_OK) {
			printf("PAPI cannot start measurement error! \n");
			papiInitializedSuccess = false;
		} 
//		else {	printf("PAPI initialized correctly for this thread! \n");		}
	}
//	else {	printf("PAPI NOT initialized correctly in the first place! \n");		}
}


void PapiProfiler::addEvents() {
	eventSet = PAPI_NULL;
	int *events;
	eventSet = add_selected_events( &count, &events );
//enum_add_native_events( &count, &events );
}



long long int *PapiProfiler::stopPapiCounterMeasurement() {

	//totalCpuTime += cpuTimeStart;

	int retval;

	long long int *myCounters = new long long int[ALL_COUNTERS];
	for (int i =0 ; i<ALL_COUNTERS; i++) {
		myCounters[i] = 0;
	}

	if (papiInitializedSuccess && eventSet > 0) {
		if((retval = PAPI_stop(eventSet, myCounters)) != PAPI_OK) {
			printf("PAPI cannot stop measurement error! \n");
		}
	}

	return myCounters;
}

string  PapiProfiler::getHardwareInfo() {
	stringstream info;

	const PAPI_hw_info_t *hwinfo = NULL;
	PAPI_mh_level_t *L;
	int i, j;

	/* Get hardware info*/
	if ((hwinfo = PAPI_get_hardware_info()) == NULL)
	{
		printf("PAPI_get_hardware_info error! \n");
		exit(1);
	}
	/* when there is an error, PAPI_get_hardware_info returns NULL */
	info << hwinfo->vendor_string << " " << hwinfo->model_string << endl;
	info << "#CPU: " << hwinfo->totalcpus << endl;
	info << "#Mhz: " << hwinfo->clock_mhz << endl;
	info << "#Active Cores: " << hwinfo->cores << endl;
	info << "#Ncpu, nodes, threads: " << hwinfo->ncpu << " "  << hwinfo->nnodes << " " << hwinfo->threads << endl;
	info << "-----------------------------------------------" << endl;
	info << "Memory levels " << hwinfo->mem_hierarchy.levels << endl;

	if ( !TESTS_QUIET ) {
		printf( "Memory Cache and TLB Hierarchy Information.\n" );
		printf
			( "------------------------------------------------------------------------\n" );
		/* Extract and report the tlb and cache information */
		L = ( PAPI_mh_level_t * ) & ( hwinfo->mem_hierarchy.level[0] );
		printf
			( "TLB Information.\n  There may be multiple descriptors for each level of TLB\n" );
		printf( "  if multiple page sizes are supported.\n\n" );
		/* Scan the TLB structures */
		for ( i = 0; i < hwinfo->mem_hierarchy.levels; i++ ) {
			for ( j = 0; j < PAPI_MH_MAX_LEVELS; j++ ) {
				switch ( PAPI_MH_CACHE_TYPE( L[i].tlb[j].type ) ) {
				case PAPI_MH_TYPE_UNIFIED:
					info << "L" << (i+1) << " Unified TLB:" << endl;
					break;
				case PAPI_MH_TYPE_DATA:
					info << "L" << (i+1) << " Data TLB:" << endl;
					break;
				case PAPI_MH_TYPE_INST:
					info << "L" << (i+1) << " Instruction TLB:" << endl;
					break;
				}
				if ( L[i].tlb[j].type ) {
					if ( L[i].tlb[j].page_size )
						printf( "  Page Size:         %6d KB\n",
								L[i].tlb[j].page_size >> 10 );
					info << "  Number of Entries: " << L[i].tlb[j].num_entries << endl;
					switch ( L[i].tlb[j].associativity ) {
					case 0: /* undefined */
						break;
					case 1:
						info << "  Associativity:      Direct Mapped\n\n";
						break;
					case SHRT_MAX:
						info << "  Associativity:       Full\n\n";
						break;
					default:
						info << "  Associativity:     "<<L[i].tlb[j].associativity<<"\n\n";
						break;
					}
				}
			}
		}
		/* Scan the Cache structures */
		printf( "\nCache Information.\n\n" );
		for ( i = 0; i < hwinfo->mem_hierarchy.levels; i++ ) {
			for ( j = 0; j < 2; j++ ) {
				switch ( PAPI_MH_CACHE_TYPE( L[i].cache[j].type ) ) {
				case PAPI_MH_TYPE_UNIFIED:
					info << "L" << (i+1) <<" Unified Cache:\n";
					break;
				case PAPI_MH_TYPE_DATA:
					info << "L" << (i+1) <<" Data Cache:\n";
					break;
				case PAPI_MH_TYPE_INST:
					info << "L" << (i+1) <<" Instruction Cache:\n";
					break;
				case PAPI_MH_TYPE_TRACE:
					info << "L" << (i+1) <<" Trace Buffer:\n";
					break;
				case PAPI_MH_TYPE_VECTOR:
					info << "L" << (i+1) <<" Vector Cache:\n";
					break;
				}
				if ( L[i].cache[j].type ) {
					info << "  Total size:        " << ( ( L[i].cache[j].size ) >> 10) << " KB\n  Line size:         "<<L[i].cache[j].line_size <<" B\n  Number of Lines:   "<<L[i].cache[j].num_lines <<"\n  Associativity:     "<<L[i].cache[j].associativity <<"\n\n";
				}
			}
		}
	}

//	for (int i =0 ; i< hwinfo->mem_hierarchy.levels; i++) {
//		info << "Level " << (1+i) << " cache " << (hwinfo->mem_hierarchy.level[i].cache[0].size/1024) << "KB " << hwinfo->mem_hierarchy.level[i].cache[0].associativity << "-associative" << endl;
//		info << "Level " << (1+i) << " tlb " << (hwinfo->mem_hierarchy.level[i].tlb[0].page_size/1024) << "KB " << hwinfo->mem_hierarchy.level[i].cache[0].associativity << "-associative" << endl;
//	}

	info << "-----------------------------------------------" << endl;

	return info.str();
}



string PapiProfiler::getAvailablePapiEvents() {

	stringstream info;

	char eventName[128];
	for (int i = PAPI_L1_DCM_idx; i < PAPI_VEC_DP_idx; i++) {
		PAPI_event_code_to_name(i, eventName);

		if (PAPI_query_event(i) != PAPI_OK) {
			info << "Event " << eventName << " does not exist" << endl;
		} else {
			info << "Event " << eventName << " exists!" << endl;
		}
	}

	return info.str();
}

void PapiProfiler::outputResults(ostream &outs) {
	std::vector<PapiProfileRecords *>::iterator it = data.begin();
//	outs << "Thread,CPU time,Real time";
	outs << "Thread,CPU time,Real time,CPU/Real,CPI,L2 miss rate";
	for (int i =0 ; i<ALL_COUNTERS;i++) {
		outs << "," << counterNames[i];
	}
	outs << endl;

	int count = 0;
	short totalMeasures = ALL_COUNTERS;
	PapiProfileRecords *total = new PapiProfileRecords("Totals", totalMeasures);
	while (it != data.end()) {
		PapiProfileRecords *record = *it;
		record->calculateInfo();
		outs << *record << endl;
		*total += *record;
		++count;
		++it;
	}

	total->average(count);
	outs << *total << endl;
}

void PapiProfiler::getTotalMeasurements() {
	outputResults(cout);
}

void PapiProfiler::getTotalMeasurements(const char *file_name) {

	filebuf fb;
	fb.open(file_name, ios::out);
	ostream os(&fb);

	outputResults(os);

	fb.close();
}

PapiProfiler::~PapiProfiler() {
	pthread_mutex_destroy(&mutex);
	//PAPI_shutdown();
	//delete totalValues;
}


void PapiProfiler::onThreadStart() {

	startPapiCounterMeasurement();
	getrusage(RUSAGE_SELF, &resourceUsage);

	if (papiInitializedSuccess) {
		startingThreadCpuTime = PAPI_get_virt_nsec();
		startingThreadRealTime = PAPI_get_real_nsec();
	} else {
		startingThreadCpuTime = 0;
		startingThreadRealTime = 0;
	}
}


void PapiProfiler::onThreadEnd(const char *threadName) {


	long long endCpuTime = 0;
	long long endRealTime = 0;

	if (papiInitializedSuccess) {
		endCpuTime = PAPI_get_virt_nsec();
		endRealTime = PAPI_get_real_nsec();
	}

	long long int *threadValues = stopPapiCounterMeasurement();

	struct rusage totalResourceUsage;
	getrusage(RUSAGE_SELF, &totalResourceUsage);
	int startCount = ALL_COUNTERS-16;
	struct timeval timediff;
	timersub(&totalResourceUsage.ru_utime, &resourceUsage.ru_utime, &timediff);
	threadValues[startCount++] = (timediff.tv_sec * 1e9 + timediff.tv_usec * 1e3)/1e6;
	timersub(&totalResourceUsage.ru_stime, &resourceUsage.ru_stime, &timediff);
	threadValues[startCount++] = (timediff.tv_sec * 1e9 + timediff.tv_usec * 1e3)/1e6;
	threadValues[startCount++] = totalResourceUsage.ru_maxrss - resourceUsage.ru_maxrss;
	threadValues[startCount++] = totalResourceUsage.ru_ixrss - resourceUsage.ru_ixrss;
	threadValues[startCount++] = totalResourceUsage.ru_idrss - resourceUsage.ru_idrss;
	threadValues[startCount++] = totalResourceUsage.ru_isrss - resourceUsage.ru_isrss;
	threadValues[startCount++] = totalResourceUsage.ru_minflt - resourceUsage.ru_minflt;
	threadValues[startCount++] = totalResourceUsage.ru_majflt - resourceUsage.ru_majflt;
	threadValues[startCount++] = totalResourceUsage.ru_nswap - resourceUsage.ru_nswap;
	threadValues[startCount++] = totalResourceUsage.ru_inblock - resourceUsage.ru_inblock;
	threadValues[startCount++] = totalResourceUsage.ru_oublock - resourceUsage.ru_oublock;
	threadValues[startCount++] = totalResourceUsage.ru_msgsnd - resourceUsage.ru_msgsnd;
	threadValues[startCount++] = totalResourceUsage.ru_msgrcv - resourceUsage.ru_msgrcv;
	threadValues[startCount++] = totalResourceUsage.ru_nsignals - resourceUsage.ru_nsignals;
	threadValues[startCount++] = totalResourceUsage.ru_nvcsw - resourceUsage.ru_nvcsw;
	threadValues[startCount++] = totalResourceUsage.ru_nivcsw - resourceUsage.ru_nivcsw;

	pthread_mutex_lock(&mutex);
	data.push_back(new PapiProfileRecords(threadName, threadValues, ALL_COUNTERS, (endCpuTime - startingThreadCpuTime), (endRealTime - startingThreadRealTime)));
	pthread_mutex_unlock(&mutex);

	//	PAPI_unregister_thread();
	//	delete[] threadValues;
}

void PapiProfiler::onThreadEnd() {
	onThreadEnd("Thread");
}

