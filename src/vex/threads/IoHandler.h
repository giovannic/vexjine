/*
 * IoHandler.h
 *
 *  Created on: 21 Apr 2011
 *      Author: root
 */

#ifndef IOHANDLER_H_
#define IOHANDLER_H_

class PredictionMethod;

class IoHandler {
public:
	IoHandler();
	virtual ~IoHandler();

	inline bool invalidIoInvocationPoint(const int &invocationPointHashValue) {
		return (stackDepth == 0 && invocationPointHashValue != 1);
	};
	inline bool invalidIoInvocationPoint() {
		return (stackDepth == 0 && ioInvocationPointHashValue != 1);
	};

	inline unsigned int getStackDepth() {
		return stackDepth;
	};
	inline void addPredictionToEstimatedRealTime() {
		timesIoPredictionWasAddedToEstimatedTime = 1;
	};

	PredictionMethod *getLastIoPredictionMethod() {
		return lastIoPredictionMethod;
	}

	void ignoreNextIo() {
		ignoringIo = true;
	}

	void recognizedCallAsCached() {
		recognizingIoAsCached = true;
	}
	bool callRecognizedAsCached() {
		return recognizingIoAsCached;
	}

	void resetIoStateFlags() {
		recognizingIoAsCached = false;
		ignoringIo = false;
	}

	bool isIgnoringIo() {
		return ignoringIo;
	}
	void enteringMethod(const int &methodId) {
		++stackDepth;	// Used to identify exiting methods
		stackTraceHash += methodId;
	};
	void exitingMethod(const int &methodId) {
		--stackDepth;	// Used to identify exiting methods
		stackTraceHash -= methodId;
	};
	inline long long const & getLastIoPrediction() const {
		return lastIoPrediction;
	};
	inline void setLastIoPrediction(const long long & _lastIoPrediction) {
		lastIoPrediction = _lastIoPrediction;
	};

	inline void setIoInvocationPointHashValue(const int &_ioInvocationPointHashValue) {
		ioInvocationPointHashValue = _ioInvocationPointHashValue;
	};

	inline int getIoInvocationPointHashValue() {
		return ioInvocationPointHashValue;
	};
	inline int getStackTraceHash() {
		return stackTraceHash;
	};
	void setIoPredictionInfo(PredictionMethod *_lastIoPredictionMethod, const long long &_lastIoPrediction) {
		lastIoPredictionMethod = _lastIoPredictionMethod;
		lastIoPrediction = _lastIoPrediction;
	}
	inline void setStackTraceHash(const int &_stackTraceHash) {
		stackTraceHash = _stackTraceHash;
	};

	inline void notifyAboutPossiblyBlockingIo(const bool &_possiblyBlockingIo) {
		possiblyBlockingIo = _possiblyBlockingIo;
	}

	inline bool const & getIoFinishedBeforeLogging() const {
		return ioFinishedBeforeLogging;
	};
	void setIoCPUTime(long long _ioCpuTime) {
		ioCpuTime = _ioCpuTime;
	}
	long long getIoCPUTime() {
		return ioCpuTime;
	}
	inline void setIoFinishedBeforeLogging(const bool & _ioFinishedBeforeLogging) {
		ioFinishedBeforeLogging = _ioFinishedBeforeLogging;
	};

	inline bool executingPossiblyBlockingIo() {
		return possiblyBlockingIo;
	}

	long long extendPredictionPeriod() {
//		assert(timesIoPredictionWasAddedToEstimatedTime != 0);	// in case of overflow for the unsigned long value...

		timesIoPredictionWasAddedToEstimatedTime *= 2;
		return timesIoPredictionWasAddedToEstimatedTime * lastIoPrediction;
	};

//	inline int const & getTotalThreadsInIO() const {
//		return totalThreadsInIO;
//	};
	long long getTotalTimePredicted() {
		return ((2*timesIoPredictionWasAddedToEstimatedTime) - 1) * lastIoPrediction;
	}

protected:

	unsigned long timesIoPredictionWasAddedToEstimatedTime;
	unsigned int stackDepth;		//counts the stackDepth of monitored I/O methods
	int ioInvocationPointHashValue;
	bool ioFinishedBeforeLogging;
	long long lastIoPrediction;
	int stackTraceHash;					// keep a number (sum of all method ids of the stack trace) that describes your stack trace
	bool possiblyBlockingIo;
	bool ignoringIo;
	bool recognizingIoAsCached;
	long long ioCpuTime;
	PredictionMethod *lastIoPredictionMethod;
};

#endif /* IOHANDLER_H_ */
