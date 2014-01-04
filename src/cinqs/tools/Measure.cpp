#include "tools/Measure.h"

Measure::Measure() {
	moments = 2 ;
	n = 0 ;
	moment = new double[ 100 ] ;
	for (int i =0 ; i<100; i++) {
		moment[i] = 0.0;
	}
	resetTime = 0.0 ;
	maxMeasures = 10 ;
}

Measure::~Measure() {
	delete[] moment;
}

Measure::Measure( int m ) {
	n = 0 ;
	moment = new double[ 100 ] ;
	resetTime = 0.0 ;
	maxMeasures = 10 ;

	if ( m > maxMeasures ) {
		moments = maxMeasures ;
	} else if ( m < 2 ) {
		moments = 2 ;
	} else {
		moments = m ;
	}
};

//------------------------------------------------


int Measure::count() {
	return n ;
}


double Measure::getMoment( int n ) {
	return moment[ n ] ;
}

void Measure::resetMeasures() {
	resetTime = SimulationTime::now() ;
	n = 0 ;
	for ( int i = 1 ; i <= moments ; i++ )
		moment[ i ] = 0.0 ;
}

