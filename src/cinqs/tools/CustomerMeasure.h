#ifndef TOOLS_CUSTOMERMEASURE_H
#define TOOLS_CUSTOMERMEASURE_H

#include "Measure.h"

class CustomerMeasure : public Measure {

public:
	CustomerMeasure() : Measure() {};
	CustomerMeasure( int m ) : Measure(m) {};

	void add( double x ) ;
	double mean() ;
	double variance() ;

};

#endif
