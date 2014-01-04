#ifndef TOOLS_MEASURE_H
#define TOOLS_MEASURE_H

#include "SimulationTime.h"

#include <iostream>



class Measure {

public:
	virtual void add( double x ) = 0;
	virtual double mean() = 0;
	virtual double variance() = 0;
	Measure() ;
	virtual ~Measure();
	Measure( int m ) ;
	int count() ;
	double getMoment( int n ) ;
	void resetMeasures() ;

protected:
	int moments, n ;
	double *moment ;
	double resetTime  ;

private:
	int maxMeasures ;

};

#endif
