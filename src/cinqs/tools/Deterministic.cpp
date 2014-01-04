#include "tools/Deterministic.h"


Deterministic::Deterministic( double t ) {
	time = t ;
}

double Deterministic::next() {
	return time ;
}

double Deterministic::deterministic( double t ) {
	return t ;
}
