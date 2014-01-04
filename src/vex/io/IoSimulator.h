#ifndef IOSIMULATOR_H_
#define IOSIMULATOR_H_

#include "IoProtocol.h"
#include "IoPredictor.h"
#include "IoRecognizer.h"
#include "PredictionMethodFactory.h"

class MethodPerformanceModel;
class MethodPerformanceModelMetadata;

/***
 * The basic I/O simulator treats all operations according to the selected protocol
 */
class IoSimulator {
public:
	IoSimulator();
	IoSimulator(IoProtocol *_protocol, IoPredictor *_predictor);
	virtual ~IoSimulator();

	//Main operations
	virtual void startIo(VexThreadState *state);
	virtual void endIo(VexThreadState *state, const long long &actualIoDuration);

	virtual bool areInIoThreadsInRunnablesQueue();

	virtual void printIoStats(const char *outputDirectory);
	virtual void printLocalLogging(const char *outputDir, const char *filename);
	virtual void exportMeasurements(const char *outputDirFileName);

protected:
	IoProtocol *protocol;
	IoPredictor *predictor;
};



/****
 * This class estimates the duration of an I/O operation by
 * simulating a queueing network describing the subsystem for
 * this I/O operation
 */
class QueueingNetworkIoSimulator : public IoSimulator {
public:
	QueueingNetworkIoSimulator(const char *filename);
	QueueingNetworkIoSimulator(const char *filename, const char *sourceNodeLabel, const int &customerClass);
	QueueingNetworkIoSimulator(const char *filename, IoLoggingBehaviour *_globalLoggingBehaviour);

	//Main operations
	virtual void startIo(VexThreadState *state);
	virtual void endIo(VexThreadState *state, const long long &actualIoDuration);

	virtual bool areInIoThreadsInRunnablesQueue();

	virtual void printIoStats(const char *outputDirectory);
	virtual void printLocalLogging(const char *outputDir, const char *filename);
	virtual void exportMeasurements(const char *outputDirFileName);

protected:
	MethodPerformanceModel *queueingNetworkDescribingIo;
	MethodPerformanceModelMetadata *queueingNetworkDescribingIoMetadata;

	IoLoggingBehaviour *ioQNSimulationLoggingBehaviour;
	int threadsInIo;
};


/***
 * This class decorates other IoSimulators offering a different
 * simulation possibilities based on the category of the simulator
 * The user can determine the different categories
 */
class CategoryIoSimulator : public IoSimulator {
public:
	CategoryIoSimulator() : IoSimulator() {};

	virtual void startIo(VexThreadState *state);
	virtual void endIo(VexThreadState *state, const long long &actualIoDuration);

	virtual bool areInIoThreadsInRunnablesQueue() = 0;

protected:
	virtual IoSimulator *getCategorySimulator(VexThreadState *state) = 0;

};


/***
 * This class decorates other IoSimulators offering a different
 * simulation possibilities based on the category of the simulator
 * The standard category simulator distinguishes the calls
 * that need to be simulated in real time in:
 * - system calls
 * - streaming I/O
 * - block I/O
 */
class StandardCategoryIoSimulator : public CategoryIoSimulator {
public:
	StandardCategoryIoSimulator(IoSimulator *_blockIoSimulator, IoSimulator *_charIoSimulator, IoSimulator *_systemCallsSimulator) : CategoryIoSimulator() {
		blockIoSimulator     = _blockIoSimulator;
		charIoSimulator		 = _charIoSimulator;
		systemCallsSimulator = _systemCallsSimulator;
	}

	StandardCategoryIoSimulator(IoSimulator *_blockIoSimulator, IoSimulator *_charIoSimulator, IoSimulator *_systemCallsSimulator, IoLoggingBehaviour *_globalLogging, const bool &_isLocalLoggingEnabled) : CategoryIoSimulator() {
		blockIoSimulator     	= _blockIoSimulator;
		charIoSimulator		 	= _charIoSimulator;
		systemCallsSimulator 	= _systemCallsSimulator;
		globalLogging 			= _globalLogging;
		isLocalLoggingEnabled 	= _isLocalLoggingEnabled;
	}

	virtual bool areInIoThreadsInRunnablesQueue();
	virtual void printIoStats(const char *outputDirectory);
	virtual void printLocalLogging(const char *outputDir, const char *filename);
	virtual void exportMeasurements(const char *outputDirFileName);


protected:
	virtual IoSimulator *getCategorySimulator(VexThreadState *state);

private:
	IoSimulator *blockIoSimulator;
	IoSimulator *charIoSimulator;
	IoSimulator *systemCallsSimulator;
	IoLoggingBehaviour *globalLogging;
	bool isLocalLoggingEnabled;
};


class IoSimulatorFactory {
public:
	static IoSimulator *create(IoParameters *parameters);
};

#endif /*IOSIMULATOR_H_*/

