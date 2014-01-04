#ifndef TOOLS_EXP_H
#define TOOLS_EXP_H

#include "DistributionSampler.h"

class Exp : public DistributionSampler {

public:
	Exp( double r ) ;
	double next() ;
	static double exp( double lam ) ;

	const char *getDistributionSampleType();
private:
	double rate ;

};

#endif
