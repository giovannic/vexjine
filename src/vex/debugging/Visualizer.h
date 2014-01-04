/*
 * Visualizer.h: Class for exporting events compatible with the VEX Visualizer Java applet
 *
 *  Created on: 12 Aug 2011
 *      Author: nb605
 */

#ifndef VISUALIZER_H_
#define VISUALIZER_H_


#include "Constants.h"

#include <pthread.h>
#include <vector>
#include <map>
#include <pthread.h>
#include <fstream>
#include <string>

class VirtualTimeline;
class VexThreadState;
class EventLogger;

namespace VisualizerEvent {
	enum Type {
	 THREAD_START = 1, THREAD_END = 2,
	 METHOD_ENTER = 3, METHOD_EXIT = 4,
	 SUSPEND = 5, RESUME = 6, WAIT = 7, WAITING_RELEASED = 8, WAITING_TIMEOUT = 9,
	 METHOD_IO_ENTER = 10, METHOD_IO_EXIT = 11,
	 SUSPEND_SELF = 12,
	 GC_STARTED = 13, GC_FINISHED = 14,
	 IO_PREDICTION = 15,
	 INTERNAL_SOCKET_READ  	= 16, INTERNAL_SOCKET_READ_NULL_WAIT 	= 17, 	INTERNAL_SOCKET_READ_RESUME  	= 18,
	 INTERNAL_SOCKET_WRITE 	= 19, INTERNAL_SOCKET_WRITE_FULL_WAIT	= 20, 		INTERNAL_SOCKET_WRITE_RESUME  	= 21,
	 INTERNAL_SOCKET_WRITE_RESUME_READ = 22,
	 SET_NATIVE_WAITING = 23, SPECIAL_TIMED_WAIT = 24};
}


#define VISUALIZE_EVENT(event, state) \
if (visualizer != NULL) visualizer->recordEvent(VisualizerEvent::event, state);

#define VISUALIZE_TIME_EVENT(event, time) \
if (visualizer != NULL) visualizer->recordEvent(VisualizerEvent::event, time);

#define VISUALIZE_METHOD_EVENT(event, state, methodId) \
if (visualizer != NULL) visualizer->recordEvent(VisualizerEvent::event, state, methodId);

#define VISUALIZE_EVENT_OF_THIS_THREAD(event) \
if (visualizer != NULL) visualizer->recordEvent(VisualizerEvent::event, this);



/*
 * Class that stores info for every event
 */
class VisualizerRecord {

public:

	VisualizerRecord(VisualizerEvent::Type _eventId, long long _timestamp) ;
	VisualizerRecord(VisualizerEvent::Type _eventId, VexThreadState *state) ;
	VisualizerRecord(VisualizerEvent::Type _eventId, VexThreadState *state, const long long &currentTime);
	VisualizerRecord(VisualizerEvent::Type _eventId, VexThreadState *state, const char *_methodName) ;
	VisualizerRecord(VisualizerEvent::Type _eventId, VexThreadState *state, const char *_methodName, const long long &currentTime) ;
	
	friend std::ostream & operator <<(std::ostream &outs, const VisualizerRecord &record) {

		if (record.methodName != NULL) {
			outs << record.threadId << "," << record.eventId << "," << record.realTime << "," << record.threadState << "," << record.methodName << std::endl;
		} else {
			outs << record.threadId << "," << record.eventId << "," << record.realTime << "," << record.threadState << std::endl;
		}
		return outs; 
	}; 

	const long long &getTime() const {
		return realTime;
	}

private:
	void setEventInfo(VisualizerEvent::Type _eventId, const unsigned long &_threadId, const long long &_realTime, VexThreadStates::Code _threadState, const char *_methodName) {
		eventId 	= _eventId;
		threadId 	= _threadId;
		realTime 	= _realTime;
		threadState = _threadState;
		methodName 	= _methodName;
	}

	VisualizerEvent::Type eventId;
	VexThreadStates::Code threadState;

	unsigned long threadId;

	const char *methodName;
	long long realTime;
};



/*
 * Class that aggregates VisualizerRecord objects in a vector
 */
class Visualizer {

public:
	Visualizer(VirtualTimeline *_globalTimer, EventLogger *_eventLogger);	// needed for converting ids to names
	Visualizer(VirtualTimeline *_globalTimer, EventLogger *_eventLogger, long long realBaseTime);
	~Visualizer();
	
	void setBaseTime(long long realBaseTime);
	void recordEvent(VisualizerEvent::Type event_id, long long _timestamp);
	void recordEvent(VisualizerEvent::Type event_id, VexThreadState *state);
	void recordEvent(VisualizerEvent::Type event_id, VexThreadState *state, const char *methodName);
	void recordEvent(VisualizerEvent::Type event_id, VexThreadState *state, const int &methodId);
	void writeToFile(const char* filename);
//	std::tr1::unordered_map<int, std::string *> methodRegistry;
//	void registerMethod(const char *methodName, int methodId);
	void clear();
	void clearThreads();

protected:
	long long basetime;
	bool using_real;
	std::vector<VisualizerRecord *> timeline;
	std::map<unsigned long, std::string> threadNames;

	EventLogger *eventLogger;
	pthread_spinlock_t spinlock;
	VirtualTimeline *virtualTimeline;
};

#endif /*VISUALIZER_H_*/
