/*
 * IoPredictor.h: Define the level of prediction (global, total-state aware, I/O operation-aware)
 *
 *  Created on: 16 Aug 2011
 *      Author: root
 */

#ifndef IOPREDICTOR_H_
#define IOPREDICTOR_H_

#include "ThreadState.h"
#include "IoLogger.h"
#include "PredictionMethod.h"
#include "IoRecognizer.h"
#include <pthread.h>
#define TOTAL_IO_OPERATIONS 30

class PredictionMethodFactory;

class IoPredictor {
public:
	IoPredictor() : predictionMethodFactory(NULL), globalLoggingBehaviour(NULL) {init();};
	IoPredictor(PredictionMethodFactory *_predictionMethodFactory) : predictionMethodFactory(_predictionMethodFactory), globalLoggingBehaviour(NULL) {init();};
	IoPredictor(PredictionMethodFactory *_predictionMethodFactory, IoLoggingBehaviour *_globalLoggingBehaviour) : predictionMethodFactory(_predictionMethodFactory), globalLoggingBehaviour(_globalLoggingBehaviour) {init();};

	void init();

	virtual ~IoPredictor();

	virtual long long onIoEntryGetIoPrediction(VexThreadState *state);
	virtual long long getPredictionValue(VexThreadState *state) = 0;

	virtual void onIoExitAddRealTimeMeasurement(VexThreadState *state, const long long &realTimeValueOnExit);
	virtual void addPredictionInfo(VexThreadState *state, const long long &realTimeValueOnExit) = 0;

	void setGlobalLoggingBehaviour(IoLoggingBehaviour *_globalLoggingBehaviour) {
		globalLoggingBehaviour = _globalLoggingBehaviour;
	}

	// Print both global and local stats under the outputDir directory
	virtual void printIoStats(const char *outputDirectory);

	// Print local stats (I/O samples) under the outputDir directory
	virtual void printLocalLogging(const char *outputDir, const char *filename);

	// Export samples under the outputDirFileName in a csv file, one measurement per line
	virtual void exportMeasurements(const char *outputDirFileName);
	void setIoRecognizer(IoRecognizer *r) {
		recognizer = r;
	}
	void setIoExcluder(IoExcluder *excl) {
		excluder = excl;
	}
protected:
	void logOnIoStart(const long long &prediction, VexThreadState *state);
	void logOnIoEnd(const long long &realTime, VexThreadState *state);

	PredictionMethodFactory *predictionMethodFactory;
	IoLoggingBehaviour *globalLoggingBehaviour;
	IoRecognizer *recognizer;
	IoExcluder *excluder;

	unsigned int increaseThreadsInIo();
	unsigned int decreaseThreadsInIo();
	unsigned int getThreadsInIo();

	unsigned int maxNumberOfThreadsPerformingIoInParallel;
	unsigned int threadsInIo;

	void lock();
	void unlock();

private:
	pthread_spinlock_t spinlock;
	int spinint;
	pthread_mutex_t mutex;
};


class NoIoPrediction : public IoPredictor {
public:
	NoIoPrediction() : IoPredictor() {};
	virtual ~NoIoPrediction();

	long long getPredictionValue(VexThreadState *state);
	void addPredictionInfo(VexThreadState *state, const long long &realTimeValueOnExit);
};



class GlobalIoPrediction : public IoPredictor {
public:
	GlobalIoPrediction(PredictionMethodFactory *_predictionMethodFactory);
	virtual ~GlobalIoPrediction();

	long long getPredictionValue(VexThreadState *state);
	void addPredictionInfo(VexThreadState *state, const long long &realTimeValueOnExit);

	void printLocalLogging(const char *outputDir, const char *filename);
	void exportMeasurements(const char *outputDirFileName);
private:
	PredictionMethod **predictionMethods;

};



class StateAwareIoPrediction : public IoPredictor {
public:
	StateAwareIoPrediction(PredictionMethodFactory *_predictionMethodFactory) : IoPredictor(_predictionMethodFactory) {
		globalLoggingBehaviour 	= NULL;
		maxNumberOfThreadsPerformingIoInParallel = 32;
		init();
	}

	StateAwareIoPrediction(PredictionMethodFactory *_predictionMethodFactory, IoLoggingBehaviour *_globalLoggingBehaviour) : IoPredictor(_predictionMethodFactory, _globalLoggingBehaviour) {
		maxNumberOfThreadsPerformingIoInParallel = 32;
		init();
	};

	StateAwareIoPrediction(PredictionMethodFactory *_predictionMethodFactory, const unsigned int &_maxThreads, IoLoggingBehaviour *_globalLoggingBehaviour);
	StateAwareIoPrediction(PredictionMethodFactory *_predictionMethodFactory, const unsigned int &_maxThreads);
	virtual ~StateAwareIoPrediction();

	long long getPredictionValue(VexThreadState *state);
	void addPredictionInfo(VexThreadState *state, const long long &realTimeValueOnExit);
	void exportMeasurements(const char *outputDirFileName);
protected:
	void printLocalLogging(const char *outputDir, const char *filename);
	void init();
	PredictionMethod *getPredictionMethodOfInvocationPoint(IoState &ioState);

	map<int, PredictionMethod *> *predictionsPerInvocationPoint; // pointer to map of PredictionMethods - one per threads-in-I/O
	map<int, int> *invocationPointToMethodId;

	pthread_mutex_t *mutex;

};



class OperationAwareIoPrediction : public IoPredictor {
public:
	OperationAwareIoPrediction(PredictionMethodFactory *_predictionMethodFactory) : IoPredictor(_predictionMethodFactory) {
		init();
	} ;
	OperationAwareIoPrediction(PredictionMethodFactory *_predictionMethodFactory,  IoLoggingBehaviour *_globalLoggingBehaviour) : IoPredictor(_predictionMethodFactory, _globalLoggingBehaviour) {
		init();
	};
	virtual ~OperationAwareIoPrediction();

	long long getPredictionValue(VexThreadState *state);
	void addPredictionInfo(VexThreadState *state, const long long &realTimeValueOnExit);
	void exportMeasurements(const char *outputDirFileName);
protected:
	void init();
	void printLocalLogging(const char *outputDir, const char *filename);
	PredictionMethod ***predictionsPerIoOperation;

};

#endif /* IOPREDICTOR_H_ */
