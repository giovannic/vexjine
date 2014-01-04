#include "network/ClassDependentDelay.h"

#include <sstream>
using namespace std;
ClassDependentDelay::ClassDependentDelay( int *classes, DistributionSampler **samplers , int classesCount) {
	this->classes = classes ;
	this->samplers = samplers ;
	this->classesCount = classesCount;
}

const char *ClassDependentDelay::getDelayDistributionSampleType() {
	stringstream str;
	str << "ClassDependentDelay: ";
	for ( int i = 0 ; i < classesCount; i++ ) {
		if (i > 0) {
			str << " / ";
		}
		str << samplers[i]->getDistributionSampleType();
	}
	return str.str().c_str();
}


double ClassDependentDelay::sample() {
	cout <<  "ERROR - ClassDependentDelay (no customer argument)" << endl ;
	return 0.0 ;
}

double ClassDependentDelay::sample( Customer *c ) {

	for ( int i = 0 ; i < classesCount; i++ ) {
		if ( classes[i] == c->getclass() ) {
			return samplers[i]->next() ;
		}
	}
	return 0.0 ;
}

