#include "tools/Pareto.h"


Pareto::Pareto( double k, double a, double b ) {
	this->k = k ;
	this->a = a ;
	this->b = b ;
	ak = pow( a, -k ) ;
	bk = pow( b, -k ) ;
}

double Pareto::next() {
	return pow( ak - ((double)rand()/(double)RAND_MAX) * ( ak - bk ), -1/k ) ;
}

double Pareto::pareto( double k, double a, double b ) {
	double ak = pow( a, -k ) ;
	double bk = pow( b, -k ) ;
	return pow( ak - ((double)rand()/(double)RAND_MAX) * ( ak - bk ), -1/k ) ;
}
