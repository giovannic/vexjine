#ifndef TOOLS_AREAHISTOGRAM_H
#define TOOLS_AREAHISTOGRAM_H

#include "SystemMeasure.h"
#include "Sim.h"

class AreaHistogram : public SystemMeasure {

public:
	AreaHistogram( double l, double h, int b ) : SystemMeasure() {
		bucket = new double [b] ;
		low = l ;
		high = h ;
		n = b ;
		width = ( high - low ) / n ;
		overflows = 0;
	}
	void add( double t ) ;
	double bucketContent( int i ) ;
	void display() ;

	~AreaHistogram();
private:
	double *bucket;
	double low, high, width ;
	int n ;
	int underflows, overflows;

};

#endif
