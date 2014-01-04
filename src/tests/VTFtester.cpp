/*
 * VTFtester.cpp
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#include "VTFtester.h"

VTFtester::VTFtester() {
	

}

VTFtester::~VTFtester() {
	 
}

string VTFtester::testName() {
	return "VTF arguments parsing";
}

bool VTFtester::test() {
	return optionsParsing();
}

bool VTFtester::optionsParsing() {
	char *options = new char[256];
	strcpy(options, "=delays_file=/data/vtf_delays");
	assert(VEX::initializeSimulator(options));

	VEX::setDefaultValues();
	strcpy(options, "delays_file=/data/vtf_delays,");
	assert(VEX::initializeSimulator(options));

	VEX::setDefaultValues();
	strcpy(options, "delays_file=/data/vtf_delays");
	assert(VEX::initializeSimulator(options));

	VEX::setDefaultValues();
	strcpy(options, "show_log,delays_file=/data/vtf_delays,show_log");
	assert(VEX::initializeSimulator(options));

	VEX::setDefaultValues();
	strcpy(options, "show_log,delays_file=/data/vtf_delays,debug=256-35-432");
	assert(VEX::initializeSimulator(options));

	VEX::setDefaultValues();
	strcpy(options, "delays_file=/data/vtf_delays,debug=256-35-23,show_log");
	assert(VEX::initializeSimulator(options));

	VEX::setDefaultValues();
	strcpy(options, "delays_file=/data/vtf_delays,output=hprof,debug=256-35-23");
	assert(VEX::initializeSimulator(options));

	delete[] options;
	return true;
}
