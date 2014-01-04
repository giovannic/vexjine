#include "Visualizer.h"
#include "VirtualTimeline.h"
#include "ThreadManager.h"
#include "ThreadState.h"
#include "EventLogger.h"
#include <fstream>
#include <algorithm>

VisualizerRecord::VisualizerRecord(VisualizerEvent::Type _eventId, long long _timestamp) {
	setEventInfo(_eventId, 0, _timestamp, VexThreadStates::UNKNOWN_STATE, NULL);
}

VisualizerRecord::VisualizerRecord(VisualizerEvent::Type _eventId, VexThreadState *state) {
	setEventInfo(_eventId, state->getId(), state->getEstimatedRealTime(), state->getState(), NULL);
}

VisualizerRecord::VisualizerRecord(VisualizerEvent::Type _eventId, VexThreadState *state, const long long &currentTime) {
	setEventInfo(_eventId, state->getId(), currentTime, state->getState(), NULL);
}

VisualizerRecord::VisualizerRecord(VisualizerEvent::Type _eventId, VexThreadState *state, const char *_methodName) {
	setEventInfo(_eventId, state->getId(), state->getEstimatedRealTime(), state->getState(), _methodName);
}

VisualizerRecord::VisualizerRecord(VisualizerEvent::Type _eventId, VexThreadState *state, const char *_methodName, const long long &currentTime) {
	setEventInfo(_eventId, state->getId(), currentTime, state->getState(), _methodName);
}

Visualizer::Visualizer(VirtualTimeline *_virtualTimelineController, EventLogger *_eventLogger) {
	virtualTimeline = _virtualTimelineController;
	basetime = 0;		
	using_real = false;
	eventLogger = _eventLogger;
	pthread_spin_init(&spinlock, 0);
}

Visualizer::Visualizer(VirtualTimeline *_virtualTimelineController, EventLogger *_eventLogger, long long realBaseTime) {
	virtualTimeline = _virtualTimelineController;
	basetime = realBaseTime;
	using_real = true;
	eventLogger = _eventLogger;
	pthread_spin_init(&spinlock, 0);

}

void Visualizer::setBaseTime(long long realBaseTime) {
	basetime = realBaseTime;
	using_real = true;
}

Visualizer::~Visualizer() {
	
}

void Visualizer::recordEvent(VisualizerEvent::Type event_id, long long _timestamp) {
	
	if (using_real) {
		_timestamp -= basetime;	
	}
	pthread_spin_lock(&spinlock);
	timeline.push_back(new VisualizerRecord(event_id, _timestamp));
	pthread_spin_unlock(&spinlock);
}

void Visualizer::recordEvent(VisualizerEvent::Type event_id, VexThreadState *state) {

	if (event_id == 1) {
		pthread_spin_lock(&spinlock);
		threadNames[state->getId()] = state->getName();
		pthread_spin_unlock(&spinlock);
	}


	if (event_id == VisualizerEvent::SPECIAL_TIMED_WAIT) {
		pthread_spin_lock(&spinlock);
		timeline.push_back(new VisualizerRecord(VisualizerEvent::WAIT, state, virtualTimeline->getGlobalTime()));
		pthread_spin_unlock(&spinlock);
	} else {
		if (using_real) {
			long long currentTime = state->getEstimatedRealTime() - basetime;
			pthread_spin_lock(&spinlock);
			timeline.push_back(new VisualizerRecord(event_id, state, currentTime));
			pthread_spin_unlock(&spinlock);
		}
		pthread_spin_lock(&spinlock);
		timeline.push_back(new VisualizerRecord(event_id, state));
		pthread_spin_unlock(&spinlock);
	}

}


void Visualizer::recordEvent(VisualizerEvent::Type event_id, VexThreadState *state, const int & methodId) {
	if (event_id == 1) {
		threadNames[state->getId()] = state->getName();
	}
	if (using_real) {
		pthread_spin_lock(&spinlock);
		long long currentTime = state->getEstimatedRealTime() - basetime;
		timeline.push_back(new VisualizerRecord(event_id, state, eventLogger->getMethodName(methodId), currentTime));
		pthread_spin_unlock(&spinlock);
	} else {
		pthread_spin_lock(&spinlock);
		timeline.push_back(new VisualizerRecord(event_id, state, eventLogger->getMethodName(methodId)));
		pthread_spin_unlock(&spinlock);
	}
}


struct VisualizerRecordCompare : public std::binary_function<VisualizerRecord *, VisualizerRecord *, bool> {

	bool operator()(const VisualizerRecord *lhs, const VisualizerRecord *rhs) {
		return lhs->getTime() < rhs->getTime();
	}

};

void Visualizer::writeToFile(const char *filename) {

	filebuf fb;
	
	fb.open(filename, ios::out);

	ostream os(&fb);
	
    std::map<unsigned long, std::string>::iterator iter;
	iter = threadNames.begin();
	while (iter != threadNames.end()) {
		os << iter->first << "," << iter->second <<endl;
		iter++;
	}	
		
	os << endl;

	std::sort(timeline.begin(), timeline.end(), VisualizerRecordCompare());
	int length = timeline.size();	
	for (int i =0; i<length; i++) {
		os << *(timeline[i]);   
	}

	fb.close();

}
		
//void Visualizer::registerMethod(const char *methodName, int methodId) {
//	std::string *fqName = new std::string(methodName);
//	methodRegistry[methodId] = fqName;
//}

void Visualizer::clear() {
	basetime = 0;
	using_real = false;

	timeline.clear();
	//threadNames.clear();
//	methodRegistry.clear();

}

void Visualizer::clearThreads() {
	threadNames.clear();
}

