/*
 * JVMOptimizer.cpp: Used to implement the iterative method that executes a program
 * a number of times to determine the optimal time-scaling factors (i.e. prioritize the
 * optimization efforts). It has not been implemented yet.
 *
 *  Created on: 21 Sep 2011
 *      Author: root
 */
#include "virtualtime_EventNotifier.h"
#include "VTF.h"
#include "JVMTIUtil.h"

using namespace VEX;


JNIEXPORT void JNICALL Java_virtualtime_EventNotifier__1exportAndProcessMainIterationResults(JNIEnv *env, jclass cl) {
//
//	static float temporaryTestingScalingFactorForMain = 0.75;
//	// Get main thread events
//	VexThreadState *state = VexThreadState::getCurrentThreadState();
//	if (state != NULL && strcmp(state->getName(), "main") == 0) {
////		std::cout << "exiting main with id " << mainMethodId << endl;
////		Methods::beforeMethodExit(mainMethodId);
//
//		// OUTPUT main MEASURES
//		ThreadManager *manager = state->getCurrentlyControllingManagerOf();
//		manager->onThreadEnd(state);
//
//#if COUNT_STATE_TRANSITIONS == 1
//		aggregateStateTransitionCounters->addTransitionsOf(state);
//#endif
//
//		eventLogger->onThreadEnd(state);
//		if (profilingHPC) {
//			hpcProfiler->onThreadEnd(state->getName());
//		}
//
//		++mainIterationsPerformedByVexOptimizer;
//		char *filename = new char[strlen(outputDir) + 256];
//
//		eventLogger -> setRegistryStats(registry->getTotalThreadsControlled(), managers->getTotalPosixSignalsSent(), aggregateStateTransitionCounters->getTotalInvocationPoints());
//		if (outputFormat == EventLogger::HPROF_LIKE_OUTPUT) {
//			sprintf(filename, "%s/vtf_java_hprof_txt%d.csv", outputDir, mainIterationsPerformedByVexOptimizer);
//			eventLogger->writeData(filename, outputFormat, virtualTimeline->getGlobalTime());
//		} else {
//			sprintf(filename, "%s/vtf_results%d.csv", outputDir, mainIterationsPerformedByVexOptimizer);
//			eventLogger->writeData(filename, outputFormat, virtualTimeline->getGlobalTime());
//		}
//
//		delete[] filename;
//
//		if (mainIterationsPerformedByVexOptimizer > 2) {
//			continueVexOptimizersIterations = false;
//		}
//		int methodId = 1025192454;
//		methodEventsBehaviour->registerMethodTimeScalingFactor(methodId, temporaryTestingScalingFactorForMain);
//		temporaryTestingScalingFactorForMain *= 0.75;
//
//		// RESET THE THREAD
////		state->resetTimeCounters();
//
////		if (state->getCurrentStackTrace() != NULL) {
////			delete state->getCurrentStackTrace();
////		}
//
//		Java_virtualtime_EventNotifier_resetPerformanceMeasures(env, cl);
////		Methods::afterMethodEntry(mainMethodId);
//	}
}



// Method that determines the termination condition - Currently dummy
JNIEXPORT jboolean JNICALL Java_virtualtime_EventNotifier__1shouldMainIterationContinue(JNIEnv *, jclass) {
	//return continueVexOptimizersIterations;
	return true;
}

