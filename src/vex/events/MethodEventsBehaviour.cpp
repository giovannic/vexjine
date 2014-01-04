/*
 * MethodEventsBehaviour.cpp
 *
 *  Created on: 18 Oct 2011
 *      Author: root
 */

#include "MethodEventsBehaviour.h"
#include "ThreadManager.h"
#include "ProfilingInvalidationPolicy.h"
#include "VirtualTimeline.h"
#include "IoSimulator.h"
#include "MethodPerformanceModel.h"

#include <cassert>

MethodEventsBehaviour::MethodEventsBehaviour(IoSimulator *_ioSimulator, const bool &_reportIoMethods, VirtualTimeline *_virtualTimeline) {
	ioSimulator = _ioSimulator;
	invalidationPolicy = NULL;
	reportIoMethods = _reportIoMethods;
	defaultTimeScalingFactor = 1.0;
	virtualTimeline = _virtualTimeline;
}

MethodEventsBehaviour::~MethodEventsBehaviour() {

}


/****************************************
 * afterMethodEntry - returns true if method is recursive
 ****************************************/
bool MethodEventsBehaviour::afterMethodEntry(const int & methodId) {

	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state == NULL) {
		return false;
	}
	//	VISUALIZE_METHOD_EVENT(METHOD_ENTRY, state, methodId);
	LOG_LAST_VEX_METHOD(state)
	return state->onVexMethodEntry(methodId, getTimeScalingFactorOfMethod(methodId));
}





/***************************************
 * beforeMethodExit- returns true if method should be invalidated (adaptive profiling)
 ****************************************/
bool MethodEventsBehaviour::beforeMethodExit(const int & methodId) {
	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state == NULL) {
		return NULL;
	}

	PerformanceMeasure *methodLog = state->onVexMethodExit(methodId);
	//	VISUALIZE_METHOD_EVENT(METHOD_EXIT, state, methodId);

	bool shouldMethodBeInvalidated = (invalidationPolicy != NULL && !invalidationPolicy->keepOnProfiling(methodLog));
	state->onVexExit();

	LOG_LAST_VEX_METHOD(state)
	return shouldMethodBeInvalidated;

}


/****************************************
 * afterIoMethodEntry: triggers an I/O simulation
 ****************************************/
void MethodEventsBehaviour::afterIoMethodEntry(const int &methodId, const int &invocationPointHashValue, const bool &possiblyBlocking) {
	VexThreadState *state = VexThreadState::getCurrentThreadState();

	if (state == NULL || state->invalidIoInvocationPoint(invocationPointHashValue))  {
		return;
	}

	state->onVexIoMethodEntry(methodId, invocationPointHashValue, possiblyBlocking);
	//VISUALIZE_METHOD_EVENT(METHOD_IO_ENTER, state, methodId);

	// push it to thread stack
	if (reportIoMethods) {
		state->logMethodEntry(methodId, false);
	}
	ioSimulator->startIo(state);

}





/****************************************
 * beforeIoMethodExit: actions upon I/O simulation completion
 ****************************************/
void MethodEventsBehaviour::beforeIoMethodExit(const int & methodId) {

	// Used to account for real time of IO operation - realTimeValueOnExit difference from state->getLastRealTime()
	long long realTimeValueOnExit  = Time::getRealTimeBeforeMethodInstrumentation();

	// Get state of current thread
	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state == NULL || state->invalidIoInvocationPoint()) {
		return;
	}

	state->onVexIoMethodExit();

	// Only add time to measurements if I/O operation was valid
	ioSimulator->endIo(state, realTimeValueOnExit);
	state->updateExitingMethodInfo(methodId);

	// Opt-out method reporting with the no_io_reporting agent flag
	if (reportIoMethods) {
		state->logMethodExit(); // Note: do not lock scheduler here
	}

	//		VISUALIZE_METHOD_EVENT(METHOD_IO_EXIT, state, methodId);
	state->setCpuTimeClockAfterIo();	// next update will be based back again on CPU time
}





/****************************************
 * Called by methods whose performance is a result of model simulation
 * but the functional behaviour is a result of code execution
 *****************************************/
long long MethodEventsBehaviour::afterMethodEntryUsingPerformanceModel(const int &methodId) {

	VexThreadState *state = VexThreadState::getCurrentThreadState();
	if (state == NULL) {
		return 0;
	}

	state->onVexMethodEntry(methodId, 0);

	//		VISUALIZE_METHOD_EVENT(METHOD_ENTER, state, methodId);

	unordered_map<int,  pair<MethodPerformanceModelMetadata *, MethodPerformanceModel *> >::iterator methodModelsIt = methodPerfomanceModels.find(methodId);
	assert(methodModelsIt != methodPerfomanceModels.end());

	MethodPerformanceModelMetadata *performanceModelMethodMetadata = methodModelsIt->second.first;
	MethodPerformanceModel *methodPerformanceModel = methodModelsIt->second.second;
	methodPerformanceModel->simulate(state, performanceModelMethodMetadata);

	state->onVexExitWithoutTimeUpdate();
	// From this point on the thread state is in the runnables list and will not be suspend/resumed until the model

	LOG_LAST_VEX_METHOD(state)
	return state->getEstimatedRealTime();
}


/****************************************
 * Called upon completion of the method body of a model described method
 *****************************************/
void MethodEventsBehaviour::beforeMethodExitUsingPerformanceModel(const int &methodId) {

	VexThreadState *state = VexThreadState::forceGetCurrentThreadState();
	if (state == NULL) {
		return;
	}
	LOG_LAST_VEX_METHOD(state)
	state->onSynchronizeOnModelExit(methodId);

}


/*
 * Return the time scaling factor (TSF) of the method with id methodId
 */
const float &MethodEventsBehaviour::getTimeScalingFactorOfMethod(const int &methodId) {
	unordered_map<int, float>::iterator method_time_override_this_method = methodTimeScalingFactors.find(methodId);
	if (method_time_override_this_method != methodTimeScalingFactors.end()) {
		return 	method_time_override_this_method->second;
	}
	return defaultTimeScalingFactor;
}

/*
 * Register the invalidation policy to be used for adaptive profiling.
 * Invalidation policy refers to the conditions that have to be fulfilled,
 * so that a method is considered to be of minor performance significance and
 * its instrumentation code may be removed for performance and accuracy (in case of JIT) reasons
 */
void MethodEventsBehaviour::registerInvalidationPolicy(const char *invalidationPolicyInfo) {
	size_t ilength = strlen(invalidationPolicyInfo);
	size_t delimiterPosition = ilength;
	for (size_t i=0; i<ilength; i++) {
		if (invalidationPolicyInfo[i] == ':') {
			delimiterPosition = i;
			break;
		}
	}

	if (delimiterPosition != ilength) {
		char samplesLimit[32];
		char *secondInfo = (char *)&invalidationPolicyInfo[delimiterPosition+1];
		strncpy(samplesLimit, invalidationPolicyInfo, delimiterPosition);

		long long minimumProfiledTime = atoi(secondInfo);
		if (minimumProfiledTime == 0) {
			invalidationPolicy = new RelativePercentageProfilingInvalidationPolicy(atoi(samplesLimit), atof(secondInfo), virtualTimeline);
		} else {
			invalidationPolicy = new AbsoluteTimeProfilingInvalidationPolicy(atoi(samplesLimit), minimumProfiledTime);
		}

	} else {
		invalidationPolicy = new ProfilingInvalidationPolicy(atoi(invalidationPolicyInfo));
	}
}



/*
 * Register the model that describes the method with id methodId.
 * The model is an xml file describing the queueing network that describes the method.
 */
void MethodEventsBehaviour::registerMethodPerformanceModel(const int &methodId, const char *modelFilename, const char *sourceNodeLabel, const int &customerClass) {

	if (modelFilename != NULL) {

		unordered_map<int, pair<MethodPerformanceModelMetadata *, MethodPerformanceModel *> >::iterator methodModelsIt = methodPerfomanceModels.find(methodId);
		if (methodModelsIt == methodPerfomanceModels.end()) {
			std::string modelFilenameString(modelFilename);
			MethodPerformanceModel *performanceModel = NULL;
			unordered_map<std::string, MethodPerformanceModel *>::iterator modelFilenameIt = modelFilenamesToPerfomanceModels.find(modelFilenameString);
			if (modelFilenameIt == modelFilenamesToPerfomanceModels.end()) {
	//			cout << "-----------------------------------------\nNew model from file: " << modelFilename << endl;
				performanceModel = new MethodPerformanceModel(modelFilename, true);
	//			cout << "-----------------------------------------" << endl << endl;
				modelFilenamesToPerfomanceModels[modelFilenameString] = performanceModel;
			} else {
				performanceModel = modelFilenameIt->second;

			}

//			cout << methodId << " methodId has model: " << modelFilename << ": customerClass " << customerClass << " and node " <<   performanceModel->getSourceNodeWithLabel(sourceNodeLabel)->getName() << endl;
			MethodPerformanceModelMetadata *metadataForInvocationFromThisMethod = new MethodPerformanceModelMetadata(performanceModel->getSourceNodeWithLabel(sourceNodeLabel), customerClass);
			methodPerfomanceModels[methodId] = make_pair(metadataForInvocationFromThisMethod, performanceModel);
		}

	}
}

/*
 * Register the time scaling factor (TSF) of method with id methodId
 */
void MethodEventsBehaviour::registerMethodTimeScalingFactor(const int &methodId, const double &speedup) {
	methodTimeScalingFactors[methodId] = (float)speedup;
}


