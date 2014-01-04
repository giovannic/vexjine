/*
 * ThreadManagerCommunicationCodes.h
 *
 *  Created on: 4 Oct 2010
 *      Author: root
 */

#ifndef THREADMANAGERCOMMUNICATIONPROTOCOL_H_
#define THREADMANAGERCOMMUNICATIONPROTOCOL_H_

#define DEBUGCLIENTSERVER 0

#define REQUEST_GLOBALTIME 						1
#define REQUEST_UPDATE_THREAD_LIST 				2
#define REQUEST_PS 								3
#define REQUEST_PRINTTHREADSTATES 				4
#define REQUEST_PRINTSTATS 						5
#define REQUEST_WAKEUP 							6
#define REQUEST_CONDITIONAL_WAKEUP 				7
#define REQUEST_UNCONDITIONAL_WAKEUP 			8
#define REQUEST_NOTIFY_SCHEDULER_FOR_VT 		9
#define REQUEST_SUSPEND_THREAD					10
#define REQUEST_SET_GLOBAL_TIME 				11
#define REQUEST_SHOULD_CURRENT_THREAD_SUSPEND 	12
#define REQUEST_SET_WAITING 					13
#define REQUEST_SET_NATIVE_WAITING 				14
#define REQUEST_SET_TIMED_WAITING 				15
#define REQUEST_SET_IO_THREAD 					16
#define REQUEST_UPDATECURRENT_TIME 				17
#define REQUEST_SETCURRENT_TIME 				18
#define REQUEST_SETCURRENT_TIME_FROM_HANDLER 	19
#define REQUEST_SET_THREAD_RUNNABLE		 		20
#define REQUEST_ADD_NEW_THREADSTATE 			21
#define NOTIFY_THREAD_WAITING_START				22
#define NOTIFY_THREAD_TIMED_WAITING_START		23
#define NOTIFY_THREAD_END						24
#define NOTIFY_TIMED_WAITING_END				25
#define NOTIFY_THREAD_BEFORE_YIELD				26
#define REQUEST_SET_RUNNING						27
#define NOTIFY_TO_BE_DISREGARDED				28
#define REQUEST_TRY_SUSPEND_CURRENT_THREAD		29
#define REQUEST_CHANGE_STATE_TO_RUNNING			30
#define REQUEST_TERMINATE						31
#define NOTIFY_IO_START							32
#define NOTIFY_IO_END							33
#define REQUEST_THREAD_TIMES					34
#define REQUEST_PROGRESS_TIME_BY				35
#define REQUEST_SET_GLOBAL_TIME_TO_THREADTIME   36
#define NOTIFY_THREAD_CONTENDED_ENTER			37
#define REQUEST_UPDATE_GLOBAL_TIME_BY			38
#define NOTIFY_THAT_THREAD_WAS_INTERRUPTED		39

#define SUSPEND_THREAD 				100
#define RESUME_THREAD				101
#define TIMED_WAITING_END_THREAD 	102
#define NOTIFY_SUSPENDED			103
#define NOTIFY_NOT_SUSPENDED		104
#define PROCESS_EXIT				105

#define TIME_CODE_DENOTING_THAT_THREAD_IS_NATIVE_WAITING -1
#define TIME_CODE_DENOTING_THAT_THREAD_WAS_NOT_SUSPENDED -2


#include "ThreadState.h"
//#include "Logger.h"
//
//static Log communicationLog("vtf_comm_log.txt");


/*
class SchedulerRequest {

public:
	SchedulerRequest() : code(0), state(NULL) {};
	SchedulerRequest(int _code): code(_code), state(NULL) {};
	SchedulerRequest(int _code, VexThreadState *_state): code(_code), state(_state->state->getVtfClientStatePtr()) {};

	int code;
	VexThreadState *state;

};

*/

class SchedulerRequest {

public:
	SchedulerRequest() : code(0), state(NULL), timestamp(0) {};
	SchedulerRequest(char _code): code(_code), state(NULL), timestamp(0) {};
	SchedulerRequest(char _code, long long _time): code(_code), state(NULL), timestamp(_time) {};
	SchedulerRequest(char _code, VexThreadState *_state): code(_code), state(_state) {};
	SchedulerRequest(char _code, VexThreadState *_state, long long _time): code(_code), state(_state), timestamp(_time) {};
	SchedulerRequest(char _code, VexThreadState *_state, long long _time, char _options): code(_code), state(_state), timestamp(_time), options(_options) {};
	SchedulerRequest(char _code, VexThreadState *_state, long long _time, int _ioInvocationPointHashValue, int _stackTraceHash): code(_code), state(_state), timestamp(_time), ioInvocationPointHashValue(_ioInvocationPointHashValue), stackTraceHash(_stackTraceHash) {};
	SchedulerRequest(char _code, VexThreadState *_state, float _speedup): code(_code), state(_state), virtualTimeSpeedup(_speedup) {};

	void send(const int &fd) {
		if (write(fd, this, sizeof(SchedulerRequest)) < 0) {
			cerr << "VTF client:: error writing schedulerRequest" << endl;
		}
	};

	static const char *codeToString(char code) {
		switch (code) {
			case REQUEST_GLOBALTIME: return "REQUEST_GLOBALTIME";
			case REQUEST_UPDATE_THREAD_LIST: return "REQUEST_UPDATE_THREAD_LIST";
			case REQUEST_PS: return "REQUEST_PS";
			case REQUEST_PRINTTHREADSTATES: return "REQUEST_PRINTTHREADSTATES";
			case REQUEST_PRINTSTATS: return "REQUEST_PRINTSTATS";
			case REQUEST_WAKEUP: return "REQUEST_WAKEUP";
			case REQUEST_CONDITIONAL_WAKEUP: return "REQUEST_CONDITIONAL_WAKEUP";
			case REQUEST_UNCONDITIONAL_WAKEUP: return "REQUEST_UNCONDITIONAL_WAKEUP";
			case REQUEST_NOTIFY_SCHEDULER_FOR_VT: return "REQUEST_NOTIFY_SCHEDULER_FOR_VT";
			case REQUEST_SUSPEND_THREAD: return "REQUEST_SUSPEND_THREAD";
			case REQUEST_SET_GLOBAL_TIME: return "REQUEST_SET_GLOBAL_TIME";
			case REQUEST_SHOULD_CURRENT_THREAD_SUSPEND: return "REQUEST_SHOULD_CURRENT_THREAD_SUSPEND";
			case REQUEST_SET_WAITING: return "REQUEST_SET_WAITING";
			case REQUEST_SET_NATIVE_WAITING: return "REQUEST_SET_NATIVE_WAITING";
			case REQUEST_SET_TIMED_WAITING: return "REQUEST_SET_TIMED_WAITING";
			case REQUEST_SET_IO_THREAD: return "REQUEST_SET_IO_THREAD";
			case REQUEST_UPDATECURRENT_TIME: return "REQUEST_UPDATECURRENT_TIME";
			case REQUEST_SETCURRENT_TIME: return "REQUEST_SETCURRENT_TIME";
			case REQUEST_SETCURRENT_TIME_FROM_HANDLER: return "REQUEST_SETCURRENT_TIME_FROM_HANDLER";
			case REQUEST_SET_THREAD_RUNNABLE: return "REQUEST_SET_THREAD_RUNNABLE";
			case REQUEST_ADD_NEW_THREADSTATE: return "REQUEST_ADD_NEW_THREADSTATE";
			case NOTIFY_THREAD_WAITING_START: return "NOTIFY_THREAD_WAITING_START";
			case NOTIFY_THREAD_TIMED_WAITING_START: return "NOTIFY_THREAD_TIMED_WAITING_START";
			case NOTIFY_THREAD_END: return "NOTIFY_THREAD_END";
			case NOTIFY_TIMED_WAITING_END: return "NOTIFY_TIMED_WAITING_END";
			case NOTIFY_THREAD_BEFORE_YIELD: return "NOTIFY_THREAD_BEFORE_YIELD";
			case REQUEST_SET_RUNNING: return "REQUEST_SET_RUNNING";
			case NOTIFY_TO_BE_DISREGARDED: return "NOTIFY_TO_BE_DISREGARDED";
			case REQUEST_TRY_SUSPEND_CURRENT_THREAD: return "REQUEST_TRY_SUSPEND_CURRENT_THREAD";
			case REQUEST_CHANGE_STATE_TO_RUNNING: return "REQUEST_CHANGE_STATE_TO_RUNNING";
			case REQUEST_TERMINATE: return "REQUEST_TERMINATE";
			case NOTIFY_IO_START: return "NOTIFY_IO_START";
			case NOTIFY_IO_END: return "NOTIFY_IO_END";
			case REQUEST_THREAD_TIMES: return "REQUEST_THREAD_TIMES";
			case REQUEST_PROGRESS_TIME_BY: return "REQUEST_PROGRESS_TIME_BY";
			case REQUEST_SET_GLOBAL_TIME_TO_THREADTIME: return "REQUEST_SET_GLOBAL_TIME_TO_THREADTIME";
			case SUSPEND_THREAD: return "SUSPEND_THREAD";
			case RESUME_THREAD: return "RESUME_THREAD";
			case TIMED_WAITING_END_THREAD: return "TIMED_WAITING_END_THREAD";
			case NOTIFY_SUSPENDED: return "NOTIFY_SUSPENDED";
			case NOTIFY_NOT_SUSPENDED: return "NOTIFY_NOT_SUSPENDED";
			case NOTIFY_THREAD_CONTENDED_ENTER: return "NOTIFY_THREAD_CONTENDED_ENTER";
			case PROCESS_EXIT: return "PROCESS_EXIT";
			case REQUEST_UPDATE_GLOBAL_TIME_BY: return "REQUEST_UPDATE_GLOBAL_TIME_BY";
			case NOTIFY_THAT_THREAD_WAS_INTERRUPTED: return "NOTIFY_THAT_THREAD_WAS_INTERRUPTED";
			default: return "Unknown signal";
		}
	}

	char code;
	VexThreadState *state;
	long long timestamp;
	char options;
	int ioInvocationPointHashValue;
	int stackTraceHash;
	float virtualTimeSpeedup;
};

class VexThreadStateInfo {
public:
	VexThreadStateInfo() { };

	VexThreadStateInfo(VexThreadState *state) {
		strncpy(name, state->getName(), 128);
		managingSchedulerFd = state->getManagingSchedulerFd();
		clientStatePtr = state;
		uniqueId = state->getUniqueId();
		tid = state->getId();
	}

	void setInto (VexThreadState *state) {
		if (state != NULL) {
			state->setName(name);
			state->setManagingSchedulerFd(managingSchedulerFd);
			state->setVtfClientStatePtr(clientStatePtr);
			state->setUniqueId(1000000*state->getManagingSchedulerFd() + uniqueId);
			state->setId(tid);
		}
	};
	char name[128];
	int managingSchedulerFd;
	VexThreadState *clientStatePtr;
	long uniqueId;
	unsigned long tid;

};
#endif /* THREADMANAGERCOMMUNICATIONPROTOCOL_H_ */
