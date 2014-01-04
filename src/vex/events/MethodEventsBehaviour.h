/*
 * MethodEventsBehaviour.h: Class to define the behaviour of actions upon method entry/exit points.
 * These include profiling, I/O simulation and model simulation techniques.
 *
 *  Created on: 18 Oct 2011
 *      Author: root
 */

#ifndef METHODEVENTSBEHAVIOUR_H_
#define METHODEVENTSBEHAVIOUR_H_

#include <tr1/unordered_map>
#include <string>

// Forward declarations
class ProfilingInvalidationPolicy;
class VirtualTimeline;
class IoSimulator;
class VirtualTimeline;
class MethodPerformanceModel;
class MethodPerformanceModelMetadata;

class MethodEventsBehaviour {
public:
	MethodEventsBehaviour(IoSimulator *ioSimulator, const bool &_reportIoMethods, VirtualTimeline *_virtualTimeline);
	virtual ~MethodEventsBehaviour();

	bool afterMethodEntry(const int & methodId);
	bool beforeMethodExit(const int & methodId);

	void afterIoMethodEntry(const int &methodId, const int &invocationPointHashValue, const bool &possiblyBlocking);
	void beforeIoMethodExit(const int & methodId);

	long long afterMethodEntryUsingPerformanceModel(const int &methodId);
	void beforeMethodExitUsingPerformanceModel(const int &methodId);

	void registerInvalidationPolicy(const char *invalidationPolicyInfo);
	void registerMethodPerformanceModel(const int &methodId, const char *modelFilename, const char *sourceNodeLabel, const int &customerClass);
	void registerMethodTimeScalingFactor(const int &methodId, const double &speedup);

protected:

	const float &getTimeScalingFactorOfMethod(const int &methodId);
	std::tr1::unordered_map<std::string, MethodPerformanceModel *> modelFilenamesToPerfomanceModels;
	std::tr1::unordered_map<int, std::pair<MethodPerformanceModelMetadata *, MethodPerformanceModel *> > methodPerfomanceModels;
	std::tr1::unordered_map<int, float> methodTimeScalingFactors;

	ProfilingInvalidationPolicy *invalidationPolicy;
	IoSimulator *ioSimulator;

	bool reportIoMethods;
	float defaultTimeScalingFactor;
	VirtualTimeline *virtualTimeline;
};

#endif /* METHODEVENTSBEHAVIOUR_H_ */
