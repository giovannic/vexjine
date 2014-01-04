#include "tools/Erlang.h"


Erlang::Erlang( int k, double theta ) {
	this->k = k ;
	this->theta = theta ;
}

double Erlang::next() {
	acc = 1.0 ;
	for ( int i = 1 ; i <= k ; i++ )
		acc *= ((double)rand()/(double)RAND_MAX) ;
	return -log( acc ) / ( k * theta ) ;
}

double Erlang::erlang( int k, double theta ) {
	double acc = 1.0 ;
	for ( int i = 1 ; i <= k ; i++ )
		acc *= ((double)rand()/(double)RAND_MAX) ;
	return -log( acc ) / ( k * theta ) ;
}
