
/*
 * ProfilingInvalidationPolicy.h
 *
 *  Created on: Jun 01, 2010
 *      Author: Nick
 */

#ifndef PROFILINGINVALIDATIONPOLICY_H_
#define PROFILINGINVALIDATIONPOLICY_H_

#include "PerformanceMeasure.h"
#include "VirtualTimeline.h"
#include <climits>
using namespace std;

/*
 * Class used to define the invalidation policy of method profiling, i.e. when the profiling of
 * an initially profiled method should be stopped.
 *
 * Current policy is only based on number of samples - it could be extended to other criteria
 */
class ProfilingInvalidationPolicy {
	
public:
	ProfilingInvalidationPolicy() : samplesLimit(INT_MAX) {};
	ProfilingInvalidationPolicy(const unsigned int &maxSamples) : samplesLimit(maxSamples) {};

	virtual ~ProfilingInvalidationPolicy();

	virtual bool keepOnProfiling(PerformanceMeasure *pm);
	virtual void printInfo(ostream &out);
protected:
	unsigned int samplesLimit;

};


/***
 * If the aggregate time of a method is less than a defined percentage of the total execution
 * time after samplesLimit samples, the profiling of the method will be invalidated
 */
class RelativePercentageProfilingInvalidationPolicy : public ProfilingInvalidationPolicy {

public:

	RelativePercentageProfilingInvalidationPolicy(const unsigned int &maxSamples, const float &percentage, VirtualTimeline *_virtualTimeline);

	~RelativePercentageProfilingInvalidationPolicy();

	virtual bool keepOnProfiling(PerformanceMeasure *pm);
	virtual void printInfo(ostream &out);
private:
	float invalidationAggregationPercentage;
	VirtualTimeline *virtualTimeline;
};


/***
 * If the aggregate time of a method is less than a defined percentage of the total execution
 * time after samplesLimit samples, the profiling of the method will be invalidated
 */
class AbsoluteTimeProfilingInvalidationPolicy : public ProfilingInvalidationPolicy {

public:

	AbsoluteTimeProfilingInvalidationPolicy(const unsigned int &maxSamples, const long long &_minimumAcceptableTime);

	~AbsoluteTimeProfilingInvalidationPolicy();

	virtual bool keepOnProfiling(PerformanceMeasure *pm);
	virtual void printInfo(ostream &out);
private:
	long long minimumAcceptableTime;
};

#endif /* PROFILINGINVALIDATIONPOLICY_H_ */
