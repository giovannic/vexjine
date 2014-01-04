#ifndef OBJECTREGISTRY_H_
#define OBJECTREGISTRY_H_

#include <tr1/unordered_map>
#include <vector>
#include <pthread.h>

using namespace std;
using namespace tr1;

class ObjectRegistryInfo {
public:
	ObjectRegistryInfo() {
		threadsWaiting = new vector<long>;
		monitorFree = true;
	}

	void enterMonitor() {
		monitorFree = false;
	}
	void releaseMonitor() {
		monitorFree = true;
	}

	bool isMonitorFree() {
		return monitorFree;
	}

	vector<long> *getThreadsWaiting() {
		return threadsWaiting;
	}

	~ObjectRegistryInfo() {
		delete threadsWaiting;
	}

private:
	vector<long> *threadsWaiting;
	bool monitorFree;
};




class ObjectRegistry {
public:
	ObjectRegistry();

	void lockMutex();
	void unlockMutex();

	void insert(const int &objectId, const long &threadId);

	bool empty(const int &objectId);
	long findNext(const int &objectId);	// return only one of the timed waiting threads to be interrupted
	//void erase(const int &objectId, vector<long>::iterator threadRecord);
	void erase(const int &objectId, const long& threadId);
	void print();
	long getNext(const int &objectId);

	bool isObjectFree(const int &objectId);
	ObjectRegistryInfo *getObjectRegistryInfo(const int &objectId);

	~ObjectRegistry();

private:
	// Registry: object ID <-> array of thread IDs (timed) waiting on that object monitor
	unordered_map<int, ObjectRegistryInfo *> *registry;

	pthread_mutex_t mutex;

};

#endif /*OBJECTREGISTRY_H_*/

