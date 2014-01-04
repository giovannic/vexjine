/*
 * IoRecognizer.cpp
 *
 *  Created on: 16 Aug 2011
 *      Author: root
 */

#include "IoRecognizer.h"

#include <cstdlib>
#include <ctime>

IoRecognizer::IoRecognizer() {

}

IoRecognizer::~IoRecognizer() {

}


PremiscuousRecognizer::PremiscuousRecognizer() {

}

PremiscuousRecognizer::~PremiscuousRecognizer() {

}

bool PremiscuousRecognizer::isRecognizedAsIoOperation(VexThreadState *state, const long long &ioDuration) {
	return true;
}


EmpiricalRecognizer::EmpiricalRecognizer() {
	threshold = 500000;	// default threshold 500 microseconds
}

EmpiricalRecognizer::EmpiricalRecognizer(const long long &_threshold) {
	threshold = _threshold;
}

EmpiricalRecognizer::~EmpiricalRecognizer() {

}

bool EmpiricalRecognizer::isRecognizedAsIoOperation(VexThreadState *state, const long long &ioDuration) {
	// The state is indifferent here - maybe we can add a real vs cpu time heuristics
	if (ioDuration == 0 || ioDuration > threshold) {
		return true;
	} else {
		return false;
	}
}


IoExcluder::IoExcluder() {
	acceptancePercentage = 1;
	srand(time(NULL));
}

IoExcluder::IoExcluder(const int &_acceptancePercentage) {
	acceptancePercentage = (float)_acceptancePercentage / 100.0;
	srand(time(NULL));
}

IoExcluder::IoExcluder(const float &_acceptancePercentage) {
	acceptancePercentage = _acceptancePercentage;
	srand(time(NULL));
}
IoExcluder::~IoExcluder() {

}

bool IoExcluder::shouldRandomlyExcludeIo() {
	return (((float)rand()/(float)RAND_MAX) <= acceptancePercentage);
}

bool AllIoExcluder::shouldRandomlyExcludeIo() {
	return false;
}

AllIoExcluder::~AllIoExcluder() {

}


