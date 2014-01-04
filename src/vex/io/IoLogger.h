/*
 * IoLogger.h: Classes for logging information about the I/O simulation module of VEX
 *
 *  Created on: 16 Aug 2011
 *      Author: root
 */

#ifndef IOLOGGER_H_
#define IOLOGGER_H_

#include <vector>
#include <string>

#include "ThreadState.h"
#include "QStats.h"

#include <fstream>
#include <iostream>

#ifdef USING_GSL
#include <gsl/gsl_histogram.h>
#else
class gsl_histogram;
#endif

#include "IoState.h"

/*
 * Abstract class for any type of I/O simulation logger: offers a common interface to all different
 * loggers by providing the thread states, predictions and actual times upon start and end of the simulation.
 * Each logger is allowed to log whichever information it wants
 */
class IoLogger {
public:
	IoLogger();
	virtual void logOnIoStart(const long long &prediction, VexThreadState *state, const unsigned int &threadsInIo);
	virtual void logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo);
	virtual ~IoLogger();

	virtual void printStats(std::ostream &outs);
	virtual void printExtraStats(const char *outputDir, IoState &ioState);

	virtual const char *getTitle() = 0;
	virtual void getColumnTitles(std::ostream &outs);
	virtual bool hasLoggedSamples() = 0; 	// returns true if there is at least one sample logged

};

/*
 * Class for loggers that return mean and stdev for all I/O simulation samples
 */
class DurationMeanStdev : public IoLogger {
public:
	virtual void logOnIoStart(const long long &prediction, VexThreadState *state, const unsigned int &threadsInIo);
	virtual void logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo);

	virtual void printStats(std::ostream &outs);
	virtual const char *getTitle() = 0;
	virtual void getColumnTitles(std::ostream &outs);
	virtual bool hasLoggedSamples(); 	// returns true if there is at least one sample logged

protected:
	QStats<long long> durations;
};


/*
 * Class to log the mean and stdev of the actual I/O duration of all samples
 */
class ActualIoDuration : public DurationMeanStdev {
public:
	void logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo);
	const char *getTitle() {
		return "Measured I/O durations";
	}
};

/*
 * Class to log the mean and stdev of the I/O prediction for all samples
 */
class PredictionIoDuration : public DurationMeanStdev {
public:
	void logOnIoStart(const long long &prediction, VexThreadState *state, const unsigned int &threadsInIo);
	const char *getTitle() {
		return "Predicted I/O durations";
	}
};

/*
 * Class to log the mean and stdev of the I/O predictions actually used
 */
class ActuallyUsedPredictionIoDuration : public DurationMeanStdev {
public:
	void logOnIoEnd(const long long &prediction, VexThreadState *state, const unsigned int &threadsInIo);
	const char *getTitle() {
		return "Used predicted I/O durations";
	}
};

/*
 * Class to log the mean and stdev of the I/O predictions not used
 */
class UnusedPredictionIoDuration : public DurationMeanStdev {
public:
	void logOnIoEnd(const long long &prediction, VexThreadState *state, const unsigned int &threadsInIo);
	const char *getTitle() {
		return "Unused predicted I/O durations";
	}
};

/*
 * Class to log the mean and stdev of the differences between real and CPU times of all I/O measurements
 */
class RealToCpuTimeDifference : public DurationMeanStdev {
public:
	void logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo);
	const char *getTitle() {
		return "I/O Real - CPU time difference";
	}
};

/*
 * Class to log the mean and stdev of all relative differences between real and CPU times
 * higher than 10% for all I/O measurements
 */
class ActualIoRealToCpuTimeDifference : public DurationMeanStdev {
public:
	void logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo);
	const char *getTitle() {
		return "Actual I/O Real - CPU time difference";
	}
};


/*
 * Class to log the mean and stdev of all I/O mispredictions
 */
class MisPredictionIoDuration : public DurationMeanStdev {
public:
	void logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo);
	const char *getTitle() {
		return "I/O prediction differences";
	}
};

/*
 * Class to log the mean and stdev of the mean squared error for all I/O samples
 */
class MSE : public DurationMeanStdev {
public:
	void logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo);
	const char *getTitle() {
		return "MSE";
	}
	virtual void printStats(std::ostream &outs);
};

/*
 * Class to log the mean and stdev of the I/O parallelism exhibited between threads
 */
class ParallelismStats : public IoLogger {
public:
	void logOnIoStart(const long long &prediction, VexThreadState *state, const unsigned int &threadsInIo);
	const char *getTitle() {
		return "Threads performing I/O in parallel";
	}
	virtual void getColumnTitles(std::ostream &outs);
	virtual void printStats(std::ostream &outs);

	virtual bool hasLoggedSamples();
private:
	QStats<int> threadsInParallelIo;
};


/*
 * Class to log the over-, under- and correct predictions assuming a 20% limit
 */
class OverUnderPrediction : public IoLogger {
public:
	OverUnderPrediction();
	void logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo);
	const char *getTitle() {
		return "Under-/Correct-/Overpredictions (20% lim)";
	}
	virtual void getColumnTitles(std::ostream &outs);
	virtual void printStats(std::ostream &outs);

	virtual bool hasLoggedSamples();
private:
	unsigned int overpredictions;
	unsigned int correctpredictions;	// within limit %
	unsigned int underpredictions;
	double limit;

};


/*
 * Class for logging information in GSL histograms
 */
class GslHistogram : public IoLogger {
public:
	GslHistogram();

	virtual bool hasLoggedSamples();

protected:
	gsl_histogram *histogram;
};


/*
 * Class to log the GSL histogram of all real I/O durations
 */
class ActualTimesGslHistogram : public GslHistogram {
public:
	virtual void logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo);
	void printExtraStats(const char *outputDir, IoState &ioState);
	const char *getTitle() {
		return "GSL Histogram created";
	}
};

/*
 * Class to log all I/O samples in a vector
 */
class AllSamplesLogging : public IoLogger {
public:
	AllSamplesLogging() : IoLogger() {
		pthread_mutex_init(&mutex, NULL);
	}

	virtual bool hasLoggedSamples();
	virtual ~AllSamplesLogging();

protected:
	void lock() {
		pthread_mutex_lock(&mutex);
	}

	void unlock() {
		pthread_mutex_unlock(&mutex);
	}

	vector<long long> samples;
	pthread_mutex_t mutex;
};

/*
 * Class to log all real I/O measurement samples in a vector
 */
class AllActualIoSamples : public AllSamplesLogging {
	void logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo);
	void printStats(std::ostream &outs);
	const char *getTitle() {
		return "All real time samples";
	}
};

/*
 * Class to log all I/O prediction samples in a vector
 */
class AllPredictedIoSamples : public AllSamplesLogging {
	void logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo);
	void printStats(std::ostream &outs);
	const char *getTitle() {
		return "All predicted time samples";
	}
};



/*
 * Class to log samples by aggregating them according to the method id of the I/O operation they belong to
 */
struct IoMethodCallInfo {
	QStats<long long> actualDurations;
	QStats<long long> predictedDurations;
	QStats<long long> misPredictionsDurations;
};
class IoMethodCallStats : public IoLogger {
public:
	void logOnIoStart(const long long &prediction, VexThreadState *state, const unsigned int &threadsInIo);
	void logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo);

	virtual bool hasLoggedSamples();
private:
	IoMethodCallInfo *getIoMethodCallInfo(const int &methodId);
	map<int, IoMethodCallInfo *> resultsPerIoMethodCall;
};


/*
 * Class to define the instances of the above logger classes that will be used in the simulation
 */
class IoLoggingBehaviour {
public:
	IoLoggingBehaviour();

	static void enableExtendedStats();
	static void enableGslStats();

	void addGlobalIoLoggers();
	void addLocalIoLoggers();

	void addOptionalLoggers();

	void addExtendedIoLoggers();

	void addPreIoLogger(IoLogger *log) {
		prelogs.push_back(log);
	}

	void addPostIoLogger(IoLogger *log) {
		postlogs.push_back(log);
	}
	void logOnIoStart(const long long &prediction, VexThreadState *state, const unsigned int &threadsInIo);
	void logOnIoEnd(const long long &actualIoTime, VexThreadState *state, const unsigned int &threadsInIo);

	bool hasLoggedSamples();
	void getTitlesInCSVRow(ostream &outs);
	void getValuesInCSVRow(ostream &outs);
	void print(const char *filename, const char *predictionMethodTitle);
	void printStatsIntoStream(std::ostream &os);
	void printExtraStats(const char *outputDir);
	void printExtraStats(const char *outputDir, IoState &ioState);	// Extended statistics files that are not grouped together with other stats

	~IoLoggingBehaviour();

protected:
	std::vector<IoLogger *> prelogs;
	std::vector<IoLogger *> postlogs;

	static bool usingExtendedStats;
	static bool usingGslStats;
};


#endif /* IOLOGGER_H_ */
