/*
 * ProfilingInvalidationPolicy.cpp for Java applications executed on the VTF
 *
 *  Created on: Jun 01, 2010
 *      Author: Nick
 *
 */

#include "ProfilingInvalidationPolicy.h"
#include "PerformanceMeasure.h"
#include "VirtualTimeline.h"
#include "Time.h"
#include <string.h>

ProfilingInvalidationPolicy::~ProfilingInvalidationPolicy() {

}

bool ProfilingInvalidationPolicy::keepOnProfiling(PerformanceMeasure *pm) {

	if (pm != NULL && pm->getMethodInvocations() > samplesLimit && !pm->isTheInstrumentationProfilingMethod()) {
		//fprintf(stderr, "Invalidating method %d\n", pm->getMethodId());fflush(stderr);
		return false;
	}
	return true;
}

void ProfilingInvalidationPolicy::printInfo(ostream &out) {
	out << "Invalidation policy: Sample-based - invalidating profiling of methods with more than " << samplesLimit << " samples" <<endl;
}


RelativePercentageProfilingInvalidationPolicy::RelativePercentageProfilingInvalidationPolicy(const unsigned int &maxSamples, const float &percentage, VirtualTimeline *_virtualTimeline) {
	samplesLimit = maxSamples;
	invalidationAggregationPercentage = percentage;
	virtualTimeline = _virtualTimeline;
}

RelativePercentageProfilingInvalidationPolicy::~RelativePercentageProfilingInvalidationPolicy() {

}

bool RelativePercentageProfilingInvalidationPolicy::keepOnProfiling(PerformanceMeasure *pm) {

	if (pm != NULL && pm->getMethodInvocations() > samplesLimit && !pm->isTheInstrumentationProfilingMethod()) {

//		cout << pm->getMethodId() << " " << fixed << setprecision(9) << ((float)pm->getEstimatedRealTimeIncludingCalleeMethods()/(float)virtualTimelineController->getGlobalTime()) << " " << pm->getEstimatedRealTimeIncludingCalleeMethods() << " " << virtualTimelineController->getGlobalTime() << endl;
		if (pm->getEstimatedRealTimeIncludingCalleeMethods() < (invalidationAggregationPercentage * virtualTimeline->getGlobalTime())) {	// use the cache version of the GVT instead of updating from the VEX server (in the distributed version)
			//fprintf(stderr, "Invalidating method %d\n", pm->getMethodId());fflush(stderr);
			return false;
		} else {
			return true;
		}

	}
	return true;
}

void RelativePercentageProfilingInvalidationPolicy::printInfo(ostream &out) {
	out << "Invalidation policy: Relative time-based - invalidating profiling of methods whose aggregate estimated time after " << samplesLimit << " samples is less than " << (100*invalidationAggregationPercentage) << "% of the global time at that point"<<endl;
}

AbsoluteTimeProfilingInvalidationPolicy::AbsoluteTimeProfilingInvalidationPolicy(const unsigned int &maxSamples, const long long &_minimumAcceptableTime) {
	samplesLimit = maxSamples;
	minimumAcceptableTime = _minimumAcceptableTime;
}

AbsoluteTimeProfilingInvalidationPolicy::~AbsoluteTimeProfilingInvalidationPolicy() {

}

bool AbsoluteTimeProfilingInvalidationPolicy::keepOnProfiling(PerformanceMeasure *pm) {

	unsigned int samples = pm->getMethodInvocations();
	if (samples == 0) {
		return true;
	}
	unsigned int ratio = samples / samplesLimit;		// will be rounded to 0 for less thatn samplesLimit
	if (pm != NULL && ratio > 0 && !pm->isTheInstrumentationProfilingMethod()) {

		//cout << pm->getMethodId() << " " << fixed << setprecision(9) << pm->getEstimatedRealTimeIncludingCalleeMethods() << " " << " " << (ratio * minimumAcceptableTime) << endl;
		if (pm->getEstimatedRealTimeIncludingCalleeMethods() < ratio * minimumAcceptableTime) {
			//fprintf(stderr, "Invalidating method %d\n", pm->getMethodId());fflush(stderr);
			return false;
		} else {
			return true;
		}

	}
	return true;
}

void AbsoluteTimeProfilingInvalidationPolicy::printInfo(ostream &out) {
	out << "Invalidation policy: Absolute time-based - invalidating profiling of methods whose aggregate estimated time after " << samplesLimit << " samples is less than ";
	Time::concisePrinting(out, minimumAcceptableTime, false);
	out << " (less than ";
	Time::concisePrinting(out, minimumAcceptableTime/samplesLimit, false);
	out << "/sample)" << endl;
}
