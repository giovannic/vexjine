#ifndef TOOLS_CAUCHY_H
#define TOOLS_CAUCHY_H

#include "Normal.h"

class Cauchy : public DistributionSampler {

public:
	Cauchy( double a, double b ) ;
	double next() ;
	static double cauchy( double a, double b ) ;

private:
	double alpha, beta ;
	Normal *norm;

};

#endif
