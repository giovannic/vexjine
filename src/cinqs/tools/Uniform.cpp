#include "tools/Uniform.h"


Uniform::Uniform( double a, double b ) {
	this->a = a ;
	this->b = b ;
}

double Uniform::next() {
	return ((double)rand()/(double)RAND_MAX) * ( b - a ) + a ;
}

double Uniform::uniform(  double a, double b ) {
	return ((double)rand()/(double)RAND_MAX) * ( b - a ) + a ;
}
