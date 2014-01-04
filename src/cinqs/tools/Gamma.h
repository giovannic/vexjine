#ifndef TOOLS_GAMMA_H
#define TOOLS_GAMMA_H

#include "DistributionSampler.h"
#include "RejectionMethod.h"

class GammaSampler;

class Gamma : public DistributionSampler {

public:
	Gamma( double theta, int beta ) ;

	double bigF( double x );
	double f( double x );
	double next() ;
	//		GammaSampler() ;
	static double gamma( double theta, int beta ) ;

	~Gamma();

	double getM() { return m; }
	double getB() { return b; }
private:
	double m, b, theta, betatheta ;
	int beta ;
	int *fact ;
	double epsilon ;

	GammaSampler *gammaSampler ;
};

class GammaSampler : public RejectionMethod {
public:
	GammaSampler(Gamma *gamma) : RejectionMethod(0, gamma->getB(), gamma->getM()) {
		this->gamma = gamma;
	}
	double density( double x ) {
		return gamma->f( x ) ;
	}

	virtual ~GammaSampler() {

	}
private:
	Gamma *gamma;
};
#endif
