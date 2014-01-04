/*
 * IoPredictor.cpp
 *
 *  Created on: 16 Aug 2011
 *      Author: root
 */

#include "IoPredictor.h"
#include "PredictionMethodFactory.h"

#include <fstream>


void IoPredictor::init() {
	pthread_spin_init(&spinlock, 0);
	recognizer = NULL;
	excluder = NULL;
	maxNumberOfThreadsPerformingIoInParallel = 32;
//		pthread_mutex_init(&mutex, NULL);
}

IoPredictor::~IoPredictor() {
	pthread_spin_destroy(&spinlock);
//	pthread_mutex_destroy(&mutex);

}

void IoPredictor::lock() {
	pthread_spin_lock(&spinlock);
//	pthread_mutex_lock(&mutex);
}

void IoPredictor::unlock() {
	pthread_spin_unlock(&spinlock);
//	pthread_mutex_unlock(&mutex);
}

void IoPredictor::logOnIoStart(const long long &prediction, VexThreadState *state) {
	if (globalLoggingBehaviour != NULL) {
		state->lockShareResourceAccessKey();
		globalLoggingBehaviour->logOnIoStart(prediction, state, threadsInIo);
		state->unlockShareResourceAccessKey();
	}
}
void IoPredictor::logOnIoEnd(const long long &realTime, VexThreadState *state) {
	if (globalLoggingBehaviour != NULL) {
		state->lockShareResourceAccessKey();
		globalLoggingBehaviour->logOnIoEnd(realTime, state, threadsInIo);
		state->unlockShareResourceAccessKey();
	}
}


long long IoPredictor::onIoEntryGetIoPrediction(VexThreadState *state) {
	state->lockShareResourceAccessKey();
	increaseThreadsInIo();
	state->unlockShareResourceAccessKey();
	long long prediction = getPredictionValue(state);

	if (recognizer != NULL && !recognizer->isRecognizedAsIoOperation(state, prediction)) {
		state->recognizedCallAsCached();
		state->lockShareResourceAccessKey();
		decreaseThreadsInIo();
		state->unlockShareResourceAccessKey();
	} else if (excluder != NULL && excluder->shouldRandomlyExcludeIo()) {
		// we do not decrease the threads in I/O count here, because the thread is still performing the I/O
		// (despite the fact that we are not simulating it or measuring its time in real time)
		state->doNotRegardAsIo();
	}
	logOnIoStart(prediction, state);

//	if (prediction > 50000000) {
//		cout << state->getName() << " returned a prediction of " << prediction << endl;
//	}
	return prediction;
}


void IoPredictor::onIoExitAddRealTimeMeasurement(VexThreadState *state, const long long &realTimeValueOnExit) {


	if (!state->callRecognizedAsCached()) {
		state->lockShareResourceAccessKey();
		decreaseThreadsInIo();
		state->unlockShareResourceAccessKey();
	}


	logOnIoEnd(realTimeValueOnExit, state);
	addPredictionInfo(state, realTimeValueOnExit);
}


// Print both global and local stats, if they are defined, under the outputDir directory
void IoPredictor::printIoStats(const char *outputDir) {
	char filename[256];
	sprintf(filename, "%s/vtf_io_stats.csv", outputDir);
	if (globalLoggingBehaviour != NULL) {
		globalLoggingBehaviour->print(filename, predictionMethodFactory->getPredictionMethodTitle());
		globalLoggingBehaviour->printExtraStats(outputDir);
	}
	if (predictionMethodFactory != NULL && predictionMethodFactory->isLocalLoggingEnabled()) {
		printLocalLogging(outputDir, filename);
	}
}

void IoPredictor::printLocalLogging(const char *outputDir, const char *filename) {

}

void IoPredictor::exportMeasurements(const char *outputDirFileName) {

}

unsigned int IoPredictor::increaseThreadsInIo() {
	lock();
	unsigned int returnValue = ++threadsInIo;
	unlock();
	return returnValue;
}

unsigned int IoPredictor::decreaseThreadsInIo() {
	lock();
	unsigned int returnValue = --threadsInIo;
	unlock();
	return returnValue;
}

unsigned int IoPredictor::getThreadsInIo() {
//	lock();
//	unsigned int returnValue = threadsInIo;
//	unlock();
	return threadsInIo; //returnValue;
}



NoIoPrediction::~NoIoPrediction() {

}

long long NoIoPrediction::getPredictionValue(VexThreadState *state) {
	return 0;
}

void NoIoPrediction::addPredictionInfo(VexThreadState *state, const long long &realTimeValueOnExit) {

}



GlobalIoPrediction::GlobalIoPrediction(PredictionMethodFactory *_predictionMethodFactory) {
	init();
	predictionMethodFactory = _predictionMethodFactory;
	predictionMethods = new PredictionMethod *[maxNumberOfThreadsPerformingIoInParallel];

	for (unsigned int i = 0; i<maxNumberOfThreadsPerformingIoInParallel; i++) {
		IoState perThreadsIoPoint;
		perThreadsIoPoint.setThreadsInIo(i);
		predictionMethods[i] = predictionMethodFactory->getPredictionMethod(perThreadsIoPoint);
	}


}

GlobalIoPrediction::~GlobalIoPrediction() {
	for (unsigned int i = 0; i<maxNumberOfThreadsPerformingIoInParallel; i++) {
		delete predictionMethods[i];
	}
	delete[] predictionMethods;
}

long long GlobalIoPrediction::getPredictionValue(VexThreadState *state) {

	PredictionMethod *pm = predictionMethods[getThreadsInIo()-1];
	state->lockShareResourceAccessKey();
	long long lastIoPrediction = pm->getNext();
	state->unlockShareResourceAccessKey();
	state->setIoPredictionInfo(pm, lastIoPrediction);
	return lastIoPrediction;
}

void GlobalIoPrediction::addPredictionInfo(VexThreadState *state, const long long &realTimeValueOnExit) {
	int threadsCurrentlyInIo = getThreadsInIo();	// omg, here the threadsInIo has already been decreased by one so you should not do it again.... this is really bad! TODO: fix
	return predictionMethods[threadsCurrentlyInIo]->addPredictionInfo(realTimeValueOnExit, state, threadsCurrentlyInIo);
}


void GlobalIoPrediction::printLocalLogging(const char *outputDir, const char *filename) {

	std::filebuf fb;
	if (globalLoggingBehaviour != NULL) {
		fb.open(filename, std::ios::out | std::ios::app);
	} else {
		fb.open(filename, std::ios::out);
	}
	std::ostream os(&fb);
	if (globalLoggingBehaviour != NULL) {
		os << endl << endl << endl;
	}

	os << "*** " << predictionMethodFactory->getPredictionMethodTitle() << " globally ***" << endl;
	bool titleWritten = false;

	for (unsigned int threads = 0; threads < maxNumberOfThreadsPerformingIoInParallel; threads++) {
		IoLoggingBehaviour *log = predictionMethods[threads]->getLog();
		if (log != NULL && log->hasLoggedSamples()) {
			if (!titleWritten) {
				os << "Threads in I/O,";
				log->getTitlesInCSVRow(os);
				os << endl;
				titleWritten = true;
			}

			os << threads << "," ;
			log->getValuesInCSVRow(os);
			os << endl;

			IoState ioState(threads, 0, 0);
			log->printExtraStats(outputDir, ioState);
		}
	}
	fb.close();


}


void GlobalIoPrediction::exportMeasurements(const char *outputDirFileName) {

	for (unsigned int i = 0; i<maxNumberOfThreadsPerformingIoInParallel; i++) {
		IoState perThreadsIoPoint;
		perThreadsIoPoint.setThreadsInIo(i);
		predictionMethods[i]->exportMeasurements(outputDirFileName, perThreadsIoPoint);
	}

}


/**
 * Different prediction per invocation point
 */
StateAwareIoPrediction::StateAwareIoPrediction(PredictionMethodFactory *_predictionMethodFactory, const unsigned int &_maxNumberOfThreadsPerformingIoInParallel, IoLoggingBehaviour *_log) {
	globalLoggingBehaviour = _log;
	maxNumberOfThreadsPerformingIoInParallel = _maxNumberOfThreadsPerformingIoInParallel;
	predictionMethodFactory = _predictionMethodFactory;
	init();
}
StateAwareIoPrediction::StateAwareIoPrediction(PredictionMethodFactory *_predictionMethodFactory, const unsigned int &_maxNumberOfThreadsPerformingIoInParallel) {
	globalLoggingBehaviour 	= NULL;
	maxNumberOfThreadsPerformingIoInParallel = _maxNumberOfThreadsPerformingIoInParallel;
	predictionMethodFactory = _predictionMethodFactory;
	init();
}

void StateAwareIoPrediction::init() {

	predictionsPerInvocationPoint = new map<int, PredictionMethod *>[maxNumberOfThreadsPerformingIoInParallel];
	invocationPointToMethodId = new map<int, int>[maxNumberOfThreadsPerformingIoInParallel];

	mutex = new pthread_mutex_t[maxNumberOfThreadsPerformingIoInParallel];
	for (unsigned int i = 0; i<maxNumberOfThreadsPerformingIoInParallel; i++) {
		pthread_mutex_init(&mutex[i], NULL);
	}

}

StateAwareIoPrediction::~StateAwareIoPrediction() {
	for (unsigned int i = 0; i<maxNumberOfThreadsPerformingIoInParallel; i++) {
		pthread_mutex_destroy(&mutex[i]);
	}
	delete mutex;
	delete[] predictionsPerInvocationPoint;
	delete[] invocationPointToMethodId;
}

PredictionMethod *StateAwareIoPrediction::getPredictionMethodOfInvocationPoint(IoState &ioState) {
	unsigned int threadsInIO = ioState.getThreadsInIO(maxNumberOfThreadsPerformingIoInParallel);

	map<int, PredictionMethod *> *currentStatus;
	map<int, PredictionMethod *>::iterator current_invocation_point_trace_iterator;

	pthread_mutex_lock(&mutex[threadsInIO]);
	currentStatus = &(predictionsPerInvocationPoint[threadsInIO]);
	current_invocation_point_trace_iterator = currentStatus->find(ioState.getTraceHash());

	PredictionMethod *iti;
	// Check whether the invocation point is already registered
	if (current_invocation_point_trace_iterator != currentStatus->end()) {
		iti = current_invocation_point_trace_iterator->second;
	} else {
		// Register the invocation point
		iti = predictionMethodFactory->getPredictionMethod(ioState);
		(*currentStatus)[ioState.getTraceHash()] = iti;

		map<int, int> *currentMethodId;
		currentMethodId = &(invocationPointToMethodId[threadsInIO]);
		(*currentMethodId)[ioState.getTraceHash()]= ioState.getIoOperation();
	}
	pthread_mutex_unlock(&mutex[threadsInIO]);

	return iti;

}

long long StateAwareIoPrediction::getPredictionValue(VexThreadState *state) {
	IoState ioState(threadsInIo, state->getIoUniqueInvocationPoint(), state->getCurrentMethodId());


	//TODO: improve performance by putting locking inside the getPredictionMethodOfInvocationPoint
	state->lockShareResourceAccessKey();	// used to protect allocation
	PredictionMethod *predictionMethod = getPredictionMethodOfInvocationPoint(ioState);

//	PredictionMethod *predictionMethod = getPredictionMethodOfInvocationPoint(threadsInIo, state->getIoHashValue());
	long long lastIoPrediction = predictionMethod->getNext();
	state->unlockShareResourceAccessKey();	// used to protect allocation and rand()

	state->setIoPredictionInfo(predictionMethod, lastIoPrediction);

	return lastIoPrediction;
}

void StateAwareIoPrediction::addPredictionInfo(VexThreadState *state, const long long &realTimeValueOnExit) {
	PredictionMethod *predictionMethod = state->getLastIoPredictionMethod(); // cached
	if (predictionMethod == NULL) {
		IoState ioState(threadsInIo, state->getIoUniqueInvocationPoint());

		//TODO: improve performance by putting locking inside the getPredictionMethodOfInvocationPoint
		state->lockShareResourceAccessKey();	// used to protect allocation
		predictionMethod = getPredictionMethodOfInvocationPoint(ioState);
		state->unlockShareResourceAccessKey();	// used to protect allocation

//		predictionMethod = getPredictionMethodOfInvocationPoint(threadsInIo, state->getIoHashValue());
	}
	predictionMethod->addPredictionInfo(realTimeValueOnExit, state, threadsInIo);
}

void StateAwareIoPrediction::printLocalLogging(const char *outputDir, const char *filename) {

	std::filebuf fb;
	if (globalLoggingBehaviour != NULL) {
		fb.open(filename, std::ios::out | std::ios::app);
	} else {
		fb.open(filename, std::ios::out);
	}
	std::ostream os(&fb);
	if (globalLoggingBehaviour != NULL) {
		os << endl << endl << endl;
	}

	os << "*** " << predictionMethodFactory->getPredictionMethodTitle() << " per I/O invocation point ***" << endl;
	map<int, int> *currentMethodId;
	bool titleWritten = false;
	for (unsigned int threadsInIO = 0; threadsInIO < maxNumberOfThreadsPerformingIoInParallel; threadsInIO++) {
		map<int, PredictionMethod *> *currentStatus;
		map<int, PredictionMethod *>::iterator current_invocation_point_trace_iterator;

		pthread_mutex_lock(&mutex[threadsInIO]);
		currentStatus = &(predictionsPerInvocationPoint[threadsInIO]);
		currentMethodId = &(invocationPointToMethodId[threadsInIO]);
		current_invocation_point_trace_iterator = currentStatus->begin();

		while (current_invocation_point_trace_iterator != currentStatus->end()) {
			int traceHash = current_invocation_point_trace_iterator->first;
			PredictionMethod *iti = current_invocation_point_trace_iterator->second;
			IoLoggingBehaviour *log = iti->getLog();
			if (log != NULL && log->hasLoggedSamples()) {
				if (!titleWritten) {
					os << "Threads,TraceHash,I/O operation,";
					log->getTitlesInCSVRow(os);
					os << endl;
					titleWritten = true;
				}

				os << threadsInIO << "," << traceHash << "," << (*currentMethodId)[traceHash] << ",";
				log->getValuesInCSVRow(os);
				os << endl;

				IoState ioState(threadsInIO, traceHash);
				log->printExtraStats(outputDir, ioState);
			}
			++current_invocation_point_trace_iterator;
		}
		pthread_mutex_unlock(&mutex[threadsInIO]);

	}
	fb.close();

}


void StateAwareIoPrediction::exportMeasurements(const char *outputDirFileName) {
	for (unsigned int threadsInIO = 0; threadsInIO < maxNumberOfThreadsPerformingIoInParallel; threadsInIO++) {
		map<int, PredictionMethod *> *currentStatus;
		map<int, PredictionMethod *>::iterator current_invocation_point_trace_iterator;

		pthread_mutex_lock(&mutex[threadsInIO]);
		currentStatus = &(predictionsPerInvocationPoint[threadsInIO]);
		current_invocation_point_trace_iterator = currentStatus->begin();
		while (current_invocation_point_trace_iterator != currentStatus->end()) {
			int traceHash = current_invocation_point_trace_iterator->first;
			PredictionMethod *pm = current_invocation_point_trace_iterator->second;
			IoState ioState(threadsInIO, traceHash);
			pm->exportMeasurements(outputDirFileName, ioState);
			++current_invocation_point_trace_iterator;
		}
		pthread_mutex_unlock(&mutex[threadsInIO]);
	}
}




void OperationAwareIoPrediction::init() {

	predictionsPerIoOperation = new PredictionMethod **[maxNumberOfThreadsPerformingIoInParallel];
	for (unsigned int j=0; j<maxNumberOfThreadsPerformingIoInParallel; j++) {
		predictionsPerIoOperation[j] = new PredictionMethod *[TOTAL_IO_OPERATIONS];

		for (int i=0; i<TOTAL_IO_OPERATIONS; i++) {
			IoState ioPoint(j, 0, i);
			predictionsPerIoOperation[j][i] = predictionMethodFactory->getPredictionMethod(ioPoint);
		}
	}
}

OperationAwareIoPrediction::~OperationAwareIoPrediction() {
	for (unsigned int j=0; j<maxNumberOfThreadsPerformingIoInParallel; j++) {
		for (int i=0; i<TOTAL_IO_OPERATIONS; i++) {
			delete predictionsPerIoOperation[j][i];
		}
		delete[] predictionsPerIoOperation[j];
	}
	delete[] predictionsPerIoOperation;
}

long long OperationAwareIoPrediction::getPredictionValue(VexThreadState *state) {
	int ioMethodId = state->getCurrentMethodId();
	PredictionMethod *pm = predictionsPerIoOperation[getThreadsInIo()][ioMethodId];
	state->lockShareResourceAccessKey();
	long long lastIoPrediction = pm->getNext();
	state->unlockShareResourceAccessKey();
	state->setIoPredictionInfo(pm, lastIoPrediction);

	return lastIoPrediction;
}

void OperationAwareIoPrediction::addPredictionInfo(VexThreadState *state, const long long &realTimeValueOnExit) {
	PredictionMethod *predictionMethod = state->getLastIoPredictionMethod(); // cached
	int threadsCurrentlyInIo = getThreadsInIo();
	if (predictionMethod == NULL) {
		predictionMethod = predictionsPerIoOperation[threadsCurrentlyInIo][state->getCurrentMethodId()];
	}
	predictionMethod->addPredictionInfo(realTimeValueOnExit, state, threadsCurrentlyInIo);
}


void OperationAwareIoPrediction::printLocalLogging(const char *outputDir, const char *filename) {

	std::filebuf fb;
	if (globalLoggingBehaviour != NULL) {
		fb.open(filename, std::ios::out | std::ios::app);
	} else {
		fb.open(filename, std::ios::out);
	}
	std::ostream os(&fb);
	if (globalLoggingBehaviour != NULL) {
		os << endl << endl << endl;
	}

	os << "*** " << predictionMethodFactory->getPredictionMethodTitle() << " per I/O operation ***" << endl;
	bool titleWritten = false;

	for (unsigned int threads = 0; threads < maxNumberOfThreadsPerformingIoInParallel; threads++) {

		for (int methodId = 1; methodId < TOTAL_IO_OPERATIONS; methodId++) {
			IoLoggingBehaviour *log = predictionsPerIoOperation[threads][methodId]->getLog();
			if (log != NULL && log->hasLoggedSamples()) {
				if (!titleWritten) {
					os << "Threads in I/O, I/O operation id,";
					log->getTitlesInCSVRow(os);
					os << endl;
					titleWritten = true;
				}

				os << threads << "," << methodId << ",";
				log->getValuesInCSVRow(os);
				os << endl;

				IoState ioState(threads, 0, methodId);
				log->printExtraStats(outputDir, ioState);
			}
		}

	}
	fb.close();


}


void OperationAwareIoPrediction::exportMeasurements(const char *outputDirFileName) {
	for (unsigned int threads = 0; threads < maxNumberOfThreadsPerformingIoInParallel; threads++) {

		for (int methodId = 1; methodId < TOTAL_IO_OPERATIONS; methodId++) {
			IoState ioState(threads, 0, methodId);
			predictionsPerIoOperation[threads][methodId]->exportMeasurements(outputDirFileName, ioState);
		}

	}
}
