/*
 * PredictionMethod.h: Define how the prediction will be generated
 *
 *  Created on: 16 Aug 2011
 *      Author: root
 */

#ifndef PREDICTIONMETHOD_H_
#define PREDICTIONMETHOD_H_

#include "ThreadState.h"
#include "IoLogger.h"

#ifdef USING_GSL
#include <gsl/gsl_histogram.h>
#else
class gsl_histogram_pdf;
#endif

#include "LinearRegression.h"
#include "AutoRegression.h"

class PredictionMethod {
public:
	PredictionMethod();
	virtual ~PredictionMethod();
	virtual long long getNext() = 0;
	virtual void addPredictionInfo(const long long &realTimeValueOnExit, VexThreadState *state, const unsigned int &threadsInIo) = 0;

	void setLoggingBehaviour(IoLoggingBehaviour *_predictionLog) {
		predictionLog = _predictionLog;
	}

	IoLoggingBehaviour *getLog() {
		return predictionLog;
	}

	virtual void import(char *inputDirFilePrefix, IoState &ioState);
	virtual void importFromFile(const std::string &filename) = 0;

	virtual void exportMeasurements(const char *outputDirFilePrefix, IoState &ioState);
	virtual void exportToFile(const std::string &filename) = 0;
protected:
	IoLoggingBehaviour *predictionLog;
};



class CyclicBufferBased : public PredictionMethod {
public:
	CyclicBufferBased(const unsigned int &_circular_buffersize);
	virtual ~CyclicBufferBased();
	long long getNext();
	virtual long long getValue() = 0;
	virtual void addPredictionInfo(const long long &realTimeValueOnExit, VexThreadState *state, const unsigned int &threadsInIo);

	virtual void importFromFile(const std::string &filename);

	virtual void exportToFile(const std::string &filename);

	void progressIndex() {
		index = index % circular_buffersize;
		++index;
	}

	int getCurrentSize() {
		return (size > circular_buffersize)?circular_buffersize:size;
	}
protected:
	long long *buffer;
	unsigned int size;
	unsigned int index;
	unsigned int circular_buffersize;
};


class ReplayCyclicBuffer: public CyclicBufferBased {
public:
	ReplayCyclicBuffer(const unsigned int &_circular_buffersize) : CyclicBufferBased(_circular_buffersize) {};
	virtual ~ReplayCyclicBuffer();
	long long getValue() {
		index = index % circular_buffersize;
		return buffer[index++];
	}
};

class AverageCyclicBuffer: public CyclicBufferBased {
public:
	AverageCyclicBuffer(const unsigned int &_circular_buffersize) : CyclicBufferBased(_circular_buffersize) {};
	virtual ~AverageCyclicBuffer();

protected:
	long long getAverage();
	long long getValue();
};


class MedianCyclicBuffer: public CyclicBufferBased {
public:
	MedianCyclicBuffer(const unsigned int &_circular_buffersize) : CyclicBufferBased(_circular_buffersize) {
		tempSortingBuffer = new long long[_circular_buffersize];
	};
	virtual ~MedianCyclicBuffer();
	long long getValue();
private:
	void sort(long long int *data, int N);
	long long *tempSortingBuffer;	// we need to use a separate buffer to sort and return the median to maintain past measurements window approach consistent

};

class SamplingCyclicBuffer: public AverageCyclicBuffer {
public:
	SamplingCyclicBuffer(const unsigned int &_circular_buffersize) : AverageCyclicBuffer(_circular_buffersize) {};
	virtual ~SamplingCyclicBuffer();
	long long getValue();

protected:
	long long getStdev(const long long &avg);
	double generageNormallyDistributedSamples(double mu=0.0, double sigma=1.0);
};

/***
 * Returns the minimum from the last circular_buffersize measurements
 */
class MinimumCyclicBuffer: public CyclicBufferBased {
public:
	MinimumCyclicBuffer(const unsigned int &_circular_buffersize) : CyclicBufferBased(_circular_buffersize) {};
	virtual ~MinimumCyclicBuffer();
	long long getValue();
};


/***
 * Returns the maximum from the last circular_buffersize measurements
 */
class MaximumCyclicBuffer: public CyclicBufferBased {
public:
	MaximumCyclicBuffer(const unsigned int &_circular_buffersize) : CyclicBufferBased(_circular_buffersize) {};
	virtual ~MaximumCyclicBuffer();
	long long getValue();
};

/***
 * Generates a Markov Modulated Process
 */
class MarkovModulatedProcess: public CyclicBufferBased {
public:
	MarkovModulatedProcess(const unsigned int &_circular_buffersize) : CyclicBufferBased(_circular_buffersize) {
		alloc(circular_buffersize, 2);
	};
	MarkovModulatedProcess(const unsigned int &_circular_buffersize, const unsigned int &_states) : CyclicBufferBased(_circular_buffersize) {
		alloc(circular_buffersize, _states);
	};

	void addPredictionInfo(const long long &realTimeValueOnExit, VexThreadState *state, const unsigned int &threadsInIo);
	virtual ~MarkovModulatedProcess();
	long long getValue();

protected:
	void alloc(const unsigned int &_circular_buffersize, unsigned int _states);
	void init();

	void distributeAllInBucket(int bucketId, const int &newBucketsToCreate, const int &lowestBucketId);
	void addValueToStateValueLimits(const int &state);

	void populateTransitionMatrix();
	void printTransitionMatrix();

	const long long &sampleMeasurementOfState(const int &currentState);
	int getRandomTransitionFrom(const int &_currentState);
	void setStateLimits();

	unsigned int states;
	int currentState;

	double **transitionMatrix;
	int *bucketOfElement;
	unsigned int *bucketSizes;


};

/***
 * Performs a simple linear regression from X=(1:circular_buffersize) to Y=(measured values vector) and
 * returns prediction for circular_buffersize+1
 */
class SimpleLinearRegressionCyclicBuffer: public CyclicBufferBased {
public:
	SimpleLinearRegressionCyclicBuffer(const unsigned int &_circular_buffersize) : CyclicBufferBased(_circular_buffersize) {regression = new LinearRegression(_circular_buffersize);};
	virtual ~SimpleLinearRegressionCyclicBuffer();
	long long getValue();

protected:
	LinearRegression *regression;
};


/***
 * Performs autoregression on the full buffer and returns the new prediction
 */
class AutoRegressionCyclicBuffer: public CyclicBufferBased {
public:
	AutoRegressionCyclicBuffer(const unsigned int &_circular_buffersize) : CyclicBufferBased(_circular_buffersize) {};
	long long getValue();
protected:
	AutoRegression *ar;
};

class BurgAutoRegressionCyclicBuffer: public AutoRegressionCyclicBuffer {
public:
	BurgAutoRegressionCyclicBuffer(const unsigned int &_circular_buffersize) : AutoRegressionCyclicBuffer(_circular_buffersize) {ar = new BurgAutoRegression(_circular_buffersize);};
};

class YulesWalkerAutoRegressionCyclicBuffer: public AutoRegressionCyclicBuffer {
public:
	YulesWalkerAutoRegressionCyclicBuffer(const unsigned int &_circular_buffersize) : AutoRegressionCyclicBuffer(_circular_buffersize) {ar = new YulesWalkerAutoRegression(_circular_buffersize);};
};


class GslHistogramPredictor : public PredictionMethod {
public:
	GslHistogramPredictor();
	long long getNext();
	void addPredictionInfo(const long long &realTimeValueOnExit, VexThreadState *state, const unsigned int &threadsInIo);

	void importFromFile(const std::string &filename);
	void exportToFile(const std::string &filename);

private:
	gsl_histogram_pdf *histogram_pdf;
};


class ConstantMeanPredictor : public PredictionMethod {
public:
	ConstantMeanPredictor() : total(0), size(0) {};
	long long getNext();
	void addPredictionInfo(const long long &realTimeValueOnExit, VexThreadState *state, const unsigned int &threadsInIo);

	void importFromFile(const std::string &filename);
	void exportToFile(const std::string &filename);

private:
	long long total;
	unsigned int size;
};


class NoPredictor : public PredictionMethod {
public:
	NoPredictor() {};
	long long getNext() {
		return 0;
	}
	void addPredictionInfo(const long long &realTimeValueOnExit, VexThreadState *state, const unsigned int &threadsInIo) {

	}

	void importFromFile(const std::string &filename) {

	}
	void exportToFile(const std::string &filename) {

	}
};

#endif /* PREDICTIONMETHOD_H_ */
