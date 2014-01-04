#ifndef TOOLS_DETERMINISTIC_H
#define TOOLS_DETERMINISTIC_H

#include "DistributionSampler.h"

class Deterministic : public DistributionSampler {

public:
	Deterministic( double t ) ;
	double next() ;
	static double deterministic( double t ) ;

private:
	double time ;

};

#endif
