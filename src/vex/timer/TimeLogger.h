/*
 * TimeLogger.h
 *
 *  Created on: 3 Sep 2011
 *      Author: root
 */

#ifndef TIMELOGGER_H_
#define TIMELOGGER_H_

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>

class VexThreadState;
class VirtualTimeSnapshot;
struct vex_and_system_states;

class TimeLogger {
public:
	TimeLogger();
	virtual void logCpuTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	virtual void logIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	virtual void logNativeWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	virtual void logWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	virtual void logModelSimulationTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	virtual void logTimedWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	virtual void logTimedOutIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	virtual void logBackgroundLoadExecutionTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	virtual ~TimeLogger();

	virtual void printStats(std::ostream &outs);
	virtual void printExtraStats(const char *outputDir);

	virtual const char *getTitle() = 0;
	virtual const char *getColumnTitles();

protected:
	long long printScaled(const long long &value);
	double printPercent(const long long &value, const long long &total);

};

/*
class DurationMeanStdev : public TimeLogger {
public:
	virtual void logOnIoStart(const long long &prediction, VexThreadState *state, const unsigned int &threadsInIo);
	virtual void logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo);

	virtual void printStats(std::ostream &outs);
	virtual const char *getTitle() = 0;
	virtual const char *getColumnTitles();
protected:
	QStats<long long> durations;
};



class AllSamplesLogging : public TimeLogger {
protected:
	vector<long long> samples;

};

class AllActualIoSamples : public AllSamplesLogging {
	void logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo);
	void printStats(std::ostream &outs);
	const char *getTitle() {
		return "All real time samples";
	}
};
*/

class TotalTime : public TimeLogger {
public:
	TotalTime();
	void logCpuTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logNativeWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logModelSimulationTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logTimedWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logTimedOutIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logBackgroundLoadExecutionTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);

	void printStats(std::ostream &outs);
	const char* getTitle() {
		return "Total time [ms]";
	}
	const char *getColumnTitles();

protected:
	void increaseTimeCounter(long long &counter, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	long long cputime, iotime, nwtime, waitingtime, timedwaitingtime, timedoutiotime, modelsimtime, backgroundloadtime;
};


class ThreadTimeInfo {
public:
	ThreadTimeInfo(VexThreadState *state);

	void setLatestUpdatingTime(const long long &time) {
		lastERT = time;
	}

	long long getERTfromBeginningToEnd() {
		return lastERT - startERT;
	}

	char *getName() {
		return threadName;
	}
protected:
	int threadId;
	char *threadName;
	long long startERT;
	long long lastERT;
};

class PerThreadTime : public TotalTime {
public:
	PerThreadTime() : TotalTime() {
		zeroTime = 0;
	};
	void logCpuTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logNativeWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logModelSimulationTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logTimedWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logTimedOutIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logBackgroundLoadExecutionTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);

	void printStats(std::ostream &outs);
	const char* getTitle() {
		return "Per-thread time [ms]";
	}
	const char *getColumnTitles();

private:
	long long &getTimelineOfThread(std::map<int, long long> &threadToTimesMap, VexThreadState *state, ThreadTimeInfo *&threadInfo);
	long long &getValue(std::map<int, long long> &threadToTimesMap, const int &threadId);

	void  log(long long &logTimeline, std::map<int, long long> &allThreadLogs, VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);

	long long zeroTime;
	std::map<int, ThreadTimeInfo *> threadMapping;
	std::map<int, long long> cputimes, iotimes, nwtimes, waitingtimes, timedwaitingtimes, timedoutiotimes, modelsimtimes, backgroundloadtimes;
};





struct CombinedTimeSamplesAnalysis {
	CombinedTimeSamplesAnalysis() : lastNativeWaitingStartTime(-1), lastNativeWaitingTime(-1), lastGvtAfter(0) {}
	void resetNativeWaitingCounters() {
		lastNativeWaitingStartTime 	= -1;
		lastNativeWaitingTime 		= -1;
	}
	long long lastNativeWaitingStartTime;
	long long lastNativeWaitingTime;	// used for analysis
	long long lastGvtAfter;
};


class ThreadTimeSample {
public:
	ThreadTimeSample(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);

	void setName(char *_threadName) {
		threadName = _threadName;
	}

	bool compareTo(ThreadTimeSample *second);

	virtual const char *getSampleType() const = 0;
	virtual const char *checkValidity() const = 0;
	virtual const char *checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const = 0;
	friend std::ostream & operator <<(std::ostream &outs, const ThreadTimeSample &sample) {
		outs << "," << sample.getSampleType() << "," << sample.resumedLastAt/1000000 << "," << sample.localTimeSinceLastResume/1000000 << "," << sample.estimatedRealTime/1000000 << ",," << sample.gvtBefore/1000000 << "," << sample.gvtAfter/1000000 << "," << sample.checkValidity();
		//outs << "," << sample.getSampleType() << "," << sample.resumedLastAt << "," << sample.localTimeSinceLastResume << "," << sample.estimatedRealTime << ",," << sample.gvtBefore << "," << sample.gvtAfter << "," << sample.checkValidity();

		return outs;
	};

	void printSortedByGvt(std::ostream &outs, CombinedTimeSamplesAnalysis &analysisTimes);

protected:
	long long resumedLastAt;
	long long localTimeSinceLastResume;
	long long estimatedRealTime;
	long long gvtBefore;
	long long gvtAfter;
	char *threadName;
};

class CpuTimeSample : public ThreadTimeSample {
public:
	CpuTimeSample(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) : ThreadTimeSample(state, before, after) {};
	const char *getSampleType() const {
		return "CPU";
	}
	const char *checkValidity() const;
	const char *checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const;
};
class IoTimeSample : public ThreadTimeSample {
public:
	IoTimeSample(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) : ThreadTimeSample(state, before, after) {};
	const char *getSampleType() const {
		return "IO";
	}
	const char *checkValidity() const {
		return "OK";
	}
	const char *checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const;
};
class NativeWaitingTimeSample : public ThreadTimeSample {
public:
	NativeWaitingTimeSample(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) : ThreadTimeSample(state, before, after) {};
	const char *getSampleType() const {
		return "NW";
	}
	const char *checkValidity() const {
		return "OK";
	}
	const char *checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const;
};
class ModelSimulatingTimeSample : public ThreadTimeSample {
public:
	ModelSimulatingTimeSample(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) : ThreadTimeSample(state, before, after) {};
	const char *getSampleType() const {
		return "MS";
	}
	const char *checkValidity() const {
		return "OK";
	}
	const char *checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const;
};
class WaitingTimeSample : public ThreadTimeSample {
public:
	WaitingTimeSample(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) : ThreadTimeSample(state, before, after) {};
	const char *getSampleType() const {
		return "WTS";
	}
	const char *checkValidity() const {
		return "OK";
	}
	const char *checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const;
};
class TimedWaitingTimeSample : public ThreadTimeSample {
public:
	TimedWaitingTimeSample(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) : ThreadTimeSample(state, before, after) {};
	const char *getSampleType() const {
		return "TWTS";
	}
	const char *checkValidity() const {
		return "OK";
	}
	const char *checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const;
};
class TimedOutIoTimeSample : public ThreadTimeSample {
public:
	TimedOutIoTimeSample(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) : ThreadTimeSample(state, before, after) {};
	const char *getSampleType() const {
		return "IOTO";
	}
	const char *checkValidity() const {
		return "OK";
	}
	const char *checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const;
};
class BackgroundLoadExecutionTimeSample : public ThreadTimeSample {
public:
	BackgroundLoadExecutionTimeSample(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) : ThreadTimeSample(NULL, before, after) {};
	const char *getSampleType() const {
		return "BGND";
	}
	const char *checkValidity() const {
		return "OK";
	}
	const char *checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const;
};

class ThreadTimeSamples {
public:

	ThreadTimeSamples(VexThreadState *_state);
	void log(ThreadTimeSample *sample);

//	void logCpuTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
//	void logIoTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
//	void logNativeWaitingTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
//	void logModelSimulationTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
//	void logWaitingTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
//	void logTimedWaitingTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
//	void logBackgroundLoadExecutionTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);

	void addAllSamples(std::vector<ThreadTimeSample *> *allSamples);
	void print(std::ostream &outs);
protected:

	void addSampleTo(std::vector<ThreadTimeSample *> &localTimeLog, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	VexThreadState *state;
	char *threadName;
	std::vector<ThreadTimeSample *> times;

};

class SamplesPerThreadAnalysis : public TimeLogger {
public:
	SamplesPerThreadAnalysis() : TimeLogger() {};
	void logCpuTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logNativeWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logModelSimulationTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logTimedWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logTimedOutIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logBackgroundLoadExecutionTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);

	void printStats(std::ostream &outs);
	const char* getTitle() {
		return "Samples per thread time analysis [ms]";
	}
	const char *getColumnTitles();

private:
	void perThreadAnalysis(std::ostream &outs);
	void combinedAnalysis(std::ostream &outs);
	ThreadTimeSamples *getTimelinesOfThread(VexThreadState *state);
	long long printScaled(const long long &value);

	std::map<int, ThreadTimeSamples *> threadsToSamples;
};



/*
 * This class allows various level loggers to be grouped together in vectors.
 * All loggers of the correct event are updated when a related event occurs (Observer pattern)
 */
class TimeLoggingBehaviour {
public:
	TimeLoggingBehaviour();

	static void enableExtendedStats();
	static void enableGslStats();

	void addGlobalLoggers(TimeLogger *log);
	void addLocalTimeLoggers();

	void addOptionalLoggers();
	void addExtendedTimeLoggers();

	void logCpuTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logNativeWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logModelSimulationTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logTimedWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logTimedOutIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);
	void logBackgroundLoadExecutionTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after);

	const char *getTitlesInCSVRow();
	const char *getValuesInCSVRow();

	void getTitlesInCSVRow(std::ostream &outs);
	void getValuesInCSVRow(std::ostream &outs);

	~TimeLoggingBehaviour();
	void print(const char *filename);
	void printExtraStats(const char *outputDir);

protected:
	std::vector<TimeLogger *> cputimelogs;
	std::vector<TimeLogger *> iotimelogs;
	std::vector<TimeLogger *> nativewaitingtimelogs;
	std::vector<TimeLogger *> waitingtimelogs;
	std::vector<TimeLogger *> modelsimulationtimelogs;
	std::vector<TimeLogger *> timedoutiotimelogs;
	std::vector<TimeLogger *> timedwaitingtimelogs;
	std::vector<TimeLogger *> backgroundloadexecutiontimelogs;

	std::vector<std::vector<TimeLogger *> * > allLogs;

	static bool usingExtendedStats;
	static bool usingGslStats;
};



#endif /* TIMELOGGER_H_ */
