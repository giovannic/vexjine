//============================================================================
// Name        : VTF.cpp
// Author      : Nick Baltas - nb605
// Version     :
// Copyright   : Imperial College London
//============================================================================
/*
 * VTF.cpp
 *
 *  Created on: 30 Jun 2010
 *      Author: nb605
 */

#include "VTF.h"
#include "VexThreadCustomer.h"

#include "NativeWaitingCriteria.h"
#include "ProfilingInvalidationPolicy.h"

#include "ThreadManagerRegistry.h"
#include "PassiveManager.h"
#include "StrictParallelThreadManager.h"
#include "LooseParallelThreadManager.h"
#include "distributed/ThreadManagerServer.h"
#include "distributed/ThreadManagerClient.h"
#include "ThreadRegistry.h"
#include "IoProtocolClient.h"
#include "TimeLogger.h"
#include "MethodPerformanceModel.h"
#include "LockRegistry.h"
#include "AggregateStateCounters.h"

#ifdef USING_PAPIPROFILER
#include "PapiProfiler.h"
#else
class PapiProfiler;
#endif

#include <cstring>
#include <sys/stat.h>
#include <errno.h>

namespace VEX {

ThreadEventsBehaviour *threadEventsBehaviour;
MethodEventsBehaviour *methodEventsBehaviour;
VirtualTimeline *virtualTimeline;
EventLogger *eventLogger;
ThreadManager *manager;
ThreadManagerRegistry *managers;
ThreadQueue *runnableThreads;
ThreadRegistry *registry;

IoSimulator *ioSimulator;
char *ioImportDirFilePrefix;
char *ioExportDir;

PredictionMethodFactory *predictionMethodFactory;
TimeLoggingBehaviour **timeLoggingBehaviour;
IoLoggingBehaviour *globalLoggingBehaviour;

ObjectRegistry *waitingOnObjectRegistry;
LockRegistry *lockRegistry;
AggregateStateCounters *aggregateStateTransitionCounters;

int mainIterationsPerformedByVexOptimizer;
bool continueVexOptimizersIterations;

bool reportIoMethods;
bool profilingYieldDuration;
int ioBufferSize;
bool printIoBuffers;
bool printGlobalIoBuffers;
bool printSchedulerStats;

bool printGlobalTimeBuffers;
bool printPerThreadTimeBuffers;
bool printAllTimeSamples;

bool pollingNativeWaitingThreads;
bool reentrantLocking;

double prunePercentage;
char *serverHost;
char *predictorType;

int serverPort;
bool isServer;	// denotes whether this process will have the server
bool isClient;

Visualizer *visualizer;
ProfilingInvalidationPolicy *invalidationPolicy;
PapiProfiler *hpcProfiler;

bool profilingHPC;
bool firstMethodLoaded;

int processors;

int initialTime;

char experimentTimePrefix[80];
long schedulerTimeslot;
float globalScalingFactor;
int outputFormat;

bool shuttingDown;
bool usingVtfScheduler;
bool strictParallelScheduling;
char *outputDir;
float busywait;

bool stackTraceMode;
bool enableVisualizer;

bool profileInstrumentationCode;
bool enableManagerLogging;

IoParameters *globalIoParameters;		// all I/O methods treated the same

IoParameters *rtsBlockIo;				// file handling I/O methods simulation parameters
IoParameters *rtsCharIo;				// socket handling I/O methods simulation parameters
IoParameters *rtsSystemCallsIo;			// system calls simulation parameters

char *delaysFile;
char *excludedThreadsFile;

ThreadManager *selectManager(unsigned int id);
bool setupSimulationEnvironment();
void resetProfilerType(const bool &_stackTraceMode, Visualizer *vis);
void setCurrentTimeString();
void printResults(char *outputDirectory);
char *createVtfOptionsFromFile(char *filename);
bool parseVtfOptions(char *options);
bool parseOption(char *option);
bool setValue(char *attr, char *value);
void ensureThatDirectoryExists(char *value);
bool shouldShowLegend();
void setDebuggerFlags();
void showLegend();
void setDefaultValues();
void printHelp();
void cleanup();

/*
 * Setting up the environment
 */
bool initializeSimulator(char *options) {
	setDefaultValues();

	shuttingDown = false;
	if (options != NULL) {

		// read options from file - just merge all file options into a single char
		if (strncmp("file", options, 4) == 0) {
			options = createVtfOptionsFromFile((char *)(&options[5]));
		}

		if (!parseVtfOptions(options)) {
			return false;
		}
	}

	if (!setupSimulationEnvironment()) {
		return false;
	}

	return true;
}

void storeDelays() {
	if (delaysFile == NULL) {
		Time::writeDelaysToFile(DEFAULT_DELAYS_FILE);
	} else {
		Time::writeDelaysToFile(delaysFile);
	}
}

/*
 * Re-initializing simulator
 */
void resetSimulator() {

	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state != NULL) {
		state->clearTransitionCounters();
		state->lockShareResourceAccessKey();
	}

	eventLogger->reset();
	aggregateStateTransitionCounters->clear();
	virtualTimeline->reset();
	registry->resetThreads();
	if (visualizer != NULL) {
		visualizer -> clear();
	}

	if (state != NULL) {
		state->unlockShareResourceAccessKey();
	}

}


/*
 * Terminating the thread manager and printing results
 */
void endSimulator() {

	managers->end();

	printResults(outputDir);
	cleanup();
	storeDelays();
	Time::onSystemClose();
}



/*
 * Outputting the results of the simulation
 */
void printResults(char *outputDirectory) {

	ensureThatDirectoryExists(outputDirectory);
	char *filename = new char[strlen(outputDirectory) + 256];

	eventLogger -> setRegistryStats(registry->getTotalThreadsControlled(), managers->getTotalPosixSignalsSent(), aggregateStateTransitionCounters->getTotalInvocationPoints());
	if (outputFormat == EventLogger::HPROF_LIKE_OUTPUT) {
		sprintf(filename, "%s/vtf_java_hprof_txt.csv", outputDirectory);
		eventLogger->writeData(filename, outputFormat, virtualTimeline->getGlobalTime());
	} else {
		sprintf(filename, "%s/vtf_results.csv", outputDirectory);
		eventLogger->writeData(filename, outputFormat, virtualTimeline->getGlobalTime());
	}

	sprintf(filename, "%s/vtf_methods.csv", outputDirectory);
	eventLogger->writeMethods(filename);

	sprintf(filename, "%s/vtf_method_registry.csv", outputDirectory);
	eventLogger->printMethodRegistry(filename, invalidationPolicy);

	if (printGlobalIoBuffers || printIoBuffers) {
		ioSimulator->printIoStats(outputDirectory);
	}

	if (ioExportDir != NULL) {
		ioSimulator->exportMeasurements(ioExportDir);
	}

	if (visualizer != NULL) {
		sprintf(filename, "%s/vtf_events.csv", outputDirectory);
		visualizer->writeToFile(filename);
	}

    if (strictParallelScheduling) {
    	sprintf(filename, "%s/vtf_strict_schedulers_stats.csv", outputDirectory);
    	managers->writeStats(filename);
    } else {
    	sprintf(filename, "%s/vtf_schedulers_stats.csv", outputDirectory);
    	managers->writeStats(filename);
    }

//	if (virtualTimeline->monitorTimeProgressStats()) {
//		sprintf(filename,"%s/vtf_time_stats.csv", outputDir);
//		virtualTimeline->printStats(filename);
//	}

	if (timeLoggingBehaviour != NULL) {
		cout << "printing timelog for " << processors << " cpus" << endl;
		if (processors == 1) {
			sprintf(filename,"%s/vtf_time_stats.csv", outputDirectory);
			timeLoggingBehaviour[0]->print(filename);
		} else {
			for (int i = 0; i<processors; i++) {
				sprintf(filename,"%s/vtf_time_stats%d.csv", outputDirectory, i);
				timeLoggingBehaviour[i]->print(filename);
			}
		}
	}

#ifdef USING_PAPIPROFILER
	if (profilingHPC) {
		sprintf(filename,"%s/vtf_cpi_and_cache_stats.csv", outputDirectory);
		hpcProfiler->getTotalMeasurements(filename);
	}
#endif

//	cout << "Total lost time " << Time::totalInstrumentationTime << endl;

	// Add code above
#if COUNT_STATE_TRANSITIONS == 1
	sprintf(filename,"%s/state_transitions", outputDirectory);
	aggregateStateTransitionCounters->printTransitionMapAndGraph(filename);
#endif
	// Don't add code here
	delete[] filename;
}


/*
 * Use this check to start the timer
 */
bool firstMethodStillUnloaded() {
	if (firstMethodLoaded) {
		firstMethodLoaded = false;
		return true;
	} else {
		return false;
	}
}

//
//bool parseIoBufferSize(char *_ioBufferSize) {
//	unsigned int cyclicByfferSize = atoi(_ioBufferSize);
//	return predictionMethodFactory->setPredictionBufferSize(cyclicByfferSize);
//}
//
//
//
//bool parseIoLevel(char *value) {
//	if (strcmp(value, "global") == 0 || strcmp(value, "invpoint") == 0 || strcmp(value, "ioop") == 0 || strcmp(value, "none") == 0) {
//		strcpy(predictorType, value);
//		return true;
//	} else {
//		return false;
//	}
//}
//
//IoPredictor *createPredictor() {
//	if (strcmp(predictorType, "global") == 0) {
//		ioPredictor = new GlobalIoPrediction(predictionMethodFactory);
//	} else if (strcmp(predictorType, "ioop") == 0) {
//		ioPredictor = new OperationAwareIoPrediction(predictionMethodFactory);
//	} else if (strcmp(predictorType, "none") == 0) {
//		ioPredictor = new NoIoPrediction();
//	} else {
//		ioPredictor = new StateAwareIoPrediction(predictionMethodFactory);
//	}
//	ioPredictor->setGlobalLoggingBehaviour(globalLoggingBehaviour);
//	return ioPredictor;
//}
//bool parseIoPredictionPolicy(char *ioPredictionPolicyOption) {
//	return predictionMethodFactory->setPredictionMethod(ioPredictionPolicyOption);
//}

bool parseRTS(char *attr, char *value);
bool parseRTSFile(char *file) {
	if (file == NULL) {
		return false;
	}
	struct stat stFileInfo;

	// Attempt to get the file attributes if the file exists
	int intStat = stat(file,&stFileInfo);
	if(intStat == 0) {
		ifstream dfile(file);
		if (dfile.is_open()) {
			for (int i = 0; i<3; i++) {

				char parameters[256];
				string line;
				getline(dfile, line);
				strcpy(parameters, line.c_str());

				char *attr = strtok(parameters, "=");
				char *value = strtok(NULL, "\n");

				if (!parseRTS(attr, value)) {
					dfile.close();
					return false;
				}
			}
			dfile.close();
			return true;
		} else {
			return false;
		}

		return true;
	}
	return true;
}



bool parseRTS(char *attr, char *value) {
	if (strcmp(attr, "rts_block_io") == 0) {
		rtsBlockIo = new IoParameters(value, "|");

	} else if (strcmp(attr, "rts_char_io") == 0) {
		rtsCharIo = new IoParameters(value, "|");

	} else if (strcmp(attr, "rts_syscalls") == 0) {
		rtsSystemCallsIo = new IoParameters(value, "|");

	} else if (strcmp(attr, "rts_file") == 0) {
		return parseRTSFile(value);

	} else {
		return false;

	}
	return true;
}

bool parseIoStats(char *value) {
	if (strcmp(value, "all") == 0) {
		printGlobalIoBuffers = true;
		printIoBuffers = true;
	} else if (strcmp(value, "global") == 0) {
		printGlobalIoBuffers = true;
	} else if (strcmp(value, "invpoint") == 0) {
		printIoBuffers = true;
	} else if (strcmp(value, "none") == 0) {
	} else {
		return false;
	}
	if (printIoBuffers) {
		predictionMethodFactory->setLocalLogging(true);
	}
	return true;
}


/*
 * Method that either verifies that the directory "value" exists or creates it
 */
void ensureThatDirectoryExists(char *value) {
	struct stat stFileInfo;

	// Attempt to get the file attributes
	int intStat = stat(value, &stFileInfo);
	if(intStat != 0) {
		cout << "VEX warning: directory " << value << " does not exist." << endl;
		if (mkdir(value, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
			cout << "Failed to created directory " << value << ". Aborting" << endl;
			exit(0);
		} else {
			cout << "Created directory " << value << " automatically." << endl;
		}
	}
}


bool parseIoExportDirectory(char *value) {
	ensureThatDirectoryExists(value);
	ioExportDir = new char[strlen(value)+1];
	strcpy(ioExportDir, value);

	return true;
}


bool parseTimeStats(char *value) {
	if (strcmp(value, "global") == 0) {
		printGlobalTimeBuffers = true;
	} else if (strcmp(value, "thread") == 0) {
		printPerThreadTimeBuffers = true;
	} else if (strcmp(value, "samples") == 0) {
		printAllTimeSamples = true;
	} else {
		return false;
	}

	return true;
}


bool parseIoExtraLogging(char *value) {
	if (strcmp(value, "ext") == 0) {
		IoLoggingBehaviour::enableExtendedStats();
	} else if (strcmp(value, "gsl") == 0) {
		IoLoggingBehaviour::enableGslStats();
	} else {
		return false;
	}
	return true;
}



bool parseClientOptions(char *clientOptions) {

	char *chunk = strtok(clientOptions, ":");
	serverHost = chunk;
	chunk = strtok(NULL, ":");
	serverPort = atoi(chunk);

	return true;
}


bool parseDirectory(char **directory, char *value) {
	struct stat stFileInfo;
	// Attempt to get the file attributes
	int intStat = stat(value, &stFileInfo);
	if(intStat == 0) {
		(*directory) = new char[strlen(value)+1];
		strcpy((*directory), value);
	} else {
		cout << "VEX error: directory " << value << " does not exist" << endl;
		return false;
	}

	return true;
}


bool parseOutputDirectory(char *value) {
	struct stat stFileInfo;

	// Attempt to get the file attributes
	int intStat = stat(value,&stFileInfo);
	if(intStat == 0) {
		outputDir = new char[strlen(value)+1];
		strcpy(outputDir, value);
	} else {
		cout << "VEX error: output directory " << value << " does not exist" << endl;
		return false;
	}

	return true;
}


bool parseOutputOptions(char *outputOption) {
	if (strcmp(outputOption, "hprof") == 0) {
		outputFormat = EventLogger::HPROF_LIKE_OUTPUT ;
	} else if (strcmp(outputOption, "vtf") == 0) {
		outputFormat = EventLogger::TYPICAL_VTF_OUTPUT;
	} else {
		return false;
	}
	return true;
}

bool parsePrunePercent(char *value) {

	if (value != NULL) {
		prunePercentage = atof(value);
		if (prunePercentage < 0.0 || prunePercentage >= 100.0) {
			cerr << "Error: prune_percent value is not valid" << endl;
		} else {
			return true;
		}
	}
	return false;
}

bool setValue(char *attr, char *value) {
	if (strcmp(attr, "delays_file") == 0 && value != NULL) {
		delaysFile = value;
		profileInstrumentationCode = false;
		return Time::parseDelaysFile(value);
	} else if (strcmp(attr, "debug") == 0) {
		enableManagerLogging = true;
		return true; //parseDebugOptions(value);
	} else if (strcmp(attr, "client") == 0 && value != NULL) {
		return parseClientOptions(value);
	} else if (strcmp(attr, "cpus") == 0 && value != NULL) {
		processors = atoi(value);
		return processors>0;
	} else if (strcmp(attr, "poll_nw") == 0) {
		pollingNativeWaitingThreads = true;
		return true;
	} else if (strcmp(attr,"reentrant_locking") == 0) {
		reentrantLocking = true;
		return true;
//	} else if (strcmp(attr, "no_jvmti") == 0) {
//		usingJvmtiForMonitorControl = false;
//		return true;

	} else if (strcmp(attr, "server") == 0 && value != NULL) {
		isServer = true;
		return parseClientOptions(value);
	} else if (strcmp(attr, "output_format") == 0 && value != NULL) {
		return parseOutputOptions(value);
	} else if (strcmp(attr, "output_dir") == 0 && value != NULL) {
		return parseOutputDirectory(value);




	} else if (strcmp(attr, "iostats") == 0 && value != NULL) {
		return parseIoStats(value);
	} else if (strcmp(attr, "iolog") == 0 && value != NULL) {
		return parseIoExtraLogging(value);
	} else if (strcmp(attr, "ioexport") == 0 && value != NULL) {
		return parseIoExportDirectory(value);
	} else if (strncmp(attr, "io", 2) == 0) {
		return globalIoParameters->setValue(attr, value);


//	} else if (strcmp(attr, "iobuffer") == 0 && value != NULL) {
//		return parseIoBufferSize(value);
//	} else if (strcmp(attr, "iolevel") == 0 && value != NULL) {
//		return parseIoLevel(value);
//	} else if (strcmp(attr, "ioexclude") == 0 && value != NULL) {
//		return parseRandomIoRecognition(value);
//	} else if (strcmp(attr, "iodisregard") == 0) {
//		ioRandomExcluder = new IoExcluder(0);
//		return true;
//	} else if (strcmp(attr, "iothreshold") == 0 && value != NULL) {
//		return parseThresholdIoRecognition(value);
//	} else if (strcmp(attr, "ioimport") == 0 && value != NULL) {
//		predictionMethodFactory->setIoImportMeasurementsFilesPrefix(value);
//		return true;
//	} else if (strcmp(attr, "io") == 0 && value != NULL) {
//		ioProtocol = IoParameters::parseIoPolicy(value);
//		return ioProtocol != NULL;
//	} else if (strcmp(attr, "iopred") == 0 && value != NULL) {
//		return parseIoPredictionPolicy(value);

	} else if (strcmp(attr, "gsf") == 0 && value != NULL) {
		globalScalingFactor = atof(value);
		return globalScalingFactor > 0.0;

	} else if (strcmp(attr, "timeslot") == 0 && value != NULL) {
		if (strcmp(value, "adaptive") == 0) {
			ThreadManager::enableTimeslotMinimazationOnNw();
			return true;
		}

		schedulerTimeslot = atol(value);
		if (schedulerTimeslot < 10000) {
//			cout << "Converting timeslot ms to ns" << endl;
			schedulerTimeslot *= 1000000;
		}
		return schedulerTimeslot > 0;

	} else if (strncmp(attr, "rts_", 4) == 0 && value != NULL) {
		return parseRTS(attr, value);

	} else if (strcmp(attr, "busywait") == 0 && value != NULL) {
		busywait = atof(value);
		return true;
//	} else if (strcmp(attr, "model_scheduler_sim") == 0) {
//		modelSchedulerSim = true;
//		return true;
//	} else if (strcmp(attr, "no_gc_time") == 0) {
//		includeGcTime = false;
//		return true;
	} else if (strcmp(attr, "no_io_reporting") == 0) {
		reportIoMethods = false;
		return true;
	} else if (strcmp(attr, "no_scheduling") == 0) {
		usingVtfScheduler = false;
		return true;
	} else if (strcmp(attr, "spex") == 0) {
		strictParallelScheduling = true;
		return true;
	} else if (strcmp(attr, "time_monitoring") == 0) {
		cout << "NOTHING HAPPENING CURRENTLY WITH TIME_MONITORING" << endl;
		return true;
	} else if (strcmp(attr, "time_stats") == 0 && value != NULL) {
		parseTimeStats(value);
		return true;

	} else if (strcmp(attr, "natwait") == 0) {
		NativeWaitingCriteriaFactory::setCriteriaType(value);
		return true;

	} else if (strcmp(attr, "scheduler_stats") == 0) {
		printSchedulerStats = true;
		return true;
	} else if (strcmp(attr, "stack_trace_mode") == 0) {
		stackTraceMode = true;
		return true;
//	} else if (strcmp(attr, "no_recursive_change") == 0) {
//		changingRecursive = false;
//		return true;
//	} else if (strcmp(attr, "print_recursion") == 0) {
//		return true;
	} else if (strcmp(attr, "profile_yield") == 0) {
		profilingYieldDuration = true;
		return true;
	} else if (strcmp(attr, "profiling_HPC") == 0) {
		profilingHPC = true;
		return true;
//	} else if (strcmp(attr, "monitoring_jvm_thr") == 0) {
//		includeJavaSystemThreads = true;
//		return true;
	} else if (strcmp(attr, "exclude_threads") == 0) {
		excludedThreadsFile = value;
		return true;

	} else if (strcmp(attr, "prune_percent") == 0 && value != NULL) {
		parsePrunePercent(value);
		return true;
	} else if (strcmp(attr, "help") == 0) {
		showLegend();
		printHelp();
		exit(1);

	} else if (strcmp(attr, "version") == 0) {
		showLegend();
		return true;
	} else if (strcmp(attr, "visualize") == 0) {
		enableVisualizer = true;
		return true;

	} else {
		// We have to continue on unknown flag - this will enable other languages (with new flags)
		// to use initializeSimulator as is
		return true;
	}

}

bool parseOption(char *option) {
	char *attr = strtok(option, "=");
	char *value = strtok(NULL, ", ");

	if (attr == NULL) {
		return false;
	}

//	printf("options: %s = attr %s and value %s\n", option, attr, value);
	return setValue(attr, value);
}

/*
 * Debugging function showing all thread states
 */
void ps() {
	cout << endl << endl << "----------------------------------------------------------------------------------------------------" << endl;
	cout << "Thread report at " << virtualTimeline->getGlobalTime() << endl;
	cout << "*******************************" << endl;
	cout << "Running threads" << endl;
	managers->printRunningThread();
	cout << endl;

	cout << "*******************************" << endl;
	int threadListSize = (int)runnableThreads->size();
	cout << "All threads registry (" << registry->getSize() <<")" << endl;
	registry->printThreadStates(threadListSize, ioSimulator->areInIoThreadsInRunnablesQueue());

	runnableThreads->print();
	cout << "*******************************" << endl;
	cout << "----------------------------------------------------------------------------------------------------" << endl;
}


void printHelp() {
	cout << "Accepted parameters in form <parameter>[=<value>],<parameter>[=<value>],...,<parameter>[=<value>] " << endl;

	cout << "-------------- VEX scheduler parameters --------------" << endl;
	cout << setw(20) << "cpus=value" << "\tNumber of virtual cores to simulate" << endl;
	cout << setw(20) << "client=host:port" << "\tRun the scheduler as a client to the server listening at host:port" << endl;
	cout << setw(20) << "debug" << "\tEnable logging into the vtf_manager_log file [Only working if the library is compiled without the -DDISABLE_LOGGING=1 gcc flag]" << endl;
	cout << setw(20) << "delays_file=filename" << "\tFile with delays incurred by the instrumentation code. Otherwise, using .vex_delays file. Otherwise, profiling delays at runtime" << endl;

	cout << setw(20) << "help"  << "\tPrint this parameter menu and exit" << endl;
//	cout << setw(20) << "model_scheduler_sim" << "\tModels are simulated by the schedulers instead of threads themselves [default: false]" << endl;

	cout << setw(20) << "no_recursive_change" << "\tDo not re-adapt recursive calls" << endl;
	cout << setw(20) << "no_scheduling" << "\tDo not schedule the threads in virtual time. Just profile their times" << endl;

	cout << setw(20) << "profile_yield" << "\tProfile the duration of sched_yield" << endl;
	cout << setw(20) << "spex" << "\tEnforce SPEX multicore synchronisation scheme [default: off]" << endl;
	cout << setw(20) << "server" << "\tStart the scheduler as a server listening at host:port" << endl;
	//cout << setw(20) << "time_monitoring" << "\tPrint events that progress the virtual timer into <output_dir>/vtf_time_progress.csv" << endl;

	cout << setw(20) << "timeslot=value" << "\tTimeslot duration of the scheduler. If value < 10,000 then timeslot=value ms (1ms to 10sec), otherwise timeslot=value ns" << endl;
	cout << setw(20) << "version"  << "\tPrint version of VEX before starting simulation" << endl;

	cout << endl;

	cout << "-------------- VEX - Java specific parameters --------------" << endl;
	cout << setw(20) << "exclude_threads=filename" << "\tFile with the names of threads to be excluded" << endl;
	cout << setw(20) << "monitoring_jvm_thr" << "\tTry to control Java system threads from VEX. Not all system threads are exported to the agent through JVMTI [default: false]" << endl;
	cout << setw(20) << "natwait=value" << "\tIdentification method for native waiting threads. Values: none|vexStats|stackTrace|combined [default: vexStats]" << endl;
	cout << setw(20) << "no_gc_time" << "\tDo not take into account the duration of garbage collections in method times" << endl;
	cout << setw(20) << "no_jvmti" << "\tExplicit monitor trapping: Do not rely on JVMTI to track monitor changes" << endl;
	cout << setw(20) << "poll_nw" << "\tPoll native waiting threads to determine whether they should be put back into the framework" << endl;
	cout << setw(20) << "reentrant_locking" << "\tAllow re-entrant locking when explicitly tracking monitors - use only in combination with JINE's limitJvmtiUsage" << endl;

	cout << endl;

	cout << "-------------- VEX output parameters --------------" << endl;
	cout << setw(20) << "output_dir=dir" << "\tDirectory where output files will be written. [default: ./]" << endl;
	cout << setw(20) << "output_format=value" << "\tFormat of output. Values: vtf|hprof [default: vtf]" << endl;
	cout << setw(20) << "print_recursion" << "\tRuntime output of recursive method names that will be re-adapted" << endl;
	cout << setw(20) << "profiling_HPC" << "\tProfile hardware performance counters (including VEX overhead) into <output_dir>/vtf_cpi_and_cache_stats.csv" << endl;
	cout << setw(20) << "prune_percent" << "\tMethods with estimated real time (inclusive) less than prune_percent will not be reported [default: disabled]" << endl;

	cout << setw(20) << "scheduler_stats" << "\tPrint statistics for thread states found by the scheduler into <output_dir>/vtf_scheduler_stats.csv" << endl;

	cout << setw(20) << "stack_trace_mode" << "\tStore results in stack trace mode (not only according to method names) - use only in combination with with JINE's stackTraceMode" << endl;
	cout << setw(20) << "time_stats=value" << "\tPrint time increase statistics into <output_dir>/vtf_time_stats.csv. Values: global|thread|samples [default: global]" << endl;
	cout << setw(20) << "visualize" << "\tLog the visualization information into <output_dir>/vtf_events.csv" << endl;
    cout << endl;
	cout << "-------------- I/O handling parameters --------------" << endl;
	cout << setw(20) << "io=value" << "\tI/O method used. Values: serial|strict|normal|lax [default: normal]" << endl;
	cout << setw(20) << "iobuffer=size" << "\tSize of I/O prediction buffer [default: 16]" << endl;
	cout << setw(20) << "iopred=value" << "\tI/O prediction method. Values: replay|avg|min|max|median|sampling|gsl|cmean|linregr|aryulwalk|arburg|markov [default: avg-16]" << endl;
	cout << setw(20) << "iolevel=value" << "\tSelect the level of I/O prediction. Values: none|global|invpoint|ioop (invocation point) [default: invpoint]" << endl;

	cout << setw(20) << "ioexclude=value" << "\tRandom exclusion of value % I/O operations, measuring their duration in CPU time [default: off]" << endl;
	cout << setw(20) << "iodisregard" << "\tOpt-out all I/O operations. Only their CPU time is taken into account. Same as ioexclude=100. [default: off]" << endl;
	cout << setw(20) << "iothreshold=value" << "\tI/O operations less than value ns will not be recognized as I/O, but still measured in real time [default: off]" << endl;

	cout << setw(20) << "iostats=value" << "\tGenerate I/O prediction statistics on the defined level. Values: off|global|invpoint|all [default: off]" << endl;
	cout << setw(20) << "iolog=value" << "\tEnable extra I/O logging options (iostats enables basic ones). Values: ext|gsl [default: off]" << endl;
	cout << setw(20) << "ioimport=file_prefix" << "\tImport I/O measurements from file_prefix into the current predictor [default: off]" << endl;
	cout << setw(20) << "ioexport=dir" << "\tExport I/O measurements from current predictor into directory dir [default: off]" << endl;
	cout << setw(20) << "no_io_reporting" << "\tDo not report I/O method times" << endl;
	cout << setw(20) << "Real Time Simulation" << "\tRTS: Define the real time simulation method for block I/O, character I/O and system calls. All 3 need to be used together" << endl;
	cout << setw(20) << "" << "\tvalue format: prediction-median|buffer-16|stats-all|recogn_random-100" << endl;

	cout << setw(20) << "rts_char_io=value" << "\tDefine the rts method for character I/O calls. Only effective when used together with rts_block_io and rts_char_io" << endl;
	cout << setw(20) << "rts_block_io=value" << "\tDefine the rts method for block I/O calls. Only effective when used together with rts_syscalls and rts_char_io" << endl;
	cout << setw(20) << "rts_syscalls=value" << "\tDefine the rts method for system calls. Only effective when used together with rts_block_io and rts_syscalls" << endl;
	cout << setw(20) << "rts_file=filename" << "\tDefine the rts method from a file. Define rts_block_io, rts_char_io and rts_syscalls in the file (rts_XXX=value)" << endl;

	cout << endl;
}

char *createVtfOptionsFromFile(char *filename) {
	ifstream file(filename);
	stringstream stream;
	string sLine;

	bool firstArg = true;
	while (file)
	{
		getline(file, sLine);
		if (sLine != "") {
			if (firstArg) {
				stream << sLine;
				firstArg = false;
			} else {
				stream << "," << sLine;
			}
		}
	}

	char *fileOptions = new char[stream.str().length()+1];
	strcpy(fileOptions, stream.str().c_str());
	return fileOptions;
}


bool parseVtfOptions(char *options) {
	int count = 0;
	char *maxOptions[100];
	maxOptions[count++] = strtok(options ,",");
	while (maxOptions[count-1] != NULL && count < 100) {
	  maxOptions[count++] = strtok(NULL, ", ");
	}
    --count;

	for (int i =0 ; i <count; ++i) {
		if (!parseOption(maxOptions[i])) {
			printHelp();
			return false;
		}
	}

	return true;
}

void setDefaultValues() {

	eventLogger = NULL;
	globalLoggingBehaviour = NULL;
	timeLoggingBehaviour = NULL;
	threadEventsBehaviour = NULL;
	methodEventsBehaviour = NULL;

	outputFormat = EventLogger::TYPICAL_VTF_OUTPUT;
	firstMethodLoaded = true;
	reportIoMethods = true;
	profilingYieldDuration = false;

	schedulerTimeslot = 100000000;

	#ifndef DISABLE_LOGGING
	#if GDB_USAGE == 1
	enableManagerLogging = true;
	#else
	enableManagerLogging = false;
	#endif
	#else
	enableManagerLogging = false;
	#endif

	visualizer = NULL;

	delaysFile = NULL;

	printIoBuffers = false;
	printGlobalIoBuffers = false;

	printSchedulerStats = false;

	printGlobalTimeBuffers = false;
	printPerThreadTimeBuffers = false;
	printAllTimeSamples = false;

	pollingNativeWaitingThreads = false;
	reentrantLocking = false;

	busywait = 0;
	usingVtfScheduler = true;

	outputDir = new char[8];
	strcpy(outputDir, "./");

	predictorType = new char[32];
	strcpy(predictorType, "");

	ioImportDirFilePrefix = NULL;
	ioExportDir = NULL;

	stackTraceMode = false;
	enableVisualizer = false;

	profileInstrumentationCode = true;
	prunePercentage = 0.0;

	profilingHPC = false;

	predictionMethodFactory = new PredictionMethodFactory();
	processors = 1;
	serverHost = NULL;
	serverPort = 0;
	isServer = false;
	isClient = false;

	strictParallelScheduling = false;
//	modelSchedulerSim = false;			// used to distinguish between threads simulating themselves and the scheduler simulating them - now only the latter applies
//	usingJvmtiForMonitorControl = true;

	globalIoParameters = new IoParameters();
	globalScalingFactor = 1.0;

	rtsBlockIo = NULL;
	rtsCharIo = NULL;
	rtsSystemCallsIo = NULL;
}

void setCurrentTimeString() {

	// Create experimental time
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(experimentTimePrefix, 80, "%Y%m%d_%X", timeinfo);
}

void showLegend() {
	cout << "VEX: Virtual EXecution framework v0.9" << endl;
    cout << "VEX Copyright (C) 2009-2012 Nikos Baltas" << endl;
    cout << "VEX comes with ABSOLUTELY NO WARRANTY. " << endl;
    cout << "This is free software, and you are welcome to redistribute it under certain conditions. " << endl;
    cout << endl;
}

void cleanup() {
	delete manager;
	delete ioSimulator;	// also deleting ioProtocol
	if (eventLogger != NULL) {
		delete eventLogger;
	}
	if (threadEventsBehaviour != NULL) {
		delete threadEventsBehaviour;
	}
	if (methodEventsBehaviour != NULL) {
		delete methodEventsBehaviour;
	}
	delete registry;
	delete runnableThreads;
	delete managers;

	delete waitingOnObjectRegistry;
	delete lockRegistry;
	delete aggregateStateTransitionCounters;
	delete visualizer;
	delete[] outputDir;
	delete[] predictorType;

	delete globalIoParameters;
	if (ioImportDirFilePrefix != NULL) {
		delete[] ioImportDirFilePrefix;
	}

	if (ioExportDir != NULL) {
		delete[] ioExportDir;
	}

	if (serverHost != NULL) {
		delete[] serverHost;
	}
	if (invalidationPolicy != NULL) {
		delete invalidationPolicy;
	}

#ifdef USING_PAPIPROFILER
	if (hpcProfiler != NULL) {
		delete hpcProfiler;
	}
#endif

	delete virtualTimeline;
}


ThreadManager *selectManager(unsigned int id) {
	VirtualTimelineController *controller;
	if (processors == 1) {
		if (usingVtfScheduler) {
			controller = new SingleVirtualTimelineController(virtualTimeline);
		} else {
			controller = new PassiveVirtualTimelineController(virtualTimeline);
		}
	} else {
		controller = new MultipleVirtualTimelinesController(static_cast<MulticoreVirtualTimeline *>(virtualTimeline), id);
	}

	if (printGlobalTimeBuffers || printPerThreadTimeBuffers || printAllTimeSamples) {
		timeLoggingBehaviour[id] = new TimeLoggingBehaviour();
		if (printGlobalTimeBuffers) {
			timeLoggingBehaviour[id]->addGlobalLoggers(new TotalTime());
		} else if (printPerThreadTimeBuffers) {
			timeLoggingBehaviour[id]->addGlobalLoggers(new PerThreadTime());
		} else if (printAllTimeSamples) {
			timeLoggingBehaviour[id]->addGlobalLoggers(new SamplesPerThreadAnalysis());
		}
		controller = new StatisticsEnabledVirtualTimelineController(controller, timeLoggingBehaviour[id]);
	}

	if (usingVtfScheduler) {
		if (serverHost != NULL) {
			if (isServer) {
				if (serverPort != 0) {
					ThreadManagerServer *managerServer = new ThreadManagerServer(id, controller, runnableThreads, ioSimulator, registry, waitingOnObjectRegistry);
					managerServer->init(serverPort + 6*id);		// different ports per server
					managerServer->startManagerServerThread();
					return managerServer;
				} else {
					cerr << "Wrong arguments for scheduler" << endl;
					return NULL;
				}
			} else {
				ThreadManagerClient *managerClient = new ThreadManagerClient(id, controller, runnableThreads, ioSimulator, registry, waitingOnObjectRegistry);
				isClient = true;
				return managerClient;
			}
		} else {
			if (processors == 1) {
				return new ThreadManager(id, controller, runnableThreads, ioSimulator, registry, waitingOnObjectRegistry);
			} else {
				if (strictParallelScheduling) {
					return new StrictParallelThreadManager(id, controller, runnableThreads, ioSimulator, registry, waitingOnObjectRegistry);
				} else {
//					return new ThreadManager(id, controller, runnableThreads, ioSimulator, registry, waitingOnObjectRegistry);
					return new LooseParallelThreadManager(id, controller, runnableThreads, ioSimulator, registry, waitingOnObjectRegistry);
				}
			}
		}
	} else {
		globalIoParameters->setValue("io", 		"none");
		globalIoParameters->setValue("iopred", 	"none");
		ioSimulator = IoSimulatorFactory::create(globalIoParameters);
		//ioSimulator = IoSimulatorFactory::create(ioProtocol, createPredictor(), ioRecognizer, ioRandomExcluder);

		//ioSimulator = new IoSimulator(new IoProtocol(), globalIoPnew NoIoPrediction());
		return new PassiveManager(controller, runnableThreads, ioSimulator, registry, waitingOnObjectRegistry);
	}

}


VirtualTimeline *selectVirtualTime() {
	if (processors == 1) {
		return new VirtualTimeline();
	} else {
		if (strictParallelScheduling) {
			return new DisablingMulticoreVirtualTimeline(processors);
		} else {
			return new MulticoreVirtualTimeline(processors);
		}
	}
}


bool usingRealTimeSimulationByCategories() {
	return rtsBlockIo != NULL && rtsCharIo != NULL && rtsSystemCallsIo != NULL;
}

void resetProfilerType(const bool &_stackTraceMode) {
	if (eventLogger != NULL) {
		delete eventLogger;
	}
	if (!_stackTraceMode) {
		eventLogger = new EventLogger();
	} else {
		eventLogger = new StackTraceLogger();
	}
	eventLogger->setPrunePercentage(prunePercentage);

	if (threadEventsBehaviour != NULL) {
		threadEventsBehaviour->setProfilerType(eventLogger, _stackTraceMode);
	}

}

int getSimulationHostProcessors() {
	int physicalProcessors = 0;
	ifstream dfile("/proc/cpuinfo");
	string line;
	while (!dfile.eof()) {
		getline(dfile, line);
		if (strncmp(line.c_str(),  "processor", 9) == 0) {
			++physicalProcessors;
		}
	}
	return physicalProcessors;
}


bool setupSimulationEnvironment() {
	if (!Time::onSystemInit()) {
		exit(1);
	}

	if (!usingVtfScheduler && processors > 1) {
		cerr << endl << "VEX Warning: Invalid parameter combination: VEX's no_scheduling cannot be used more than one virtual cores. Ignoring no_scheduling" << endl;
		usingVtfScheduler = true;
	}

	if (outputFormat == EventLogger::HPROF_LIKE_OUTPUT && stackTraceMode) {
		cerr << endl << "VEX Warning: Invalid parameter combination: VEX's output_format=hprof cannot be used with stack_trace_mode. Ignoring stack_trace_mode" << endl;
		stackTraceMode = false;
	}

	int physicalProcessors = getSimulationHostProcessors();
	if (8 * physicalProcessors <= processors) {
		cerr << endl << "VEX Warning: Too few physical cores to run VEX simulation..." << endl;
		NativeWaitingCriteriaFactory::setCriteriaType("combined");
		cerr << "Setting combined native waiting identification criteria" << endl;
//		schedulerTimeslot *= ((processors / physicalProcessors) < 32) ? (processors / physicalProcessors) : 32;
//		schedulerTimeslot *= (processors  / (physicalProcessors));
//		cerr << "Adapting scheduler timeslot to " << schedulerTimeslot/1e6 << " ms"<< endl << endl;
	}

	srand(time(NULL));
	virtualTimeline = selectVirtualTime();

	invalidationPolicy = NULL;
	runnableThreads = new ThreadQueue();

	if (printGlobalTimeBuffers || printPerThreadTimeBuffers || printAllTimeSamples) {
		timeLoggingBehaviour = new TimeLoggingBehaviour*[processors];
	}


	if (printGlobalIoBuffers) {
		globalLoggingBehaviour = new IoLoggingBehaviour();
		globalLoggingBehaviour->addGlobalIoLoggers();
	}

	if (usingRealTimeSimulationByCategories()) {
		rtsBlockIo->setLoggingBehaviour(globalLoggingBehaviour, printIoBuffers);
		rtsCharIo->setLoggingBehaviour(globalLoggingBehaviour, printIoBuffers);
		rtsSystemCallsIo->setLoggingBehaviour(globalLoggingBehaviour, printIoBuffers);

		ioSimulator = new StandardCategoryIoSimulator(IoSimulatorFactory::create(rtsBlockIo), IoSimulatorFactory::create(rtsCharIo), IoSimulatorFactory::create(rtsSystemCallsIo), globalLoggingBehaviour, printIoBuffers);

	} else {
		globalIoParameters->setLoggingBehaviour(globalLoggingBehaviour, printIoBuffers);
		ioSimulator = IoSimulatorFactory::create(globalIoParameters);
		//ioSimulator = IoSimulatorFactory::create(ioProtocol, createPredictor(), ioRecognizer, ioRandomExcluder);
	}


	if (pollingNativeWaitingThreads) {
		registry = new NwPollingThreadRegistry(processors);
	} else {
		registry = new ThreadRegistry(processors);
	}
	waitingOnObjectRegistry = new ObjectRegistry();

	if (!reentrantLocking) {
		lockRegistry = new LockRegistry();
	} else {
		lockRegistry = new ReentrantLockRegistry();
	}
	aggregateStateTransitionCounters = new AggregateStateCounters();

	managers = new ThreadManagerRegistry(processors);
	for (int i=0; i<processors; i++) {
		ThreadManager *newManager = selectManager(i);
		if (!managers->addThreadManager(newManager)) {
			return false;
		}

		if ((errno = pthread_create(new pthread_t, NULL, pthreadManagerWorker, newManager)) != 0) {
			perror("Failed to create scheduler threads");
		}
	}
	manager = managers->getDefaultManager();

	managers-> setDefaultSchedulerTimeslot(schedulerTimeslot);

	if (printSchedulerStats) {
		managers-> enableSchedulerStats();
	}

	resetProfilerType(stackTraceMode);

	if (enableVisualizer) {
		visualizer = new Visualizer(virtualTimeline, eventLogger);
		managers->setVisualizer(visualizer);
		VexThreadState::setVisualizer(visualizer);
	}

	if (enableManagerLogging) {
		Log *globalLog = new Log("vtf_manager_log", virtualTimeline);
		managers->setLog(globalLog);
		runnableThreads->setLog(globalLog);
		registry->setLog(globalLog);
	}


	if (profilingHPC) {
#ifdef USING_PAPIPROFILER
		hpcProfiler = new PapiProfiler(!Time::usingPapiTimer());
#else
		cerr << "Cannot use PAPI profiling - VEX has not been correctly setup to use PAPI. Remove \"profiling_HPC\" parameter and retry. Aborting..." << endl;
		return false;
#endif
	} else {
		hpcProfiler = NULL;
	}

	threadEventsBehaviour = new ThreadEventsBehaviour(managers, registry, eventLogger, waitingOnObjectRegistry, lockRegistry, aggregateStateTransitionCounters, stackTraceMode, usingVtfScheduler, hpcProfiler);
	methodEventsBehaviour = new MethodEventsBehaviour(ioSimulator, reportIoMethods, virtualTimeline);

	if (delaysFile == NULL) {
		Time::parseDelaysFile(DEFAULT_DELAYS_FILE);
	}
	if (excludedThreadsFile != NULL) {
		threadEventsBehaviour->registerThreadsToBeExcludedFromSimulation(excludedThreadsFile);
	}

	VexThreadState::setDefaultThreadManager(managers->getDefaultManager());
	VexThreadState::setThreadManagerRegistry(managers);
	Timers::setGlobalScalingFactor(globalScalingFactor);

	// TODO incorporate somehow in virtualTimelineController
	VexThreadCustomer::calculateIterationsPerNs();

	if (profilingYieldDuration) {
		Time::measureYieldDuration();
	}
	return true;

}



}
//~VTF() {
//	 
//}
