/*
 * Class used to store which objects have their monitors being waited on.
 * This is used for Java threads simulation
 */

#include "ObjectRegistry.h"

#include <cassert>
#include <iostream>
using namespace std;

ObjectRegistry::ObjectRegistry() {
	registry = new unordered_map<int, ObjectRegistryInfo *>;
	pthread_mutex_init(&mutex, NULL);
}

void ObjectRegistry::lockMutex() {
	pthread_mutex_lock(&mutex);
}

void ObjectRegistry::unlockMutex() {
	pthread_mutex_unlock(&mutex);
}

/*
 * Return only one of the timed waiting threads on that object's monitor
 */
long ObjectRegistry::findNext(const int &objectId) {
	unordered_map<int, ObjectRegistryInfo *>::iterator registry_iter;
	registry_iter = registry -> find(objectId);

	if (registry_iter != registry -> end()) {
		ObjectRegistryInfo *objectRegistryInfo = (ObjectRegistryInfo *)registry_iter -> second;
		vector<long> *objectThreadList = objectRegistryInfo -> getThreadsWaiting();

		if (objectThreadList -> empty()) {
			return 0;
		} else {
			vector<long>::iterator vect_iter = objectThreadList -> end() ;
			--vect_iter;
			long value = *vect_iter;
			return value;
		}
	}

	return 0;

}

long ObjectRegistry::getNext(const int &objectId) {
	unordered_map<int, ObjectRegistryInfo *>::iterator registry_iter;
	registry_iter = registry -> find(objectId);

	if (registry_iter != registry -> end()) {
		ObjectRegistryInfo *objectRegistryInfo = (ObjectRegistryInfo *)registry_iter -> second;
		vector<long> *objectThreadList = objectRegistryInfo -> getThreadsWaiting();

		if (objectThreadList -> empty()) {
			return 0;
		} else {
			vector<long>::iterator vect_iter = objectThreadList -> end() ;
			--vect_iter;
			long value = *vect_iter;
			objectThreadList -> pop_back();
			return value;

		}
	}

	return 0;

}

/*
 * Erase a thread waiting record from this object monitor
 */
//void ObjectRegistry::erase(const int &objectId, vector<long>::iterator threadRecord) {
void ObjectRegistry::erase(const int &objectId, const long &threadId) {
	unordered_map<int, ObjectRegistryInfo *>::iterator registry_iter;
	registry_iter = registry -> find(objectId);

	if (registry_iter != registry -> end()) {
		ObjectRegistryInfo *objectRegistryInfo = (ObjectRegistryInfo *)registry_iter -> second;
		vector<long> *objectThreadList = objectRegistryInfo -> getThreadsWaiting();

		if (objectThreadList -> empty()) {
			return;
		} else {

			vector<long>::iterator threadRecord = objectThreadList -> begin();
			while (threadRecord != objectThreadList->end()) {
				if (threadId == *threadRecord) {
					objectThreadList -> erase(threadRecord);
					return;
				}
				++threadRecord;
			}

		}
	}

}

/*
 * Add a new thread waiting on this object monitor
 */
void ObjectRegistry::insert(const int &objectId, const long &threadId) {
	unordered_map<int, ObjectRegistryInfo *>::iterator registry_iter;
	registry_iter = registry -> find(objectId);

	ObjectRegistryInfo *objectRegistryInfo;
	if (registry_iter != registry -> end()) {
		objectRegistryInfo = (ObjectRegistryInfo *)registry_iter -> second;
		vector<long> *objectThreadList = objectRegistryInfo -> getThreadsWaiting();

		vector<long>::iterator vector_iter = objectThreadList -> end();
		objectThreadList -> insert(vector_iter, threadId);
		//objectThreadList -> push_back(threadId);

	} else {
		objectRegistryInfo = new ObjectRegistryInfo();
		vector<long> *objectThreadList = objectRegistryInfo -> getThreadsWaiting();

		vector<long>::iterator vector_iter = objectThreadList -> end();
		vector_iter = objectThreadList -> insert(vector_iter, threadId);
		(*registry)[objectId] = objectRegistryInfo;
	//	return vector_iter;
	}

	objectRegistryInfo -> releaseMonitor();

}


/*
 * Print object registry contents
 */
void ObjectRegistry::print() {
	unordered_map<int, ObjectRegistryInfo *>::iterator registry_iter;
	vector<long>::iterator threadRecord;


	registry_iter = registry -> begin();
	cout << "Object Registry Printing" << endl;
	cout << "========================" << endl;
	while (registry_iter != registry-> end()) {

		cout << "Object: " << registry_iter->first << endl;

		ObjectRegistryInfo *objectRegistryInfo = (ObjectRegistryInfo *)registry_iter -> second;
		vector<long> *objectThreadList = objectRegistryInfo -> getThreadsWaiting();

		threadRecord = objectThreadList -> begin();
		if (!objectThreadList ->empty()) {
			cout << "Threads waiting: ";
			while (threadRecord != objectThreadList->end()) {
				cout << *threadRecord << " ";
				++threadRecord;
			}
		} else {
			cout << "No threads waiting";
		}
		cout << endl << endl;;
		++registry_iter;

	}
}

/*
 * Check whether anyone is waiting on this monitor
 */
bool ObjectRegistry::empty(const int &objectId) {
	unordered_map<int, ObjectRegistryInfo *>::iterator registry_iter;
	registry_iter = registry -> find(objectId);

	if (registry_iter != registry -> end()) {

		ObjectRegistryInfo *objectRegistryInfo = (ObjectRegistryInfo *)registry_iter->second;
		assert(objectRegistryInfo != NULL);		// this always exists as it created once and left there
		vector<long> *objectThreadList = objectRegistryInfo -> getThreadsWaiting();

		if (objectThreadList -> empty()) {
			return true;
		} else {
			return false;
		}
	} else {
		return true;
	}
}


/*
 * Check whether anyone is waiting on this monitor
 */
bool ObjectRegistry::isObjectFree(const int &objectId) {
	unordered_map<int, ObjectRegistryInfo *>::iterator registry_iter;
	registry_iter = registry -> find(objectId);

	ObjectRegistryInfo *objectRegistryInfo;
	if (registry_iter != registry -> end()) {
		// this always exists as it created once and left there
		objectRegistryInfo = (ObjectRegistryInfo *)registry_iter->second;
		assert(objectRegistryInfo != NULL);
	} else {
		objectRegistryInfo = new ObjectRegistryInfo();
		(*registry)[objectId] = objectRegistryInfo;

	}
	return objectRegistryInfo -> isMonitorFree();
}

ObjectRegistryInfo *ObjectRegistry::getObjectRegistryInfo(const int &objectId) {
	unordered_map<int, ObjectRegistryInfo *>::iterator registry_iter;
	registry_iter = registry -> find(objectId);

	ObjectRegistryInfo *objectRegistryInfo;
	if (registry_iter != registry -> end()) {
		// this always exists as it created once and left there
		objectRegistryInfo = (ObjectRegistryInfo *)registry_iter->second;
		assert(objectRegistryInfo != NULL);
	} else {
		objectRegistryInfo = new ObjectRegistryInfo();
		(*registry)[objectId] = objectRegistryInfo;

	}
	return objectRegistryInfo;
}

ObjectRegistry::~ObjectRegistry() {
	delete registry;
	pthread_mutex_destroy(&mutex);
}
