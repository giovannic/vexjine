/*
 * IoLogger.cpp
 *
 *  Created on: 16 Aug 2011
 *      Author: root
 */

#include "IoLogger.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include "PredictionMethod.h"

#ifdef USING_GSL
#include <gsl/gsl_histogram.h>
#else
struct GslNotUsedException : std::exception {
	const char *what() {return "GSL library not used in this installation of VEX.";}
};
#endif



IoLogger::IoLogger() {

}
void IoLogger::logOnIoStart(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {

}
void IoLogger::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {

}
void IoLogger::getColumnTitles(std::ostream &outs) {

}

void IoLogger::printStats(std::ostream &outs) {

}

void IoLogger::printExtraStats(const char *outputDir, IoState &ioState) {

}


IoLogger::~IoLogger() {

}


void DurationMeanStdev::logOnIoStart(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {

}
void DurationMeanStdev::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {

}

void DurationMeanStdev::printStats(std::ostream &outs) {
	outs << durations.getSamples() << "," << durations.getMean()/1e9 << "," << durations.getStdev()/1e9;
}


void DurationMeanStdev::getColumnTitles(std::ostream &outs) {
	outs << getTitle() << "_samples," << getTitle() << "_mean," << getTitle() << "_stdev";
}

bool DurationMeanStdev::hasLoggedSamples() {
	return (durations.getSamples() > 0);
}




void ParallelismStats::logOnIoStart(const long long &prediction, VexThreadState *state, const unsigned int &threadsInIo) {
	if (!state->isIgnoringIo() && !state->callRecognizedAsCached()) {
		threadsInParallelIo.addSample(threadsInIo);
	}
}
void PredictionIoDuration::logOnIoStart(const long long &prediction, VexThreadState *state, const unsigned int &threadsInIo) {
	if (prediction > 0) {
		durations.addSample(prediction);
	}
}

void ActuallyUsedPredictionIoDuration::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {
	if (!state->isIgnoringIo()) {
		durations.addSample(state->getLastIoPrediction());
	}
}

void UnusedPredictionIoDuration::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {
	if (state->isIgnoringIo()) {
		durations.addSample(state->getLastIoPrediction());
	}
}


void RealToCpuTimeDifference::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {
	long long difference = abs((long double)(actualTime - state->getIoCPUTime()));
	// quite inexact - TODO: FIX: no acceleration factors
	durations.addSample(difference);

}

void ActualIoRealToCpuTimeDifference::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {
	long long difference = abs((long double)(actualTime - state->getIoCPUTime()));
	// quite inexact - TODO: FIX: no acceleration factors
	if (((float)difference/(float)actualTime) > 0.10) {
		durations.addSample(difference);
	}

}


void ActualIoDuration::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {
	durations.addSample(actualTime);
}


void MisPredictionIoDuration::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {
	long long prediction = state->getLastIoPrediction();
	if (prediction > 0 && !state->isIgnoringIo()) {
		long long misprediction = abs((long double)(actualTime - prediction));
		durations.addSample(misprediction);
	}
}

void MSE::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {
	long long prediction = state->getLastIoPrediction();
	if (prediction > 0 && !state->isIgnoringIo()) {
		long long misprediction = (actualTime - prediction);
		misprediction *= misprediction;
		durations.addSample(misprediction);
	}
}

void MSE::printStats(std::ostream &outs) {
	outs << durations.getSamples() << "," << durations.getMean()/1e18 << "," << durations.getStdev()/1e18;
}

void ParallelismStats::getColumnTitles(std::ostream &outs) {
	outs << getTitle() << "_samples," << getTitle() << "_mean," << getTitle() << "_stdev";
}
void ParallelismStats::printStats(std::ostream &outs) {
	outs << threadsInParallelIo;
}

bool ParallelismStats::hasLoggedSamples() {
	return threadsInParallelIo.getSamples() > 0;
}

OverUnderPrediction::OverUnderPrediction() {
	underpredictions = 0;
	overpredictions = 0;
	correctpredictions = 0;
	limit = 0.2;	// default 20% limit
}

void OverUnderPrediction::getColumnTitles(std::ostream &outs) {
	outs << "Under-,Correct-,Overpredictions";
}
void OverUnderPrediction::printStats(std::ostream &outs) {
	outs << underpredictions << "," << correctpredictions << "," << overpredictions;
}
bool OverUnderPrediction::hasLoggedSamples() {
	return ((underpredictions + correctpredictions + overpredictions) > 0);
}


void OverUnderPrediction::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {
	double error = (double)(state->getLastIoPrediction() - actualTime) / actualTime;
	if (error > limit) {
		++overpredictions;
	} else if (error < (-limit)) {
		++underpredictions;
	} else {
		++correctpredictions;
	}

}



IoMethodCallInfo *IoMethodCallStats::getIoMethodCallInfo(const int &ioMethodId) {
	map<int, IoMethodCallInfo *>::iterator mit = resultsPerIoMethodCall.find(ioMethodId);
	IoMethodCallInfo *methodInfo;
	if (mit != resultsPerIoMethodCall.end()) {
		methodInfo = mit->second;
	} else {
		methodInfo = new IoMethodCallInfo;
		resultsPerIoMethodCall[ioMethodId] = methodInfo;
	}
	return methodInfo;

}

void IoMethodCallStats::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {
	IoMethodCallInfo *methodInfo = getIoMethodCallInfo(state->getCurrentMethodId());
	methodInfo->predictedDurations.addSample(state->getLastIoPrediction());
	long long misprediction = actualTime - state->getLastIoPrediction();
	methodInfo->misPredictionsDurations.addSample(misprediction);
	methodInfo->actualDurations.addSample(actualTime);
}


bool IoMethodCallStats::hasLoggedSamples() {
	map<int, IoMethodCallInfo *>::iterator mit = resultsPerIoMethodCall.begin();
	while (mit != resultsPerIoMethodCall.end()) {
		IoMethodCallInfo *iom = mit->second;
		if (iom->actualDurations.getSamples() > 0) {
			return true;
		}
		++mit;
	}
	return false;
}

//TODO: IMPROVEMENT: read from file
GslHistogram::GslHistogram() {
#ifdef USING_GSL
    double range[75];
    double element = 1000;
    range[0] = 0;
    size_t count = 1;
    for (int i=1; i<10; i++) {
            range[count++] = i * element;
    }
    element = 10000;
    for (int i=1; i<10; i++) {
            range[count++] = i * element;
    }
    element = 50000;
    for (int i=2; i<20; i++) {
            range[count++] = i * element;
    }
    element = 200000;
    for (int i=5; i<25; i++) {
            range[count++] = i * element;
    }
    element = 500000;
    for (int i=10; i<20; i++) {
            range[count++] = i * element;
    }
    element = 1000000000;
    for (int i=1; i<5; i++) {
            range[count++] = i * element;
    }
    range[count++] = 10000000000;
    range[count++] = 20000000000;
    range[count++] = 50000000000;
    range[count++] = 100000000000;

	histogram = gsl_histogram_alloc(count-1);
    gsl_histogram_set_ranges(histogram, range, count); // gsl throws exceptions in failure
#else
    throw GslNotUsedException();
#endif
}

bool GslHistogram::hasLoggedSamples() {
	//TODO: fix me
	return false;
}


void ActualTimesGslHistogram::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {
#ifdef USING_GSL
	if (gsl_histogram_increment(histogram, actualTime) != 0) {
		cout << "Real I/O value " << actualTime << " was too large for the set histogram range" << endl;
	}
#else
    throw GslNotUsedException();
#endif
}

void ActualTimesGslHistogram::printExtraStats(const char *outputDir, IoState &ioState) {
#ifdef USING_GSL
	stringstream filename;
	filename << outputDir << "/vtf_gsl_io_time_histogram";
	ioState.getIoPointFileSuffix(filename);
	filename << ".csv";
	FILE *histo_file = fopen(filename.str().c_str(), "w");
	gsl_histogram_fwrite (histo_file, histogram);
	fclose(histo_file);
#else
	throw GslNotUsedException();
#endif

}


bool AllSamplesLogging::hasLoggedSamples() {
	return samples.size() > 0;
}

AllSamplesLogging::~AllSamplesLogging() {
//	pthread_mutex_init(&mutex, NULL);
//	spin_init(&spinlock, 0);
//	int spinint;
//	pthread_spinlock_t spinlock;
//	pthread_mutex_t mutex;
	pthread_mutex_destroy(&mutex);
}


void AllActualIoSamples::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {
	lock();
	samples.push_back(actualTime);
	unlock();
}

void AllActualIoSamples::printStats(std::ostream &outs) {
	outs << ",,Real_Samples";
	lock();
	vector<long long>::iterator vit = samples.begin();
	while (vit != samples.end()) {
		outs << "," << (*vit);
		++vit;
	}
	unlock();
}


void AllPredictedIoSamples::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {
	lock();
	samples.push_back(state->getLastIoPrediction());
	unlock();
}

void AllPredictedIoSamples::printStats(std::ostream &outs) {
	outs << ",,Predicted_Samples";
	lock();
	vector<long long>::iterator vit = samples.begin();
	while (vit != samples.end()) {
		outs << "," << (*vit);
		++vit;
	}
	unlock();
}



/**
 * Wrapper (Subject)
 */
bool IoLoggingBehaviour::usingExtendedStats = false;
bool IoLoggingBehaviour::usingGslStats = false;
IoLoggingBehaviour::IoLoggingBehaviour() {

}


void IoLoggingBehaviour::enableExtendedStats() {
	usingExtendedStats = true;
}
void IoLoggingBehaviour::enableGslStats() {
	usingGslStats = true;
}

void IoLoggingBehaviour::addOptionalLoggers() {
	if (IoLoggingBehaviour::usingExtendedStats) {
		addExtendedIoLoggers();
	}

#ifdef USING_GSL
	if (IoLoggingBehaviour::usingGslStats) {
		addPostIoLogger(new ActualTimesGslHistogram());
	}
#endif

}
void IoLoggingBehaviour::addGlobalIoLoggers() {
	addPreIoLogger(new ParallelismStats());
	addPreIoLogger(new PredictionIoDuration());
	addPostIoLogger(new ActualIoDuration());
	addPostIoLogger(new MisPredictionIoDuration());
	addPostIoLogger(new MSE());
	addPostIoLogger(new OverUnderPrediction());
	addPostIoLogger(new ActuallyUsedPredictionIoDuration());
	addPostIoLogger(new UnusedPredictionIoDuration());
	addPostIoLogger(new RealToCpuTimeDifference());
	addPostIoLogger(new ActualIoRealToCpuTimeDifference());

	addOptionalLoggers();

}

void IoLoggingBehaviour::addLocalIoLoggers() {
	addPreIoLogger(new PredictionIoDuration());
	addPostIoLogger(new ActualIoDuration());
	addPostIoLogger(new MisPredictionIoDuration());
	addPostIoLogger(new MSE());
	addOptionalLoggers();

}

void IoLoggingBehaviour::addExtendedIoLoggers() {
	addPostIoLogger(new AllPredictedIoSamples());
	addPostIoLogger(new AllActualIoSamples());
}


void IoLoggingBehaviour::logOnIoStart(const long long &prediction, VexThreadState *state, const unsigned int &threadsInIo) {
	vector<IoLogger *>::iterator vit = prelogs.begin();

	while (vit != prelogs.end()) {
		(*vit)->logOnIoStart(prediction, state, threadsInIo);
		++vit;
	}

}

void IoLoggingBehaviour::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {
	vector<IoLogger *>::iterator vit = postlogs.begin();

	while (vit != postlogs.end()) {
		(*vit)->logOnIoEnd(actualTime, state, threadsInIo);
		++vit;
	}

}


bool IoLoggingBehaviour::hasLoggedSamples() {
	vector<IoLogger *>::iterator vit = prelogs.begin();
	while (vit != prelogs.end()) {
		IoLogger *log = (*vit);
		if (log->hasLoggedSamples()) {
			return true;
		}
		++vit;
	}

	vit = postlogs.begin();
	while (vit != postlogs.end()) {
		IoLogger *log = (*vit);
		if (log->hasLoggedSamples()) {
			return true;
		}
		++vit;
	}
	return false;
}

void IoLoggingBehaviour::getTitlesInCSVRow(ostream &outs) {

	vector<IoLogger *>::iterator vit = prelogs.begin();
	while (vit != prelogs.end()) {
		IoLogger *log = (*vit);
		log->getColumnTitles(outs);
		outs << ",";
		++vit;
	}

	vit = postlogs.begin();
	while (vit != postlogs.end()) {
		IoLogger *log = (*vit);
		log->getColumnTitles(outs);
		outs << ",";
		++vit;
	}

}

void IoLoggingBehaviour::getValuesInCSVRow(ostream &outs) {


	vector<IoLogger *>::iterator vit = prelogs.begin();
	while (vit != prelogs.end()) {
		(*vit)->printStats(outs);
		outs << ",";
		++vit;
	}

	vit = postlogs.begin();
	while (vit != postlogs.end()) {
		(*vit)->printStats(outs);
		outs << ",";
		++vit;
	}

}

void IoLoggingBehaviour::printStatsIntoStream(std::ostream &os) {

	vector<IoLogger *>::iterator vit = prelogs.begin();
	while (vit != prelogs.end()) {
		IoLogger *log = (*vit);
		os << log->getTitle() << ":,";
		log->printStats(os);
		os << endl;
		++vit;
	}

	vit = postlogs.begin();
	while (vit != postlogs.end()) {
		IoLogger *log = (*vit);
		os << log->getTitle() << ":,";
		log->printStats(os);
		os << endl;
		++vit;
	}

}

/*
 * Print statistics gathered from the I/O duration prediction buffer
 */
void IoLoggingBehaviour::print(const char *filename, const char *predictionMethodTitle) {
	std::filebuf fb;
	fb.open(filename, std::ios::out);
	std::ostream os(&fb);

	os << predictionMethodTitle << endl;
	printStatsIntoStream(os);

	fb.close();
}


void IoLoggingBehaviour::printExtraStats(const char *outputDir) {
	IoState ioState;
	printExtraStats(outputDir, ioState);
}

void IoLoggingBehaviour::printExtraStats(const char *outputDir, IoState &ioState) {

	vector<IoLogger *>::iterator vit = prelogs.begin();
	while (vit != prelogs.end()) {
		IoLogger *log = (*vit);
		log->printExtraStats(outputDir, ioState);
		++vit;
	}
	vit = postlogs.begin();
	while (vit != postlogs.end()) {
		IoLogger *log = (*vit);
		log->printExtraStats(outputDir, ioState);
		++vit;
	}

}



IoLoggingBehaviour::~IoLoggingBehaviour() {

	vector<IoLogger *>::iterator vit = prelogs.begin();
	while (vit != prelogs.end()) {
		delete (*vit);
		++vit;
	}
	vit = postlogs.begin();
	while (vit != postlogs.end()) {
		delete (*vit);
		++vit;
	}
}
