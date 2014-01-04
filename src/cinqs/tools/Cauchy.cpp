#include "tools/Cauchy.h"


Cauchy::Cauchy( double a, double b ) {
	alpha = a ;
	beta = b ;
	norm = new Normal( 0, 1 );
}

double Cauchy::next() {
	return ( norm->next() / norm->next() ) * beta + alpha ;
}

double Cauchy::cauchy( double a, double b ) {
	return ( Normal::normal( 0, 1 ) / Normal::normal( 0, 1 ) ) * b + a ;
}
