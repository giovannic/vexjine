/*
 * ThreadExecutionPattern.cpp
 *
 *  Created on: 14 Oct 2010
 *      Author: root
 */

#include "ThreadExecutionPattern.h"

#include "ThreadState.h"

#include <cstdlib>
#include <sys/time.h>

int ThreadExecutionPattern::totalThreads = 0;
ThreadExecutionPattern::ThreadExecutionPattern() {
	

}

ThreadExecutionPattern::~ThreadExecutionPattern() {
	 
}

/***
 * startsuspendexit: functional test using event timestamps to show whether multiple threads are
 * correctly interleaved
 */
StartSuspendExit::StartSuspendExit(VexThreadState *_state) {
	state = _state;
	totalThreadTime = 1000000000;
}

StartSuspendExit::StartSuspendExit(VexThreadState * _state, long long _totalThreadTime) {
	state = _state;
	totalThreadTime = 1000000000;
}

void StartSuspendExit::apply() {
	long long threadTime = 0;

	do {
		threadTime += (rand() % 10000000);


		state->lockShareResourceAccessKey();
		if (AgentTester::virtualExecution) {
			manager-> suspendCurrentThread(state, threadTime, 0);
		}
		state->unlockShareResourceAccessKey();
		state->setLastCPUTime(threadTime);
	} while(threadTime < totalThreadTime);

}

StartSuspendExit::~StartSuspendExit() {

}


/***
 * constant I/O: performance test simulating I/O requests of 20ms each
 */
IssuingConstantIo::IssuingConstantIo(VexThreadState * _state, int _ioRequests) {
	state = _state;
	ioRequests = _ioRequests;
}


void IssuingConstantIo::apply() {
	bool realReal = false;

	struct timespec ioTime;
	ioTime.tv_sec = 0;
	cout << busyCpu(0.02) <<endl;

	for (int i = 0; i<ioRequests; i++) {
		ioTime.tv_nsec = 20000000;
		//ioTime.tv_nsec =  (long)(217680000 * duration);
		if (!realReal) {
			VEX::methodEventsBehaviour->afterIoMethodEntry(21, 20, false);
		}
		nanosleep(&ioTime, NULL);

		if (!realReal) {
			VEX::methodEventsBehaviour->beforeIoMethodExit(21);
		}

	}
	cout <<busyCpu(0.02) << endl;

}


/*
void IssuingConstantIo::apply() {
bool realReal = false;


	struct timespec ioTime;
	long long realTimeOnExit;
	ioTime.tv_sec = 0;
	long long endingTime ;

if (!realReal) {
	sleep(1);
	manager-> onThreadSpawn(state);






state->currentMethodInfo = eventLogger->logMethodEntry(state, 101);


	state->setIoInvocationPointHashValue( 10);

	cout << busyCpu(0.02) <<endl;

if (!realReal) {

	endingTime = Time::getVirtualTime();
	manager->setCurrentThreadVT(endingTime, state);
}
	for (int i = 0; i<ioRequests; i++) {
		ioTime.tv_nsec = 20000000;
		//ioTime.tv_nsec =  (long)(217680000 * duration);
if (!realReal) {

// push it to thread stack
state->currentMethodInfo = eventLogger->logMethodEntry(state, 21);

		ioSimulator -> startIo(state);
}
		nanosleep(&ioTime, NULL);

if (!realReal) {

		realTimeOnExit = Time::getRealTime();
		ioSimulator -> endIo(realTimeOnExit, state, 100);
state->getExitingMethodInfo()->setInfo(21, state->getVirtualTime(), state->getEstimatedRealTime());
state->logMethodExit();
}

	}

	cout <<busyCpu(0.02) << endl;
if (!realReal) {
	endingTime = Time::getVirtualTime();
	manager->setCurrentThreadVT(endingTime, state);


state->getExitingMethodInfo()->setInfo(101, state->getVirtualTime(), state->getEstimatedRealTime());
state->logMethodExit();

eventLogger->createMeasuresForThread(state);
	manager-> onThreadEnd(state);
}

}

*/

IssuingConstantIo::~IssuingConstantIo() {

}




IssuingIoWhileRunningLoops::IssuingIoWhileRunningLoops(VexThreadState * _state, int _ioRequests, bool _isRunner) {
	state 		= _state;
	ioRequests 	= _ioRequests;
	isRunner 	= _isRunner;
}

void IssuingIoWhileRunningLoops::apply() {

	struct timespec ioTime;
	ioTime.tv_sec = 0;

	if (isRunner) {
		long iterations = 795840000 * 0.02 * (double)ioRequests;
		double temp = 0.0;
		for (long i  =0 ; i<iterations; i++) {
			temp += 1;
		}

		cout << temp << endl;
		long long endingTime = Time::getVirtualTime();
		manager->setCurrentThreadVT(endingTime, state);
	} else {

		for (int i = 0; i<ioRequests; i++) {
			ioTime.tv_nsec = 20000000;
			//ioTime.tv_nsec =  (long)(217680000 * duration);

			methodEventsBehaviour->afterIoMethodEntry(2, 10, false);
			nanosleep(&ioTime, NULL);
			methodEventsBehaviour->beforeIoMethodExit(2);

		}
	}
}

IssuingIoWhileRunningLoops::~IssuingIoWhileRunningLoops() {

}


IssuingVariableIo::IssuingVariableIo() {

}
IssuingVariableIo::IssuingVariableIo(VexThreadState * _state, int _ioRequests) {
	state = _state;
	ioRequests = _ioRequests;

}

long IssuingVariableIo::getRandomIoDuration() {
	int selector = rand() % 14;
	if (selector < 4) {
		return 2000000;
	} else if (selector <6) {
		return 3000000;
	} else if (selector < 10) {
		return 5000000;
	} else if (selector < 11) {
		return 12000000;
	} else {
		return 35000000;
	}
}
/*
long IssuingVariableIo::getRandomIoDuration() {
		int selector = rand() % 105;
		if (selector < 40) {
			return 5000;
		} else if (selector <65) {
			return 30000;
		} else if (selector < 76) {
			return 100000;
		} else if (selector < 80) {
			return 200000;

		} else if (selector < 90) {
			return 500000;
		} else if (selector < 95) {
			return 1000000;
		} else if (selector < 98) {
			return 2000000;
		} else if (selector < 99) {
			return 10000000;
		} else {
			return 35000000;
		}

}
*/
long *IssuingVariableIo::requestsDuration;
long long IssuingVariableIo::maxIoDuration;
int IssuingVariableIo::predefined;
void IssuingVariableIo::createRandomRequests(int requestsNum) {
	srand(10);
	requestsDuration = new long[requestsNum];
	maxIoDuration = 0;
	IssuingVariableIo f(NULL,0);
	for (int i =0 ; i <requestsNum; i++) {
		requestsDuration[i] = f.getRandomIoDuration();
		if (requestsDuration[i] > maxIoDuration) {
			maxIoDuration = requestsDuration[i];
		}
//		cout << i << " " <<requestsDuration[i]<<endl;
	}
	predefined=0;
}

long long IssuingVariableIo::getMaxIoDuration() {
	return maxIoDuration;
}

void IssuingVariableIo::readRequestsFromFile(const char *filename) {
	srand(10);

	ifstream ioDurationsFile;
	ioDurationsFile.open(filename);

	maxIoDuration = 0;
	if (!ioDurationsFile.fail()) {
		char buffer[32];
		ioDurationsFile.getline(buffer,32);
		predefined=atoi(buffer);
		requestsDuration = new long[predefined];
		int count = 0;
		while (ioDurationsFile.getline(buffer,32)) {
			requestsDuration[count] = atoi(buffer);
			if (requestsDuration[count] > maxIoDuration) {
				maxIoDuration = requestsDuration[count];
			}
			++count;
		}
	} else {
		cout << "Failed to open file " << filename << endl;
	}

	ioDurationsFile.close();

//
//	for (int i =0 ; i< predefined; i++) {
//		cout << requestsDuration[i] << endl;
//	}
}

void IssuingVariableIo::apply() {

	struct timespec ioTime;
	ioTime.tv_sec = 0;

	state->setIoInvocationPointHashValue( 10);
	state->setStackTraceHash(2);

	for (int i = 0; i<ioRequests; i++) {

		if (predefined == 0) {
			ioTime.tv_nsec = requestsDuration[i];//getRandomIoDuration();
		} else {
			ioTime.tv_nsec = requestsDuration[rand() % predefined];
		}
		//ioTime.tv_nsec =  (long)(217680000 * duration);


		methodEventsBehaviour->afterIoMethodEntry(2, 10, false);
		nanosleep(&ioTime, NULL);
		methodEventsBehaviour->beforeIoMethodExit(2);
	}

}

IssuingVariableIo::~IssuingVariableIo() {

}





IssuingIoAndLoops::IssuingIoAndLoops(VexThreadState * _state, int _ioRequests) {
	state 		= _state;
	ioRequests 	= _ioRequests;
}

void IssuingIoAndLoops::apply() {

	struct timespec ioTime;
	ioTime.tv_sec = 0;

	float factor = 0.00001;
	for (int i = 0; i<ioRequests; i++) {

		factor = 0.00001 * (rand() % 100);
		busyCpu(factor);
		long long endingTime = Time::getVirtualTime();
		manager->setCurrentThreadVT(endingTime, state);

		if (predefined == 0) {
			ioTime.tv_nsec = requestsDuration[i];//getRandomIoDuration();
		} else {
			ioTime.tv_nsec = requestsDuration[rand() % predefined];
		}

		methodEventsBehaviour->afterIoMethodEntry(2, 10, false);
		nanosleep(&ioTime, NULL);
		methodEventsBehaviour->beforeIoMethodExit(2);

		factor = 0.00001 * (rand() % 100);
		busyCpu(factor);
		endingTime = Time::getVirtualTime();
		manager->setCurrentThreadVT(endingTime, state);
	}

}

IssuingIoAndLoops::~IssuingIoAndLoops() {

}



int Barrier::counter = 0;
pthread_mutex_t Barrier::lock;

pthread_cond_t Barrier::cond;

void Barrier::init() {
	pthread_mutex_init(&Barrier::lock, NULL);
	pthread_cond_init(&Barrier::cond, NULL);
}

Barrier::Barrier(VexThreadState * _state, int _loops, bool isTimedBarrier) {
	state = _state;
	loops = _loops;
	temp2 = 0;
	timedBarrier = isTimedBarrier;
}

void Barrier::apply() {

	long long start = 0;
	if (!AgentTester::isVTFenabled()) {
		start = Time::getRealTime();
	}

	if (!timedBarrier) {

		for (int i=0; i<loops; i++) {
			AgentTester::onThreadInteractionPointEncounter(state);

//			AgentTester::onThreadContendedEnter(state, &lock);
			pthread_mutex_lock(&lock);
//			AgentTester::onThreadContendedEntered(state);


			if (++counter == totalThreads) {
				counter = 0;
				AgentTester::notifyAllTimedWaitingThreads(state, &lock);
				pthread_cond_broadcast(&cond);

			} else {

				AgentTester::onThreadWaitingStart(state, -1);
				pthread_cond_wait(&cond, &lock);
				AgentTester::onThreadWaitingEnd(state);
			}

			AgentTester::onThreadInteractionPointEncounter(state);
			pthread_mutex_unlock(&lock);
		}
	} else {
		struct timeval tp;
		struct timespec ts;
		gettimeofday(&tp, NULL);
		ts.tv_sec = (24*3600) + tp.tv_sec;
		ts.tv_nsec = tp.tv_usec;

		for (int i=0; i<loops; i++) {
			AgentTester::onThreadInteractionPointEncounter(state);

			AgentTester::onThreadContendedEnter(state, &lock);
			pthread_mutex_lock(&lock);
			AgentTester::onThreadContendedEntered(state);

			if (++counter == totalThreads) {
				counter = 0;
				AgentTester::notifyAllTimedWaitingThreads(state, &lock);
				pthread_cond_broadcast(&cond);

			} else {
				if (AgentTester::isVTFenabled()) {
					AgentTester::interruptOnVirtualTimeout(state, &lock, &cond, 500000000);
				} else {
					pthread_cond_timedwait(&cond, &lock, &ts);
				}
			}

			AgentTester::onThreadInteractionPointEncounter(state);
			pthread_mutex_unlock(&lock);
		}



	}
	if (!AgentTester::isVTFenabled()) {
		long long end = Time::getRealTime();
		pthread_mutex_lock(&lock);
		cout << (end -start) << endl;
		pthread_mutex_unlock(&lock);
	}


}

Barrier::~Barrier() {

}



TimedWaiting::TimedWaiting(VexThreadState * _state, const long long &_totalThreadTime, bool _randomDurations) {
	state = _state;
	randomDurations = _randomDurations;
	totalThreadTime = _totalThreadTime;
}

void TimedWaiting::apply() {

	if (randomDurations) {
		long long threadTime = 0;
		long long nextWaitingTime;
		do {
			nextWaitingTime = rand() % (totalThreadTime/10);
			threadTime += nextWaitingTime;
			AgentTester::interruptOnVirtualTimeout(state, 0, 0, nextWaitingTime);

		} while(threadTime < totalThreadTime);

	} else {
		AgentTester::interruptOnVirtualTimeout(state, 0, 0, totalThreadTime);

	}
}

TimedWaiting::~TimedWaiting() {

}


ModelRunning::ModelRunning(VexThreadState *_state, const bool &_modelSchedulerSim) {
	state = _state;
	modelSchedulerSim = _modelSchedulerSim;
}

void ModelRunning::apply() {
	int methodId = 101;
	VEX::methodEventsBehaviour->registerMethodPerformanceModel(methodId, "/homes/nb605/VTF/src/models/waiting400_local_normal_spec_sunflow.jsimg", "Source 0", 0);
//	methodPerfomanceModels[methodId] = new MethodPerformanceModel("/homes/nb605/VTF/src/models/waiting400_local_normal_spec_sunflow.jsimg", modelSchedulerSim);
	VEX::methodEventsBehaviour->afterMethodEntryUsingPerformanceModel(methodId);
	double temp = 0.0;
	cout << "this is the first message " << endl;
	temp += busyCpu(1.0);
	cout << "this is the second message " << endl;
	temp += busyCpu(3.0);
	cout << "this is the third message " << temp << endl;
	VEX::methodEventsBehaviour->beforeMethodExitUsingPerformanceModel(methodId);
}

ModelRunning::~ModelRunning() {

}
