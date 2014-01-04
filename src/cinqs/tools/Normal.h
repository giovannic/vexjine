#ifndef TOOLS_NORMAL_H
#define TOOLS_NORMAL_H

#include "DistributionSampler.h"

class Normal : public DistributionSampler {

public:
	Normal( double mu, double sigma ) ;
	double next() ;
	static double normal(  double m, double s ) ;

	const char *getDistributionSampleType();
private:
	static const double twoPI = 2 * M_PI;
	double mu, sigma, r1, r2, k ;
	bool mustRedo ;

};

#endif
