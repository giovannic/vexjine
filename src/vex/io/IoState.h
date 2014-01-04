/*
 * IoState.h
 *
 *  Created on: 24 Aug 2011
 *      Author: root
 */

#ifndef IOSTATE_H_
#define IOSTATE_H_
#include <sstream>

class IoState {
public:
	IoState();
	IoState(const int &_methodId) : threadsInIo(0), traceHash(0), ioOperation(_methodId) {}
	IoState(const unsigned int &_threadsInIo, const int &_traceHash) : threadsInIo(_threadsInIo), traceHash(_traceHash), ioOperation(0) {}
	IoState(const unsigned int &_threadsInIo, const int &_traceHash, const int &_methodId) : threadsInIo(_threadsInIo), traceHash(_traceHash), ioOperation(_methodId) {}

	virtual ~IoState();

	unsigned int &getThreadsInIO(const unsigned int &maxNumberOfThreadsPerformingIoInParallel) {
		if (threadsInIo >= maxNumberOfThreadsPerformingIoInParallel) {
			threadsInIo = maxNumberOfThreadsPerformingIoInParallel-1;
		}
		return threadsInIo;
	}

	int &getTraceHash() {
		return traceHash;
	}

	int const &getIoOperation() {
		return ioOperation;
	}

	void setIoOperation(const int &io) {
		ioOperation = io;
	}

	void setThreadsInIo(const int &tii) {
		threadsInIo = tii;
	}

	void getIoPointFileSuffix(std::stringstream &str);
private:
	unsigned int threadsInIo;
	int traceHash;
	int ioOperation;
};

#endif /* IOSTATE_H_ */
