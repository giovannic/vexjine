#include "tools/Gamma.h"

// Only for integer values of beta, so Gamma( b, t ) is the same as
// Erlang( b, t ) - the code is for demonstration purposes only,
// showing the application of the rejection method and reinforcing
// the tutorial exercise on distribution sampling


Gamma::Gamma( double theta, int beta ) {
	epsilon = 0.00001;

	this->theta = theta ;
	this->beta = beta ;
	betatheta = beta * theta ;
	fact = new int[ beta ] ;
	int fi = 1 ;
	for ( int i = 0 ; i < beta ; i++ ) {
		fact[ i ] = fi ;
		fi *= ( i + 1 ) ;
	}
//	double y = 0.0 ;//UNUSED!
	m = f( ( beta - 1 ) / betatheta ) ;
	double x = 1.0, xold = 2.0 ;
	while ( fabs( x - xold ) / x > epsilon ) {
		xold = x ;
		x = xold - ( bigF( xold ) - 0.999 ) / f( xold ) ;
	}
	b = x ;
	gammaSampler = new GammaSampler(this) ;
}

Gamma::~Gamma() {
	delete[] fact;
	delete gammaSampler;
}

double Gamma::bigF( double x ) {
	double acc = 0.0 ;
	for ( int i = 0 ; i < beta-1 ; i++ )
		acc += pow( betatheta * x, (float) i ) / fact[ i ] ;
	return 1 - acc * exp( -betatheta * x ) ;
}

double Gamma::f( double x ) {
	return betatheta * pow( betatheta * x, (float) beta - 1 ) *
			exp( -betatheta * x ) / fact[ beta - 1 ] ;
}

double Gamma::next() {
	return gammaSampler->next() ;
}

double Gamma::gamma( double theta, int beta ) {
	Check::check( false, "Static method for gamma sampling not available\nUse Gamma class instead" ) ;
	return 0.0 ;
}
