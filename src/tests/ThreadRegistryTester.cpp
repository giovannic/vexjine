/*
 * ThreadRegistryTester.cpp
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#include "ThreadRegistryTester.h"

ThreadRegistryTester::ThreadRegistryTester() {
	

}

ThreadRegistryTester::~ThreadRegistryTester() {
	 
}


string ThreadRegistryTester::testName() {
	return "Thread registry add/retrieve";
}

bool ThreadRegistryTester::test() {
	return registryAddAndRetrieve();
}

bool ThreadRegistryTester::registryAddAndRetrieve() {
	VexThreadState *pts = new VexThreadState();
	ThreadRegistry *reg = new ThreadRegistry(1);

//	cout << "Hello testing world by " << pts->name <<" "<< pts->getUniqueId() << endl;
	assert(pts->getUniqueId() != 0);

	reg->add(pts);
	reg->printThreadStates(-1, false);


	VexThreadState *ptr;
	long tid;

	tid = pts->getUniqueId() +1;
	ptr = (VexThreadState *)reg->getCurrentThreadState(tid);
	assert (ptr == NULL);

	tid = pts->getUniqueId();
	ptr = (VexThreadState *)reg->getCurrentThreadState(tid);
	assert (ptr != NULL);

	reg->remove(pts);
	delete pts;
	pts = NULL;

	ptr = (VexThreadState *)reg->getCurrentThreadState(tid);
	assert(ptr == NULL);

	delete reg;
	return true;
}
