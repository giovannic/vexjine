#include "IoSimulator.h"
#include "ThreadManager.h"
#include "MethodPerformanceModel.h"

IoSimulator::IoSimulator() {
	protocol = NULL;
	predictor = NULL;
}

IoSimulator::IoSimulator(IoProtocol *_protocol, IoPredictor *_predictor) {
	protocol = _protocol;
	predictor= _predictor;
}

IoSimulator::~IoSimulator() {
	if (protocol != NULL) delete protocol;
	if (predictor != NULL) delete predictor;
}

void IoSimulator::startIo(VexThreadState *state) {
//	assert(state->getState() != LEARNING_IO && state->getState() != IN_IO);
	predictor->onIoEntryGetIoPrediction(state);
	protocol->onStart(state);
}


void IoSimulator::endIo(VexThreadState *state, const long long &realTimeValueOnExit) {
	// TODO: IMPROVEMENT: try out various different logging real time techniques
	long long actualIoDuration = (state->getTimeScalingFactor()*(realTimeValueOnExit - state->getLastRealTime()));
	predictor->onIoExitAddRealTimeMeasurement(state, actualIoDuration);
	protocol->onEnd(state, actualIoDuration);

}

bool IoSimulator::areInIoThreadsInRunnablesQueue() {
	return protocol->areInIoThreadsInRunnablesQueue();
}

void IoSimulator::printIoStats(const char *outputDirectory) {
	predictor->printIoStats(outputDirectory);
}
void IoSimulator::printLocalLogging(const char *outputDir, const char *filename) {
	predictor->printLocalLogging(outputDir, filename);
}
void IoSimulator::exportMeasurements(const char *outputDirFileName) {
	predictor->exportMeasurements(outputDirFileName);
}



/***
 * Queueing network simulation
 */
QueueingNetworkIoSimulator::QueueingNetworkIoSimulator(const char *modelFilename) {
	queueingNetworkDescribingIo = new MethodPerformanceModel(modelFilename, true);
	queueingNetworkDescribingIoMetadata = new MethodPerformanceModelMetadata(queueingNetworkDescribingIo->getSourceNodeWithLabel(0), 0);
	ioQNSimulationLoggingBehaviour = NULL;
	threadsInIo = 0;
}

// TODO: this constructor should be used to enable different I/O operations to use the same model
QueueingNetworkIoSimulator::QueueingNetworkIoSimulator(const char *modelFilename, const char *sourceNodeLabel, const int &customerClass) {
	queueingNetworkDescribingIo = new MethodPerformanceModel(modelFilename, true);
	queueingNetworkDescribingIoMetadata = new MethodPerformanceModelMetadata(queueingNetworkDescribingIo->getSourceNodeWithLabel(sourceNodeLabel), customerClass);
	ioQNSimulationLoggingBehaviour = NULL;
	threadsInIo = 0;

}

QueueingNetworkIoSimulator::QueueingNetworkIoSimulator(const char *modelFilename, IoLoggingBehaviour *_ioQNSimulationLoggingBehaviour) {
	queueingNetworkDescribingIo = new MethodPerformanceModel(modelFilename, true);
	queueingNetworkDescribingIoMetadata = new MethodPerformanceModelMetadata(queueingNetworkDescribingIo->getSourceNodeWithLabel(0), 0);
	ioQNSimulationLoggingBehaviour = _ioQNSimulationLoggingBehaviour;
	threadsInIo = 0;
}

void QueueingNetworkIoSimulator::startIo(VexThreadState *state) {
//	cout << state->getName() << " starting model I/O: " << state->getEstimatedRealTime()/1000000 << endl;
	if (ioQNSimulationLoggingBehaviour != NULL) {
		state->setLastIoPrediction(state->getEstimatedRealTime());
	}
	queueingNetworkDescribingIo->simulate(state, queueingNetworkDescribingIoMetadata);
	state->resetThreadIntoVEX();		// only a single VEX call will be made to beforeIoMethodExit - no need to "hide" the threadstate

	if (ioQNSimulationLoggingBehaviour != NULL) {
		++threadsInIo;
		state->updateRealTimeClock();
	}
}

void QueueingNetworkIoSimulator::endIo(VexThreadState *state, const long long &actualIoDuration) {

	state->blockUntilModelSimulationEnd();
	state->endModelSimulation();
//	cout << state->getName() << " ending model I/O: " << state->getEstimatedRealTime()/1000000 << " " << state->doicurrentlyOwnLock() << endl;

	if (ioQNSimulationLoggingBehaviour != NULL) {
		state->setLastIoPrediction(state->getEstimatedRealTime() - state->getLastIoPrediction());
//		cout << "inserting actual " << (actualIoDuration-state->getLastRealTime()) << " vs prediction " << state->getLastIoPrediction() << endl;
		long long realTimeDiff = actualIoDuration-state->getLastRealTime();
		ioQNSimulationLoggingBehaviour->logOnIoEnd(realTimeDiff, state, threadsInIo);
		--threadsInIo;
		state->setLastIoPrediction(0);
	}


	state->lockShareResourceAccessKey();
	ThreadManager *currentManager = state->getThreadCurrentlyControllingManager();
	state->updateCpuTimeClock();
	currentManager -> suspendModelSimulationFinishingThread(state);

	state->unlockShareResourceAccessKey();
//	cout << state->getName() << " resuming after model I/O at: " << state->getEstimatedRealTime()/1000000 << " " << state->doicurrentlyOwnLock() << endl;

}

bool QueueingNetworkIoSimulator::areInIoThreadsInRunnablesQueue() {
	return false;
}

// The statistics are only stored to global statistics - there is no prediction policy to store buffers
void QueueingNetworkIoSimulator::printIoStats(const char *outputDirectory) {

}

void QueueingNetworkIoSimulator::printLocalLogging(const char *outputDir, const char *filename) {
//	if (ioQNSimulationLoggingBehaviour != NULL) {
//		std::filebuf fb;
//		fb.open(filename, std::ios::out | std::ios::app);
//		std::ostream os(&fb);
//
//		os << "Queueing network I/O simulation" << endl;
//		ioQNSimulationLoggingBehaviour->printStatsIntoStream(os);
//		fb.close();
//
//		ioQNSimulationLoggingBehaviour->printExtraStats(outputDir);
//	}
}

void QueueingNetworkIoSimulator::exportMeasurements(const char *outputDirFileName) {

}




/**
 * The standard categorization is based on the hard-coded known I/O values
 */
void CategoryIoSimulator::startIo(VexThreadState *state) {
	IoSimulator *sim = getCategorySimulator(state);
	sim->startIo(state);
}


void CategoryIoSimulator::endIo(VexThreadState *state, const long long &actualIoDuration) {
	IoSimulator *sim = getCategorySimulator(state);
	sim->endIo(state, actualIoDuration);
}


IoSimulator *StandardCategoryIoSimulator::getCategorySimulator(VexThreadState *state) {
	int ioMethodId = state->getCurrentMethodId();
	IoSimulator *sim;
	if (ioMethodId >= 15 && ioMethodId < 31) {
		sim = systemCallsSimulator;
	} else if (ioMethodId == 13 || ioMethodId == 14) {
		sim = charIoSimulator;
	} else {
		sim = blockIoSimulator;
	}
	return sim;
}

bool StandardCategoryIoSimulator::areInIoThreadsInRunnablesQueue() {
	return blockIoSimulator->areInIoThreadsInRunnablesQueue() || charIoSimulator->areInIoThreadsInRunnablesQueue();
}

void StandardCategoryIoSimulator::printIoStats(const char *outputDir) {
	char filename[256];
	sprintf(filename, "%s/vtf_io_stats.csv", outputDir);
	if (globalLogging != NULL) {
		string s("RTS_logging");
		globalLogging->print(filename, s.c_str());//predictionMethodFactory->getPredictionMethodTitle());
		globalLogging->printExtraStats(outputDir);
	}

	printLocalLogging(outputDir, filename);
}

void StandardCategoryIoSimulator::printLocalLogging(const char *outputDir, const char *filename) {
	if (isLocalLoggingEnabled) {
		systemCallsSimulator->printLocalLogging(outputDir, filename);
		charIoSimulator->printLocalLogging(outputDir, filename);
		blockIoSimulator->printLocalLogging(outputDir, filename);
	}
}

void StandardCategoryIoSimulator::exportMeasurements(const char *outputDir) {
	char outputDirectory[256];
	sprintf(outputDirectory, "%ssystem_calls_exported", outputDir);
	systemCallsSimulator->exportMeasurements(outputDirectory);
	sprintf(outputDirectory, "%sstream_io_exported", outputDir);
	charIoSimulator->exportMeasurements(outputDirectory);
	sprintf(outputDirectory, "%sblock_io_exported", outputDir);
	blockIoSimulator->exportMeasurements(outputDirectory);
}



// Factory methods
IoSimulator *IoSimulatorFactory::create(IoParameters *parameters) {

	if (parameters->getModelFilename() != NULL) {
		return new QueueingNetworkIoSimulator(parameters->getModelFilename(), parameters->getGlobalLoggingBehaviour());

	} else {

		PredictionMethodFactory *_predictionMethodFactory = new PredictionMethodFactory(parameters);
		return new IoSimulator(parameters->getIoProtocol(), parameters->getIoPredictor(_predictionMethodFactory));

	}
}

/***
 * _randomExcluder: Disregards a percentage of the I/O operations
 */
//IoSimulator *IoSimulatorFactory::create(IoProtocol *_protocol, IoPredictor *_predictor, IoRecognizer *_randomExcluder) {
//	if (_recognizer == NULL) {
//		_predictor->setIoRecognizer(NULL);
//		return new IoSimulator(_protocol, _predictor, _recognizer, _randomExcluder);
//	} else {
//		return new RecognizingIoSimulator(_protocol, _predictor, _recognizer, _randomExcluder);
//	}
//}
