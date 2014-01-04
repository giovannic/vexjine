/*
 * MethodData.h
 *
 *  Created on: 1 Mar 2011
 *      Author: root
 */

#ifndef METHODDATA_H_
#define METHODDATA_H_

#include <string.h>
#include <pthread.h>

class MethodData {
public:
	MethodData(const char *_name, int _methodId) {
		methodIdOfFirstInstrumentedMethodOfClass = 0;
		init(_name, _methodId);
	};

	MethodData(const char *_name, int _methodId, int _methodIdOfFirstInstrumentedMethodOfClass) {
		methodIdOfFirstInstrumentedMethodOfClass = _methodIdOfFirstInstrumentedMethodOfClass;
		init(_name, _methodId);
	};

	void init(const char *_name, const int &_methodId) {
		name 		= new char[strlen(_name)+1];
		strcpy(name, _name);
		methodId 	= _methodId;
		invalidated = false;
		recursive   = false;
//		threadsCurrentlyExecutingMethod = 0;
//		pthread_spin_init(&spinlock, 0);
	};

	virtual ~MethodData();

	int getMethodIdOfFirstInstrumentedMethodOfClass() {
		return methodIdOfFirstInstrumentedMethodOfClass;
	};

	char *getName() {
		return name;
	};

	void setInvalidated() {
		invalidated = true;
	};

	bool isInvalidated() {
		return invalidated;
	};


	void setRecursive() {
		recursive = true;
	};

	bool isRecursive() {
		return recursive;
	};

//	void increaseThreadsExecutingMethodCounter() {
//		blockNewThreadsEnteringMethod();
//		++threadsCurrentlyExecutingMethod;
//		allowNewThreadsToEnterMethod();
//	};

//	unsigned int getGlobalEntryCount() {
//		unsigned int temp = 0;
//		blockNewThreadsEnteringMethod();
//		temp = threadsCurrentlyExecutingMethod;
//		allowNewThreadsToEnterMethod();
//		return temp;
//	}
//	void decreaseThreadsExecutingMethodCounter() {
//		blockNewThreadsEnteringMethod();
//		if (threadsCurrentlyExecutingMethod > 0) {
//			--threadsCurrentlyExecutingMethod;
//		}
//		allowNewThreadsToEnterMethod();
//	};

//	bool areAnyThreadsExecutingMethod() {
//		return (threadsCurrentlyExecutingMethod != 0);
//	};
//	bool areMoreThanOneThreadsExecutingMethod() {
//		return threadsCurrentlyExecutingMethod > 1;
//	}
//	void blockNewThreadsEnteringMethod() {
//		pthread_spin_lock(&spinlock);
//	};
//	void allowNewThreadsToEnterMethod() {
//		pthread_spin_unlock(&spinlock);
//	};
private:
	char *name;
	int methodId;
	int methodIdOfFirstInstrumentedMethodOfClass;

	bool invalidated;
	bool recursive;

//	pthread_spinlock_t spinlock;
//	unsigned int threadsCurrentlyExecutingMethod;	// used for adaptive profiling invalidation

};

#endif /* METHODDATA_H_ */
