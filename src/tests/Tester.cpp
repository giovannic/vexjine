/*
 * Tester.cpp
 *
 *  Created on: 6 Jul 2010
 *      Author: nb605
 */

#include "Tester.h"

//#include "ThreadState.h"
//#include "VirtualTimeline.h"

#include <pthread.h>
#include <cassert>
bool Tester::virtualExecution = true;


using namespace VEX;
Tester::Tester() {
	

}

void Tester::enableVTF() {
	virtualExecution = true;
}
void Tester::disableVTF() {
	virtualExecution = false;
}

bool Tester::isVTFenabled() {
	return virtualExecution;
}

Tester::~Tester() {
	 
}

void Tester::spawnThreadsToExecute(int _threads, void *(*threadRoutine)(void *)) {

	pthread_t *threads = new pthread_t[_threads];
	int *ids = new int[_threads];
	for(int i = 0 ; i< _threads; i++) {
		ids[i] =  i;
	}

	if (Tester::virtualExecution) {
		for(int i =0 ; i< _threads; i++) {
			VEX::threadEventsBehaviour->beforeCreatingThread(ids[i]);
			//registry->newThreadBeingSpawned();
		}
	}

	for(int i = 0 ; i< _threads; i++) {
		pthread_create(&threads[i], NULL, threadRoutine, &ids[i]);
	}
	for(int i =0 ; i< _threads; i++) {
		pthread_join(threads[i], NULL);
	}

	delete[] threads;
	delete[] ids;
}

void *managerStart(void *Args) {
	// TODO: removed - should not call thread managers directly -use VTF interface
	assert(false);
	//manager->start();
	return NULL;
}

void Tester::startVTFManager() {
	if (Tester::virtualExecution) {
		pthread_create(&manager_thread, NULL, managerStart, NULL);
	}
//	pthread_join(manager_thread, NULL);
}

void Tester::endVTFManager() {
	if (Tester::virtualExecution) {
		// TODO: removed - should not call thread managers directly -use VTF interface

		assert(false);
//		manager->end();
		//endSimulator();
	}
//	pthread_kill(manager_thread, 123);
}

//int main(int argc, char **argv) {
//	cout << endl<<endl<<"New test starting" << endl << "--------------------" << endl;
//	Tester *t = new Tester();
////	t -> registryAddAndRetrieve();
////	t -> optionsParsing();
////	t -> methodLogging();
//
//	int threads = 2;
//	long timeslot = 10000000;
//
//	if (argc != 1) {
//		//timeslot = atol(argv[1]);
//		threads = atoi(argv[1]);
//		globalMethodPerThread = atoi(argv[2]);
//		globalStackDepthPerThread = atoi(argv[3]);
//	}
//
//	pthread_spin_init(&lock, 0);
//	if (argc > 4) {
//		Tester::virtualExecution = false;
//	}
//	t -> testThreadLifeCycle(threads, timeslot);
//	return 0;
//}
