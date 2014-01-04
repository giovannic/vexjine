/*
 * PerformanceTest.cpp
 *
 *  Created on: 12 Nov 2011
 *      Author: root
 */
#include "AgentTester.h"

#include <sstream>
#include <algorithm>

#include "ThreadState.h"

static pthread_mutex_t mutex;

void *serverRoutine(void *arg) {
	long tid = gettid();
	stringstream str("");

	if(AgentTester::virtualExecution) {
		str << "server-" << gettid();
		threadEventsBehaviour->onStart(tid, str.str().c_str());
	}

	double temp = 1.0;

	if(AgentTester::virtualExecution) {
		threadEventsBehaviour->onRequestingLock(&mutex);
	}
	pthread_mutex_lock(&mutex);
	int iterations = rand()% 10000000;
	cout << str.str() << " will execute " << iterations << " iterations" << endl;
	for (double i = 1; i < iterations ; i++) {
		temp += pow(1.0/i, i);
	}

	cout << "result: " << temp << endl;

	if(AgentTester::virtualExecution) {
		threadEventsBehaviour->onReleasingLock(&mutex);
	}
	pthread_mutex_unlock(&mutex);

	if(AgentTester::virtualExecution) {
		threadEventsBehaviour->onEnd();
	}

	return NULL;
}

void *clientRoutine(void *arg) {
	long tid = gettid();
	stringstream str("");

	if(AgentTester::virtualExecution) {
		str << "client-" << gettid();
		threadEventsBehaviour->onStart(tid, str.str().c_str());
	}

	double temp = 1.0;
	if(AgentTester::virtualExecution) {
		threadEventsBehaviour->onRequestingLock(&mutex);
	}
	pthread_mutex_lock(&mutex);

	int iterations = rand() % 1000000;
	cout << str.str() << " will execute " << iterations << " iterations" << endl;
	for (double i = 1; i < iterations ; i++) {
		temp += pow(1.0/i, i);
	}

	cout << "result: " << temp << endl;


	if(AgentTester::virtualExecution) {
		threadEventsBehaviour->onReleasingLock(&mutex);
	}
	pthread_mutex_unlock(&mutex);

	if(AgentTester::virtualExecution) {
		threadEventsBehaviour->onEnd();
	}

	return NULL;
}

int main(int argc, char **argv) {

	if (argc != 2 && argc != 3) {
		cout << "Validate locking: spawn 2threads, each of which performs a random number of iterations whilst holding a mutex" << endl;
		cout << "Syntax: ./vlock <threads> [enable VEX]" << endl;
		return 1;
	}

	if (argc == 3) {
		AgentTester::virtualExecution = true;
	} else {
		AgentTester::virtualExecution = false;
	}

	if(AgentTester::virtualExecution) {
		if (!initializeSimulator(argv[2])) {
			cout << "Failed to initialize VEX simulator with arguments: " << argv[2] << endl;
			return 1;
		}
		methodEventsBehaviour->afterMethodEntry(100);
	}
	pthread_mutex_init(&mutex, NULL);

	int totalRequests = atoi(argv[1]);
    int request = 0;
    pthread_t *requestingThreads = new pthread_t[totalRequests];
    int *requestsTable = new int[totalRequests];
    std::fill(requestsTable, requestsTable + totalRequests, 0);

    while (request < totalRequests) {
    	if (request % 2 == 0) {
    		pthread_create(&requestingThreads[request], NULL, clientRoutine, (void *)&requestsTable[request]);
    	} else {
    		pthread_create(&requestingThreads[request], NULL, serverRoutine, (void *)&requestsTable[request]);
    	}
        request++;
    }

    void *status;
    for (int i =0; i<totalRequests; i++) {
    	pthread_join(requestingThreads[i], &status);

    }

	if(AgentTester::virtualExecution) {
		methodEventsBehaviour->beforeMethodExit(100);
	}

	return 0;
}
