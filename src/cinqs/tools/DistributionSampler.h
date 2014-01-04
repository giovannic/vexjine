#ifndef TOOLS_DISTRIBUTIONSAMPLER_H
#define TOOLS_DISTRIBUTIONSAMPLER_H

#include <cstdlib>
#include <cmath>

#include "Check.h"

class DistributionSampler {

public:
	virtual double next() = 0;
	virtual const char *getDistributionSampleType();
	virtual ~DistributionSampler();
};

#endif
