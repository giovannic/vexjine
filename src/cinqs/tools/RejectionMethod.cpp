#include "tools/RejectionMethod.h"

//
// Generic code for the (acceptance-)rejection method...
//

RejectionMethod::RejectionMethod( double a, double b, double m ) {
	this->a = a ;
	this->b = b ;
	this->m = m ;
}

double RejectionMethod::next() {
	double x = Uniform::uniform( a, b ) ;
	double y = Uniform::uniform( 0, m ) ;
	int nrej = 0 ;
	while ( y > density( x ) ) {
		x = Uniform::uniform( a, b ) ;
		y = Uniform::uniform( 0, m ) ;
		nrej++ ;
	}
	return x ;
}
