/*
 * PredictionMethodFactory.cpp
 *
 *  Created on: 26 Sep 2011
 *      Author: root
 */

#include "PredictionMethodFactory.h"

#include <sstream>

/***
 * Factory for prediction methods
 */
PredictionMethodFactory::PredictionMethodFactory() {
	type = 2;
	bufferSize = 16;
	localLogging = false;
	ioImportMeasurementsFilesPrefix = NULL;
	markovStates = 2;
}

PredictionMethodFactory::PredictionMethodFactory(IoParameters *parameters) {
	setPredictionMethod(parameters->getPredictionMethod());
	bufferSize = parameters->getPredictionMethodBufferSize();
	localLogging = parameters->getLocalLogging();
	ioImportMeasurementsFilesPrefix = parameters->getImportMeasurementsPrefix();
	markovStates = 2;
}

PredictionMethodFactory::PredictionMethodFactory(const char *method, const unsigned int &_bufferSize) {
	type = 2;
	bufferSize = _bufferSize;
	localLogging = false;
	ioImportMeasurementsFilesPrefix = NULL;
	setPredictionMethod(method);
}

bool PredictionMethodFactory::setPredictionMethod(const char *method, const unsigned int &_bufferSize) {
	if (setPredictionMethod(method) && setPredictionBufferSize(_bufferSize)) {
		return true;
	} else {
		return false;
	}
}

void PredictionMethodFactory::setIoImportMeasurementsFilesPrefix(char *value) {
	ioImportMeasurementsFilesPrefix = new char[strlen(value)+1];
	strcpy(ioImportMeasurementsFilesPrefix, value);
}

bool PredictionMethodFactory::setPredictionMethod(const char *method) {
	if (method == NULL) {
		type = 1;
		return true;
	}
	if (strcmp(method, "avg") == 0) {
		type = 1;
	} else if (strcmp(method, "median") == 0) {
		type = 2;
	} else if (strcmp(method, "sampling") == 0) {
		type = 3;
	} else if (strcmp(method, "min") == 0) {
		type = 4;
	} else if (strcmp(method, "max") == 0) {
		type = 5;
	} else if (strcmp(method, "gsl") == 0) {
		type = 6;
	} else if (strcmp(method, "cmean") == 0) {
		type = 7;
	} else if (strcmp(method, "linregr") == 0) {
		type = 8;
	} else if (strcmp(method, "arburg") == 0) {
		type = 9;
	} else if (strcmp(method, "aryulwalk") == 0) {
		type = 10;
	} else if (strncmp(method, "markov", 6) == 0) {
		type = 11;
		string s(method);
		if (s.size() > 6) {
			markovStates = atoi(s.substr(6).c_str());
		}
	} else if (strcmp(method, "none") == 0) {
		type = 12;
	} else if (strcmp(method, "replay") == 0) {
		type = 0;
	} else {
		return false;
	}
	return true;
}

void PredictionMethodFactory::setLocalLogging(const bool &value) {
	localLogging = value;
}

bool PredictionMethodFactory::setPredictionBufferSize(const unsigned int &_bufferSize) {
	if (_bufferSize > 0) {
		bufferSize = _bufferSize;
	} else {
		return false;
	}
	return true;
}


PredictionMethod *PredictionMethodFactory::getPredictionMethod(IoState &ioState) {
	PredictionMethod *pm = NULL;
	switch(type) {
		case 1: pm = new AverageCyclicBuffer(bufferSize);break;
		case 2: pm = new MedianCyclicBuffer(bufferSize);break;
		case 3: pm = new SamplingCyclicBuffer(bufferSize);break;
		case 4: pm = new MinimumCyclicBuffer(bufferSize);break;
		case 5: pm = new MaximumCyclicBuffer(bufferSize);break;
		case 6: if (ioImportMeasurementsFilesPrefix != NULL) { pm = new GslHistogramPredictor(); } else {pm = new ReplayCyclicBuffer(bufferSize);} break;  //cout << "Warning: GSL-histogram prediction without any importing files - using replay sampling instead" << endl;
		case 7: pm = new ConstantMeanPredictor(); break;
		case 8: pm = new SimpleLinearRegressionCyclicBuffer(bufferSize);break;
		case 9: pm = new BurgAutoRegressionCyclicBuffer(bufferSize); break;
		case 10: pm = new YulesWalkerAutoRegressionCyclicBuffer(bufferSize); break;
		case 11: pm = new MarkovModulatedProcess(bufferSize, markovStates); break;
		case 12: pm = new NoPredictor();break;
		default: pm = new ReplayCyclicBuffer(bufferSize);
	}
	if (localLogging) {
		IoLoggingBehaviour *localLog = new IoLoggingBehaviour();
		localLog->addLocalIoLoggers();

		pm->setLoggingBehaviour(localLog);
	}

	if (ioImportMeasurementsFilesPrefix != NULL) {
		pm->import(ioImportMeasurementsFilesPrefix, ioState);
	}
	return pm;
}


const char *PredictionMethodFactory::getPredictionMethodTitle() {
	std::stringstream str;
	switch(type) {
		case 1: str << "Average Cyclic Buffer " << bufferSize;break;
		case 2: str << "Median Cyclic Buffer " << bufferSize;break;
		case 3: str << "Sampling Cyclic Buffer " << bufferSize;break;
		case 4: str << "Minimum Cyclic Buffer " << bufferSize;break;
		case 5: str << "Maximum Cyclic Buffer " << bufferSize;break;
		case 6: return "GSL-histrogram";
		case 7: return "Constant mean";
		case 8: str << "Simple Linear Regression Cyclic Buffer " << bufferSize;break;
		case 9: str << "Auto-Regression (Burg) Cyclic Buffer " << bufferSize;break;
		case 10: str << "Auto-Regression (Yules-Walker) Cyclic Buffer " << bufferSize;break;
		case 11: str << "Markov-Modulated Process with " << markovStates << " states and one-off training using " << bufferSize << " samples";break;
		default: str << "Replay Cyclic Buffer " << bufferSize;
	}
	 return str.str().c_str();
}



