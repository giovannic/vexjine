/*
 * IoRecognizer.h
 *
 *  Created on: 16 Aug 2011
 *      Author: root
 */

#ifndef IORECOGNIZER_H_
#define IORECOGNIZER_H_

#include "ThreadState.h"

class IoRecognizer {
public:
	IoRecognizer();
	virtual ~IoRecognizer();

	virtual bool isRecognizedAsIoOperation(VexThreadState *state, const long long &ioDuration) = 0;
};



/**
 * All read/write/etc operations are considered I/O
 */
class PremiscuousRecognizer : public IoRecognizer {
public:
	PremiscuousRecognizer();
	virtual ~PremiscuousRecognizer();

	bool isRecognizedAsIoOperation(VexThreadState *state, const long long &ioDuration);
};


/**
 * Decide if the read/write/etc operation is indeed I/O, based on its duration and a predefined threshold
 */
class EmpiricalRecognizer : public IoRecognizer {
public:
	EmpiricalRecognizer();
	EmpiricalRecognizer(const long long &_threshold);
	virtual ~EmpiricalRecognizer();

	bool isRecognizedAsIoOperation(VexThreadState *state, const long long &ioDuration);

private:
	long long threshold;
};



/**
 * Randomly exclude some I/O operations from real time measurements
 */
class IoExcluder {
public:
	IoExcluder();
	IoExcluder(const int &percent);
	IoExcluder(const float &_acceptancePercentage);
	virtual ~IoExcluder();

	virtual bool shouldRandomlyExcludeIo();

protected:
	float acceptancePercentage;
};

class AllIoExcluder : public IoExcluder {
public:
	virtual ~AllIoExcluder();
	virtual bool shouldRandomlyExcludeIo();

};
#endif /* IORECOGNIZER_H_ */
