/*
 * Constants.h
 *
 *  Created on: 28 Jul 2010
 *      Author: nb605
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

/*************************************************************************
 ****
 **** CONSTANT DECLARATIONS
 ****
 ************************************************************************/

// Basic functionalities enabling/disabling
#define REPORTING_IO_METHODS 1
#define REPORTING_INNER_SOCKETS 1
#define TRACK_GARBAGE_COLLECTOR 1

// Custom preprocessor flags for enabling additional callbacks (found in JVMOptionalCallbacks.cpp)
#define REPORT_ALL_METHODS 0
#define CHANGE_NATIVE_METHODS 0
#define TRIGGER_CLASS_HOOKS 0
#define TRACK_JIT 0

#ifndef DEFAULT_DELAYS_FILE
#define DEFAULT_DELAYS_FILE ".vex_delays"
#endif
// Convert the agent just to add virtual times on method entry - exit
#define PAPI_TESTING 0

#ifndef GDB_USAGE
#define GDB_USAGE 0
#else
#define GDB_USAGE 1
#endif

#ifndef COUNT_STATE_TRANSITIONS
#define COUNT_STATE_TRANSITIONS 1
#endif


#ifndef ENABLE_ASSERTIONS
#define ENABLE_ASSERTIONS 0
#else
#define ENABLE_ASSERTIONS 1
#endif


// Debugging
// Normal case
#define DEBUG 31

//Add 2^x to the debug variable for the following information ddddddddddddddd
/* 0:
 * 1: (03): Enter - exit methods (only!)
 * 2: (03): Thread start/end
 * 3: (02): Thread PIPs

 * 4: (14): IO related methods
 * 5: (04): Speedup
 * 6: (04): Times at method entries/exits
 * 7: (07): Inner sockets monitoring
 * 8: ( ) : Monitors
 */

#define SHOW_LOG 1


#if PAPI_TESTING == 1
static long long *tempMeasurements;
static long long *tempDifferences;
#endif

//Thread states codes:
namespace VexThreadStates {
	enum Code {UNKNOWN_STATE = 0, RUNNING = 1, WAITING = 2, SUSPENDED = 3, LEARNING_IO = 4, IN_IO = 5, TIMED_WAITING = 6,
				IN_NATIVE = 7, IN_MODEL = 8, REGISTERING = 9, SUSPENDED_SELF = 10, NATIVE_WAITING = 11,
				IN_IO_STALE = 12, NATIVE_IO_WAITING = 13,  IN_IO_INVALIDATION = 14, RECOVERED_AFTER_FAILED_IO = 15,
				AWAKING_FROM_WAITING = 16,
				WAITING_INTERNAL_SOCKET_READ = 17,
				VEX_ZOMBIE = 18,
				CUSTOM1 = 19, CUSTOM2 = 20, CUSTOM3 = 21,
				IN_MODEL_WAITING = 22, AFTER_MODEL_SIM_WAITING_FOR_REAL_CODE = 23, IN_SYSTEM_CALL = 24};
};
/* ----------------------- -------------------------------------------- */



#endif /* CONSTANTS_H_ */
