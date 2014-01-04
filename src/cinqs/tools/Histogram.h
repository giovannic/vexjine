#ifndef TOOLS_HISTOGRAM_H
#define TOOLS_HISTOGRAM_H

#include "CustomerMeasure.h"

class Histogram : public CustomerMeasure {

public:
	Histogram( double l, double h, int b ) ;
	void add( double x ) ;
	int bucketContent( int i ) ;
	void display() ;

	~Histogram();
private:
	int *bucket ;
	double low, high, width ;
	int n ;
	int underflows, overflows ;

};

#endif
