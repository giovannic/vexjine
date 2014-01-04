/*
 * PerformanceTest.cpp
 *
 *  Created on: 12 Nov 2011
 *      Author: root
 */

#include "Queueing.h"

#include "ThreadState.h"

#include <sstream>

void *serverRoutine(void *arg) {
	if(AgentTester::virtualExecution) {
		long tid = gettid();
		threadEventsBehaviour->onStart(tid, "Server");
	}
	Server *mm1Server = (Server *)arg;
	if (mm1Server != NULL) {
		mm1Server->start("Server");
	}

	if(AgentTester::virtualExecution) {
		threadEventsBehaviour->onEnd();
	}
	return NULL;
}

void *clientRoutine(void *arg) {
	if(AgentTester::virtualExecution) {
		long tid = gettid();
		stringstream str;
		str << "client-" << gettid();
		threadEventsBehaviour->onStart(tid, str.str().c_str());
	}

	Client *mm1Client = (Client *)arg;
	if (mm1Client != NULL) {
		mm1Client->makeRequest();
	}
	if(AgentTester::virtualExecution) {
		threadEventsBehaviour->onEnd();
	}
	return NULL;
}

int main(int argc, char **argv) {

	if (argc != 2 && argc != 3) {
		cout << "Syntax: ./queue <server_rate> [enable VEX]" << endl;
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


	ThreadSafeQueue *queue = new ThreadSafeQueue();
	Server *mm1Server = new Server(atoi(argv[1]), queue);
	Client *mm1Client = new Client(400.0, queue);
	pthread_create(new pthread_t, NULL, serverRoutine, (void *)mm1Server);

    int request = 0;
    pthread_t *requestingThreads = new pthread_t[400];
    while (request < 400) {
    	pthread_create(&requestingThreads[request], NULL, clientRoutine, (void *)mm1Client);
        mm1Client->think();
        request++;
    }

    int ret;
    for (int i =0; i<400; i++) {
    	pthread_join(requestingThreads[i], (void **)&ret);
    }

	if(AgentTester::virtualExecution) {
		methodEventsBehaviour->beforeMethodExit(100);
	}

	return 0;
}
