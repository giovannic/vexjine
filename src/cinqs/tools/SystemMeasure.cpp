#include "tools/SystemMeasure.h"

#include "Sim.h"

#include <cmath>

SystemMeasure::SystemMeasure() {
	lastChange = 0.0;
	current = 0.0;
}

void SystemMeasure::add( double x ) {
	for ( int i = 1 ; i <= moments ; i++ )
		moment[ i ] += ( pow( current, (double) i ) ) *
		( Sim::now() - lastChange ) ;
	current = x ;
	lastChange = Sim::now() ;
	n++ ;
}

SystemMeasure::~SystemMeasure() {

}

double SystemMeasure::mean() {
	return moment[1] / ( Sim::now() - resetTime ) ;
}

double SystemMeasure::variance() {
	double mean = this->mean() ;
	return moment[2] / ( Sim::now() - resetTime ) - mean * mean ;
}

double SystemMeasure::currentValue() {
	return current ;
}

double SystemMeasure::timeLastChanged() {
	return lastChange ;
}

