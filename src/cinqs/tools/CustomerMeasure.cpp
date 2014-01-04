#include "tools/CustomerMeasure.h"

#include <cmath>

void CustomerMeasure::add( double x ) {
	for ( int i = 1 ; i <= moments ; i++ )
		moment[ i ] += pow( x, (double) i ) ;
	n += 1 ;
}

double CustomerMeasure::mean() {
	return moment[1] / n ;
}

double CustomerMeasure::variance() {
	double mean = this->mean() ;
	return ( moment[2] - n * mean * mean ) / ( n - 1 ) ;
}

