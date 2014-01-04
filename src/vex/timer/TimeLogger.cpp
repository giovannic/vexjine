/*
 * TimeLogger.cpp
 *
 *  Created on: 16 Aug 2011
 *      Author: root
 */

#include "TimeLogger.h"
#include "VirtualTimeline.h"

//#include "QStats.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <map>
#include "PredictionMethod.h"
#include <algorithm>

TimeLogger::TimeLogger() {

}
void TimeLogger::logCpuTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {

}
void TimeLogger::logIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {

}
void TimeLogger::logNativeWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {

}
void TimeLogger::logWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {

}
void TimeLogger::logModelSimulationTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {

}
void TimeLogger::logTimedWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {

}
void TimeLogger::logTimedOutIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {

}

void TimeLogger::logBackgroundLoadExecutionTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {

}

ThreadTimeInfo::ThreadTimeInfo(VexThreadState *state) {
	if (state != NULL) {
		threadName = new char[strlen(state->getName())+1];
		strcpy(threadName, state->getName());
		startERT = state->getResumedLastAt();
	} else {
		threadName = new char[32];
		strcpy(threadName, "Background Load");
		startERT = 0;
	}
	lastERT = 0;
}

long long TimeLogger::printScaled(const long long &value) {
	return value / 1000000;
}
double TimeLogger::printPercent(const long long &value, const long long &total) {
	if (total == 0) {
		return 0.0;
	} else {
		return (double)value/(double)total;
	}
}

const char * TimeLogger::getColumnTitles() {
	return "";
}

void TimeLogger::printStats(std::ostream &outs) {

}

void TimeLogger::printExtraStats(const char *outputDir) {

}

TimeLogger::~TimeLogger() {

}

/*
void DurationMeanStdev::logOnIoStart(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {

}
void DurationMeanStdev::logOnIoEnd(const long long &actualTime, VexThreadState *state, const unsigned int &threadsInIo) {

}

void DurationMeanStdev::printStats(std::ostream &outs) {
	outs << durations;
}

const char *DurationMeanStdev::getColumnTitles() {
	stringstream str;
	str << getTitle() << "_samples," << getTitle() << "_mean," << getTitle() << "_stdev";
	return str.str().c_str();
}

*/

TotalTime::TotalTime() {
	cputime = 0;
	iotime = 0;
	nwtime = 0;
	waitingtime = 0;
	timedwaitingtime = 0;
	timedoutiotime = 0;
	modelsimtime = 0;
	backgroundloadtime = 0;

}

void TotalTime::increaseTimeCounter(long long &counter, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	counter += after->getGlobalTime()-before->getGlobalTime();
}
void TotalTime::logCpuTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	increaseTimeCounter(cputime, before, after);
}
void TotalTime::logIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	increaseTimeCounter(iotime, before, after);
}
void TotalTime::logNativeWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	increaseTimeCounter(nwtime, before, after);
}
void TotalTime::logWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	increaseTimeCounter(waitingtime, before, after);
}
void TotalTime::logModelSimulationTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	increaseTimeCounter(modelsimtime, before, after);
}
void TotalTime::logTimedWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	increaseTimeCounter(timedwaitingtime, before, after);
}
void TotalTime::logTimedOutIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	increaseTimeCounter(timedoutiotime, before, after);
}
void TotalTime::logBackgroundLoadExecutionTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	increaseTimeCounter(backgroundloadtime, before, after);
}

const char * TotalTime::getColumnTitles() {
	return "CPU, NW, IO, Waiting, ModelSim, TimedWait, TimedOutIo, BackgroundLoad";
}

void TotalTime::printStats(std::ostream &outs) {
	outs << printScaled(cputime) << "," << printScaled(nwtime) << "," << printScaled(iotime) << "," << printScaled(waitingtime) << "," << printScaled(modelsimtime) << "," << printScaled(timedwaitingtime) << "," << printScaled(timedoutiotime) << "," << printScaled(backgroundloadtime) << endl;
}



/***
 * Per thread and total times
 */

long long &PerThreadTime::getTimelineOfThread(std::map<int, long long> &threadToTimesMap, VexThreadState *state, ThreadTimeInfo *&threadInfo) {
	int threadId;
	if (state != NULL) {
		threadId = state->getId();
	} else {
		threadId = -1;	// used for global system events (background load)
	}
	std::map<int, long long>::iterator mit = threadToTimesMap.find(threadId);
	if (mit == threadToTimesMap.end()) {
		threadToTimesMap[threadId] = 0;
		threadMapping[threadId] = new ThreadTimeInfo(state);		// ThreadTimeInfo is aware that state might be NULL
	}
	threadInfo = threadMapping[threadId];
	return threadToTimesMap[threadId];
}

void PerThreadTime::log(long long &logTimeline, std::map<int, long long> &allThreadLogs, VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	increaseTimeCounter(logTimeline, before, after);
	ThreadTimeInfo *threadInfo = NULL;
	increaseTimeCounter(getTimelineOfThread(allThreadLogs, state, threadInfo), before, after);
	threadInfo->setLatestUpdatingTime(after->getGlobalTime());
}

void PerThreadTime::logCpuTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	log(cputime, cputimes, state, before, after);
}
void PerThreadTime::logIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	log(iotime, iotimes, state, before, after);
}
void PerThreadTime::logNativeWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	log(nwtime, nwtimes, state, before, after);
}
void PerThreadTime::logWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	log(waitingtime, waitingtimes, state, before, after);
}
void PerThreadTime::logModelSimulationTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	log(modelsimtime, modelsimtimes, state, before, after);
}
void PerThreadTime::logTimedWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	log(timedwaitingtime, timedwaitingtimes, state, before, after);
}
void PerThreadTime::logTimedOutIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	log(timedoutiotime, timedoutiotimes, state, before, after);
}
void PerThreadTime::logBackgroundLoadExecutionTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	log(backgroundloadtime, backgroundloadtimes, NULL, before, after);
}

const char * PerThreadTime::getColumnTitles() {
	return "ThreadName,Total_CPU_time,Total_ERT,CPU,NW,IO,Waiting,TimedWait,TimedOutIo,ModelSim,BackgroundLoad,%CPU,%NW,%IO,%Waiting,%ModelSim,%TimedWait,%TimedOutIo,%BackgroundLoad";
}

long long &PerThreadTime::getValue(std::map<int, long long> &threadToTimesMap, const int &threadId) {
	std::map<int, long long>::iterator mit = threadToTimesMap.find(threadId);
	if (mit != threadToTimesMap.end()) {
		return mit->second;
	} else {
		return zeroTime;
	}
}



void PerThreadTime::printStats(std::ostream &outs) {
	long long tcputime, tiotime, tnwtime, twaitingtime, ttimedwaitingtime, ttimedoutiotime, tmodelsimtime, tbackground;
	std::map<int, ThreadTimeInfo *>::iterator threadIt = threadMapping.begin();
	while (threadIt != threadMapping.end()) {
		int threadId = threadIt->first;
		ThreadTimeInfo *threadTimeInfo = threadIt->second;
		tcputime = getValue(cputimes, threadId);
		tnwtime  = getValue(nwtimes, threadId);
		tiotime  = getValue(iotimes, threadId);
		twaitingtime = getValue(waitingtimes, threadId);
		ttimedwaitingtime = getValue(timedwaitingtimes, threadId);
		ttimedoutiotime = getValue(timedoutiotimes, threadId);
		tmodelsimtime = getValue(modelsimtimes, threadId);
		tbackground = getValue(backgroundloadtimes, threadId);		// only the background load thread will have this
		long long threadTotal = tcputime + tnwtime +tiotime+twaitingtime+ttimedwaitingtime+ttimedoutiotime+tmodelsimtime;
		outs << threadTimeInfo->getName() << "," << printScaled(threadTotal) << "," << printScaled(threadTimeInfo->getERTfromBeginningToEnd()) << "," << printScaled(tcputime) << "," << printScaled(tnwtime) <<"," << printScaled(tiotime) << "," << printScaled(twaitingtime) <<","<< printScaled(ttimedwaitingtime) << ","<< printScaled(ttimedoutiotime) << "," << printScaled(tmodelsimtime) << "," << printScaled(tbackground);
		outs<< fixed << setprecision(2) << printPercent(tcputime, cputime) << "," << printPercent(tnwtime,nwtime) <<"," << printPercent(tiotime,iotime) << "," << printPercent(twaitingtime,waitingtime) <<","<<printPercent(ttimedwaitingtime,timedwaitingtime) << ","<<printPercent(ttimedoutiotime,timedoutiotime) << "," <<printPercent(tmodelsimtime,modelsimtime) << "," <<printPercent(tbackground, backgroundloadtime) << endl;
		outs.unsetf(ios_base::fixed);
		++threadIt;
	}
	outs << endl;
	outs << "Totals,";
	TotalTime::printStats(outs);
	outs << endl;

}






/***
 * Analytic samples per thread and checks in the end
 */
const char *CpuTimeSample::checkValidity() const {
	std::stringstream str;
	short errors = 0;
	short warnings = 0;
	if (localTimeSinceLastResume != 0 && (estimatedRealTime != gvtAfter)) {
		str << ",LOST TIME";
		++errors;
	} else {
		str << ",";
	}
	if (resumedLastAt + localTimeSinceLastResume != estimatedRealTime) {
		str << ",Discrepancy RLA+Local=ERT";
		++warnings;
	} else {
		str << ",";
	}
	if (resumedLastAt > gvtBefore) {
		str << ",Started after";
		++warnings;
	} else {
		str << ",";
	}
	if (estimatedRealTime != gvtAfter) {
		str << ",ERT!=GVT";
		++warnings;
	} else {
		str << ",";
	}
	if (errors+warnings == 0) {
		return "OK";
	} else {
		str << errors << "," << warnings;
		return str.str().c_str();
	}
}
void ThreadTimeSample::printSortedByGvt(ostream &outs, CombinedTimeSamplesAnalysis &analysisTimes) {
	outs << gvtBefore/1000000 << "," << gvtAfter/1000000 << "," << threadName << "," << getSampleType() << "," << resumedLastAt/1000000 << "," << localTimeSinceLastResume/1000000 << "," << estimatedRealTime/1000000 << "," << checkInterThreadValidity(analysisTimes) << endl;
}


const char *CpuTimeSample::checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const {
	std::stringstream str;
	short errors = 0;

	if (analysisTimes.lastNativeWaitingStartTime == resumedLastAt && (analysisTimes.lastNativeWaitingStartTime + analysisTimes.lastNativeWaitingTime + localTimeSinceLastResume < gvtAfter)) {
		str << ", LOST TIME FROM NW";
		++errors;
	} else {
		str << ",";
	}
	if (-1000000 > (gvtBefore - analysisTimes.lastGvtAfter) ||  (gvtBefore - analysisTimes.lastGvtAfter) > 1000000) {
		str << ", LEAP FROM ONE GVT TO THE NEXT";
		++errors;
	} else {
		str << ",";
	}
	analysisTimes.lastGvtAfter = gvtAfter;
	analysisTimes.resetNativeWaitingCounters();

	if (errors > 0) {
		return str.str().c_str();
	} else {
		return "OK";
	}
}
const char *IoTimeSample::checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const {
	std::stringstream str;
	short errors = 0;

	if (-1000000 > (gvtBefore - analysisTimes.lastGvtAfter) ||  (gvtBefore - analysisTimes.lastGvtAfter) > 1000000) {
		str << ", LEAP FROM ONE GVT TO THE NEXT";
		++errors;
	} else {
		str << ",";
	}
	analysisTimes.lastGvtAfter = gvtAfter;
	if (errors > 0) {
		return str.str().c_str();
	} else {
		return "OK";
	}

}
const char *NativeWaitingTimeSample::checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const {
	std::stringstream str;
	short errors = 0;

	if (analysisTimes.lastNativeWaitingStartTime == -1) {
		analysisTimes.lastNativeWaitingStartTime = resumedLastAt;
		analysisTimes.lastNativeWaitingTime 	 = localTimeSinceLastResume;
	} else {
		analysisTimes.lastNativeWaitingTime 	 += localTimeSinceLastResume;
	}
	if (-1000000 > (gvtBefore - analysisTimes.lastGvtAfter) ||  (gvtBefore - analysisTimes.lastGvtAfter) > 1000000) {
		str << ", LEAP FROM ONE GVT TO THE NEXT";
		++errors;
	} else {
		str << ",";
	}
	analysisTimes.lastGvtAfter = gvtAfter;
	if (errors > 0) {
		return str.str().c_str();
	} else {
		return "OK";
	}
}

//TODO: are there any more interesting model related checks? I just copied the CpuTime checks
const char *ModelSimulatingTimeSample::checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const {

	std::stringstream str;
	short errors = 0;

	if (analysisTimes.lastNativeWaitingStartTime == resumedLastAt && (analysisTimes.lastNativeWaitingStartTime + analysisTimes.lastNativeWaitingTime + localTimeSinceLastResume < gvtAfter)) {
		str << ", LOST TIME FROM NW";
		++errors;
	} else {
		str << ",";
	}
	if (-1000000 > (gvtBefore - analysisTimes.lastGvtAfter) ||  (gvtBefore - analysisTimes.lastGvtAfter) > 1000000) {
		str << ", LEAP FROM ONE GVT TO THE NEXT";
		++errors;
	} else {
		str << ",";
	}
	analysisTimes.lastGvtAfter = gvtAfter;
	analysisTimes.resetNativeWaitingCounters();

	if (errors > 0) {
		return str.str().c_str();
	} else {
		return "OK";
	}
}

// TODO - Add some meaningful validity checks based on the type of event logged instead of just returning ok
const char *WaitingTimeSample::checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const {
	return "OK";
}
const char *TimedWaitingTimeSample::checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const {
	return "OK";
}
const char *TimedOutIoTimeSample::checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const {
	return "OK";
}
const char *BackgroundLoadExecutionTimeSample::checkInterThreadValidity(CombinedTimeSamplesAnalysis &analysisTimes) const {
	return "OK";
}



ThreadTimeSamples::ThreadTimeSamples(VexThreadState *_state) {
	state = _state;
	threadName = new char[strlen(state->getName())+1];
	strcpy(threadName, state->getName());
}
//hhhhhhhhhhhhhhhhhhh
void ThreadTimeSamples::log(ThreadTimeSample *sample) {
	times.push_back(sample);
}
//void ThreadTimeSamples::logCpuTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
//	times.push_back(new CpuTimeSample(state, before, after));
//}
//void ThreadTimeSamples::logIoTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
//	times.push_back(new IoTimeSample(state, before, after));
//
//}
//void ThreadTimeSamples::logNativeWaitingTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
//	times.push_back(new NativeWaitingTimeSample(state, before, after));
//
//}
//void ThreadTimeSamples::logWaitingTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
//	times.push_back(new WaitingTimeSample(state, before, after));
//}
//void ThreadTimeSamples::logModelSimulationTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
//	times.push_back(new ModelSimulatingTimeSample(state, before, after));
//
//}
//void ThreadTimeSamples::logTimedWaitingTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
//	times.push_back(new TimedWaitingTimeSample(state, before, after));
//}
//
//void logBackgroundLoadExecutionTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
//	times.push_back(new TimedWaitingTimeSample(state, before, after));
//}

void ThreadTimeSamples::addAllSamples(std::vector<ThreadTimeSample *> *allSamples) {
	std::vector<ThreadTimeSample *>::iterator vit = times.begin();
	while (vit != times.end()) {
		(*vit)->setName(threadName);
		++vit;
	}
	allSamples->insert(allSamples->end(), times.begin(), times.end());
}

void ThreadTimeSamples::print(std::ostream &outs) {
	std::vector<ThreadTimeSample *>::iterator vit = times.begin();
	outs << threadName;
	while (vit != times.end()) {
		outs << "," << threadName << *(*vit) << endl;
		++vit;
	}

}

ThreadTimeSamples *SamplesPerThreadAnalysis::getTimelinesOfThread(VexThreadState *state) {
	int threadId;
	if (state != NULL) {
		threadId = state->getId();
	} else {
		threadId = -1;	// used for global system events (background load)
	}

	std::map<int, ThreadTimeSamples *>::iterator mit = threadsToSamples.find(threadId);
	ThreadTimeSamples *samples;
	if (mit == threadsToSamples.end()) {
		samples = new ThreadTimeSamples(state);
		threadsToSamples[threadId] = samples;
	} else {
		samples = mit->second;
	}
	return samples;
}

void SamplesPerThreadAnalysis::logCpuTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	ThreadTimeSamples *threadSamples = getTimelinesOfThread(state);
	threadSamples->log(new CpuTimeSample(state, before, after));
}
void SamplesPerThreadAnalysis::logIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	ThreadTimeSamples *threadSamples = getTimelinesOfThread(state);
	threadSamples->log(new IoTimeSample(state, before, after));
}
void SamplesPerThreadAnalysis::logNativeWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	ThreadTimeSamples *threadSamples = getTimelinesOfThread(state);
	threadSamples->log(new NativeWaitingTimeSample(state,before, after));
}
void SamplesPerThreadAnalysis::logWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	ThreadTimeSamples *threadSamples = getTimelinesOfThread(state);
	threadSamples->log(new WaitingTimeSample(state, before, after));
}
void SamplesPerThreadAnalysis::logModelSimulationTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	ThreadTimeSamples *threadSamples = getTimelinesOfThread(state);
	threadSamples->log(new ModelSimulatingTimeSample(state, before, after));
}
void SamplesPerThreadAnalysis::logTimedWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	ThreadTimeSamples *threadSamples = getTimelinesOfThread(state);
	threadSamples->log(new TimedWaitingTimeSample(state, before, after));
}
void SamplesPerThreadAnalysis::logTimedOutIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	ThreadTimeSamples *threadSamples = getTimelinesOfThread(state);
	threadSamples->log(new TimedOutIoTimeSample(state, before, after));
}

void SamplesPerThreadAnalysis::logBackgroundLoadExecutionTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	ThreadTimeSamples *threadSamples = getTimelinesOfThread(NULL);
	threadSamples->log(new BackgroundLoadExecutionTimeSample(before, after));

}

long long SamplesPerThreadAnalysis::printScaled(const long long &value) {
	return value / 1000000;
}

const char * SamplesPerThreadAnalysis::getColumnTitles() {
	return "";
}

void SamplesPerThreadAnalysis::perThreadAnalysis(std::ostream &outs) {
	outs << "Per thread analysis" << endl;
	std::map<int, ThreadTimeSamples *>::iterator sit = threadsToSamples.begin();
	while (sit != threadsToSamples.end()) {
		sit->second->print(outs);
		++sit;
	}
}




ThreadTimeSample::ThreadTimeSample(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	gvtBefore = before->getGlobalTime();
	gvtAfter = after->getGlobalTime();
	resumedLastAt = state->getResumedLastAt();
	localTimeSinceLastResume = state->getLocalTimeSinceLastResume();
	estimatedRealTime = state->getEstimatedRealTime();
	threadName = NULL;
}


bool ThreadTimeSample::compareTo(ThreadTimeSample *second) {

	if (gvtBefore > second->gvtBefore) {
		return true;
	} else if (gvtBefore == second->gvtBefore) {
		if (gvtAfter < second->gvtAfter) {
			return true;
		} else if (gvtAfter == second->gvtAfter) {
			return resumedLastAt < second->resumedLastAt;
		}
	}
	return false;
}

struct compare_ThreadTimeSamples : binary_function<ThreadTimeSample *, ThreadTimeSample *, bool> {
	bool operator() (ThreadTimeSample *lhs, ThreadTimeSample *rhs) const {
		return rhs->compareTo(lhs);
	}
};


void SamplesPerThreadAnalysis::combinedAnalysis(std::ostream &outs) {
	outs  << "Combined analysis" << endl;

	outs  << "From GVT,To GVT,Thread,Type,LastResumed,TimeSinceLastResumed,ERT" << endl;
	std::vector<ThreadTimeSample *> *allSamples = new std::vector<ThreadTimeSample *>;	// sample initial space

	std::map<int, ThreadTimeSamples *>::iterator sit = threadsToSamples.begin();
	while (sit != threadsToSamples.end()) {
		sit->second->addAllSamples(allSamples);
		++sit;
	}

	std::sort(allSamples->begin(), allSamples->end(), compare_ThreadTimeSamples());
	CombinedTimeSamplesAnalysis analysisTimes;
	std::vector<ThreadTimeSample *>::iterator allit = allSamples->begin();
	while (allit != allSamples->end()) {
		(*allit)->printSortedByGvt(outs, analysisTimes);
		++allit;
	}
	delete allSamples;

}
void SamplesPerThreadAnalysis::printStats(std::ostream &outs) {
	combinedAnalysis(outs);
	outs << endl << endl << endl<< endl << endl << endl<< endl << endl << endl;
	perThreadAnalysis(outs);


}
/*
void SamplesPerThreadAnalysis::printExtraStats(const char *outputDir) {

}
*/



/**
 * Wrapper (Subject)
 */
bool TimeLoggingBehaviour::usingExtendedStats = false;
bool TimeLoggingBehaviour::usingGslStats = false;
TimeLoggingBehaviour::TimeLoggingBehaviour() {
	allLogs.push_back(&cputimelogs);
	allLogs.push_back(&iotimelogs);
	allLogs.push_back(&nativewaitingtimelogs);
	allLogs.push_back(&waitingtimelogs);
	allLogs.push_back(&modelsimulationtimelogs);
	allLogs.push_back(&timedoutiotimelogs);
	allLogs.push_back(&timedwaitingtimelogs);
}

void TimeLoggingBehaviour::enableExtendedStats() {
	usingExtendedStats = true;
}
void TimeLoggingBehaviour::enableGslStats() {
	usingGslStats = true;
}

void TimeLoggingBehaviour::addOptionalLoggers() {
	if (TimeLoggingBehaviour::usingExtendedStats) {
		//addExtendedTimeLoggers();
	}
	if (TimeLoggingBehaviour::usingGslStats) {
		//addPostTimeLogger(new ActualTimesGslHistogram());
	}
}

/*
 * Add one logger to each category
 */
void TimeLoggingBehaviour::addGlobalLoggers(TimeLogger *log) {
	cputimelogs.push_back(log);
	iotimelogs.push_back(log);
	nativewaitingtimelogs.push_back(log);
	waitingtimelogs.push_back(log);
	modelsimulationtimelogs.push_back(log);
	timedoutiotimelogs.push_back(log);
	timedwaitingtimelogs.push_back(log);
	backgroundloadexecutiontimelogs.push_back(log);
}

void TimeLoggingBehaviour::addLocalTimeLoggers() {

}

void TimeLoggingBehaviour::addExtendedTimeLoggers() {
//	addPostTimeLogger(new AllActualIoSamples());
}


void TimeLoggingBehaviour::logCpuTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	vector<TimeLogger *>::iterator vit = cputimelogs.begin();
	while (vit != cputimelogs.end()) {
		(*vit)->logCpuTime(state, before, after);
		++vit;
	}
}
void TimeLoggingBehaviour::logIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	vector<TimeLogger *>::iterator vit = iotimelogs.begin();
	while (vit != iotimelogs.end()) {
		(*vit)->logIoTime(state, before, after);
		++vit;
	}
}
void TimeLoggingBehaviour::logNativeWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	vector<TimeLogger *>::iterator vit = nativewaitingtimelogs.begin();
	while (vit != nativewaitingtimelogs.end()) {
		(*vit)->logNativeWaitingTime(state, before, after);
		++vit;
	}
}
void TimeLoggingBehaviour::logWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	vector<TimeLogger *>::iterator vit = waitingtimelogs.begin();
	while (vit != waitingtimelogs.end()) {
		(*vit)->logWaitingTime(state, before, after);
		++vit;
	}
}
void TimeLoggingBehaviour::logModelSimulationTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	vector<TimeLogger *>::iterator vit = modelsimulationtimelogs.begin();
	while (vit != modelsimulationtimelogs.end()) {
		(*vit)->logModelSimulationTime(state, before, after);
		++vit;
	}
}
void TimeLoggingBehaviour::logTimedWaitingTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	vector<TimeLogger *>::iterator vit = timedwaitingtimelogs.begin();
	while (vit != timedwaitingtimelogs.end()) {
		(*vit)->logTimedWaitingTime(state, before, after);
		++vit;
	}
}
void TimeLoggingBehaviour::logTimedOutIoTime(VexThreadState *state, VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	vector<TimeLogger *>::iterator vit = timedoutiotimelogs.begin();
	while (vit != timedoutiotimelogs.end()) {
		(*vit)->logTimedOutIoTime(state, before, after);
		++vit;
	}
}

void TimeLoggingBehaviour::logBackgroundLoadExecutionTime(VirtualTimeSnapshot *before, VirtualTimeSnapshot *after) {
	vector<TimeLogger *>::iterator vit = backgroundloadexecutiontimelogs.begin();
	while (vit != backgroundloadexecutiontimelogs.end()) {
		(*vit)->logBackgroundLoadExecutionTime(before, after);
		++vit;
	}
}

const char *TimeLoggingBehaviour::getTitlesInCSVRow() {
	stringstream stream;
	vector< vector<TimeLogger *> * >::iterator allLogit = allLogs.begin();
	while (allLogit != allLogs.end()) {
		vector<TimeLogger *>::iterator vit= (*allLogit)->begin();
		while (vit != (*allLogit)->end()) {
			TimeLogger *log = (*vit);
			stream << log->getColumnTitles() << ",";
			++vit;
		}
		++allLogit;
	}
	return stream.str().c_str();
}

const char *TimeLoggingBehaviour::getValuesInCSVRow() {
	stringstream stream;
	vector< vector<TimeLogger *> *>::iterator allLogit = allLogs.begin();
	while (allLogit != allLogs.end()) {
		vector<TimeLogger *>::iterator vit= (*allLogit)->begin();
		while (vit != (*allLogit)->end()) {
			TimeLogger *log = (*vit);
			stream << log->getColumnTitles() << ",";
			++vit;
		}
		++allLogit;
	}
	return stream.str().c_str();
}

void TimeLoggingBehaviour::getTitlesInCSVRow(ostream &outs) {
	stringstream stream;
	vector< vector<TimeLogger *> *>::iterator allLogit = allLogs.begin();
	while (allLogit != allLogs.end()) {
		vector<TimeLogger *>::iterator vit= (*allLogit)->begin();
		while (vit != (*allLogit)->end()) {
			TimeLogger *log = (*vit);
			outs << log->getColumnTitles() << ",";
			++vit;
		}
		++allLogit;
	}
}

void TimeLoggingBehaviour::getValuesInCSVRow(ostream &outs) {
	stringstream stream;
	vector< vector<TimeLogger *> *>::iterator allLogit = allLogs.begin();
	while (allLogit != allLogs.end()) {
		vector<TimeLogger *>::iterator vit= (*allLogit)->begin();
		while (vit != (*allLogit)->end()) {
			(*vit)->printStats(outs);
			outs << ",";
			++vit;
		}
		++allLogit;
	}
}

/*
 * Print statistics gathered from the I/O duration prediction buffer
 */
void TimeLoggingBehaviour::print(const char *file_name) {
	std::filebuf fb;
	fb.open(file_name, std::ios::out);
	std::ostream os(&fb);
	stringstream stream;
	vector< vector<TimeLogger *> *>::iterator allLogit = allLogs.begin();
	while (allLogit != allLogs.end()) {
		vector<TimeLogger *>::iterator vit= (*allLogit)->begin();

		while (vit != (*allLogit)->end()) {
			TimeLogger *log = (*vit);
			os << log->getTitle() << endl;
			os << log->getColumnTitles() << endl;
			log->printStats(os);
			os << endl;
			++vit;
		}
		// TODO: that was pretty bad....
		fb.close();
		return;
		++allLogit;
	}
	fb.close();
}


void TimeLoggingBehaviour::printExtraStats(const char *outputDir) {
//	std::filebuf fb;
//	fb.open(file_name, std::ios::out);
//	std::ostream os(&fb);
//	stringstream stream;
//	vector< vector<TimeLogger *> *>::iterator allLogit = allLogs.begin();
//	while (allLogit != allLogs.end()) {
//		vector<TimeLogger *>::iterator vit= (*allLogit)->begin();
//		while (vit != (*allLogit)->end()) {
//			TimeLogger *log = (*vit);
////			log->printExtraStats(outputDir);
//			++vit;
//		}
//		++allLogit;
//	}
//	fb.close();
}


TimeLoggingBehaviour::~TimeLoggingBehaviour() {

}




