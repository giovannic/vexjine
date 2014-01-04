#include "tools/Sim.h"

#include "SimulationTime.h"

#include <iostream>
using namespace std;

Diary *Sim::diary ;
double *Sim::vtime ;

Sim::Sim() {
	initialize();
	*vtime = 0.0 ;
}

void Sim::initialize() {
	diary = new Diary() ;
	vtime = SimulationTime::timePtr();
}

double Sim::now() {
	return *vtime ;
}

void Sim::schedule( Event *e ) {
	diary->insertInOrder( e ) ;
}

void Sim::deschedule( Event *e ) {
	diary->remove( e ) ;
}

void Sim::simulate() {
	go() ;
}

void Sim::simulate( double t ) {
	schedule( new EndOfWarmUp( now() + t, this) ) ;
	go() ;
}

void Sim::go() {
	while ( !diary->isEmpty() && !stop()) {
		Event *e = diary->removeFromFront() ;
		*vtime = e->invokeTime() ;
		//      cout << vtime);
		if ( !stop() ) {
			e->invoke() ;
		}
		delete e;
	}
}

void Sim::resetMeasures() {
	cout <<  "WARNING: resetMeasures() has not been overridden" << endl ;
}
