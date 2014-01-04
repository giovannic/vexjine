/*
 * VTF.h: This header file contains the entire API that should be used by the instrumentation framework
 *
 *  Created on: 30 Jun 2010
 *      Author: nb605
 */

#ifndef VTF_H_
#define VTF_H_

#include "Constants.h"

// Profiler
#include "profiler/EventLogger.h"
#include "profiler/MethodData.h"

// Basic event handling methods
#include "events/ThreadEventsBehaviour.h"
#include "events/MethodEventsBehaviour.h"

// Time handling
#include "timer/Time.h"

namespace VEX {

	// Absolutely essential
	bool initializeSimulator(char *options);				// pass the options to VEX to initialize its data structures
	void endSimulator();									// call upon program termination to output simulation results

	extern ThreadEventsBehaviour *threadEventsBehaviour;	// interface to thread-state changing API
	extern MethodEventsBehaviour *methodEventsBehaviour;	// interface to method entering/exiting API
	extern EventLogger *eventLogger;						// interface to the profiled methods registry

	// Optional
	void resetSimulator();									// call to explicitly reset profiling counters (not normally needed)
	void printResults(char *outputDirectory);				// call to explicitly print the simulation results under directory outputDirectory (not normally needed)

	extern bool usingVtfScheduler;				// flat to denote whether the VEX simulator is responsible for thread scheduling or not (i.e. the OS schedules threads normally)
	extern bool shuttingDown;					// flag to denote that the simulation is exiting and all instrumented methods should just exit
	extern char *outputDir;						// The output directory of the simulation - useful to export custom instrumentation-layer statistics
}


#if GDB_USAGE==1

namespace VEX {
	bool firstMethodStillUnloaded();	// Used for debugging from gdb: latch to allow enough time for gdb to connect when GDB_USAGE=1
	void ps();							// Used for debugging from gdb to print VEX state table
}

#define GDB_DEBUGGING_ONE_OFF_DELAY(duration) \
if (VEX::firstMethodStillUnloaded()) { cout << "GDB versions: started waiting" << endl; sleep(duration); cout << "GDB verions: finished waiting" << endl; }


#else
#define GDB_DEBUGGING_ONE_OFF_DELAY(duration);
#endif

#endif /* VTF_H_ */

