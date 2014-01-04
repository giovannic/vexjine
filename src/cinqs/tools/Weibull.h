#ifndef TOOLS_WEIBULL_H
#define TOOLS_WEIBULL_H

#include "DistributionSampler.h"

class Weibull : public DistributionSampler {

public:
	Weibull( double alpha, double beta ) ;
	double next() ;
	static double weibull(  double alpha, double beta ) ;

private:
	double alpha, beta ;

};

#endif
