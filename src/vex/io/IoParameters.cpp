/*
 * IoParameters.cpp
 *
 *  Created on: 26 Sep 2011
 *      Author: root
 */

#include <cstring>
#include "IoParameters.h"
#include "IoPredictor.h"
#include "PredictionMethodFactory.h"

#include <iostream>
using namespace std;


IoParameters::IoParameters() {
	setDefaultValues();
}

IoParameters::IoParameters(char *parameters, const char *separator) {

	setDefaultValues();
	char allSeps[16];
	sprintf(allSeps, "%s ", separator);

	int count = 0;
	char *maxOptions[100];
	maxOptions[count++] = strtok(parameters, allSeps);

	while (maxOptions[count-1] != NULL && count < 10) {
	  maxOptions[count++] = strtok(NULL, allSeps);
	}
    --count;

	for (int i =0 ; i <count; ++i) {
		if (!parseOption(maxOptions[i], allSeps)) {
			printHelp(parameters);
		}
	}
}

void IoParameters::setDefaultValues() {
	predictionMethod = new char[7];
	strcpy(predictionMethod, "avg");
	predictorType = new char[9];
	strcpy(predictorType, "invpoint");
	predictionMethodBufferSize = 16;
	protocol = new IoProtocolNormal();

	recognizer = NULL;
	excluder = NULL;
	printGlobalIoBuffers = false;
	printIoBuffers = false;
	predictor = NULL;
	ioImportMeasurementsFilesPrefix = NULL;
	globalLoggingBehaviour = NULL;
	modelFilename = NULL;
}

void IoParameters::printHelp(char *parameters) {
	cout << "Error in parsing RTS parameters: " << parameters << endl;
}

bool IoParameters::parseOption(char *option, char *delims) {
	char *attr = strtok(option, "-");
	char *value = strtok(NULL, delims);

	if (attr == NULL || value == NULL) {
		return false;
	}

	return setValue(attr, value);
}

IoProtocol *IoParameters::parseIoPolicy(char *ioPolicyOption) {
	if (strcmp(ioPolicyOption, "serial") == 0) {
		return new IoProtocolSerial();
	} else if (strcmp(ioPolicyOption, "strict") == 0) {
		return  new IoProtocolStrict();
	} else if (strcmp(ioPolicyOption, "lax") == 0) {
		return  new IoProtocolLax();
	} else if (strcmp(ioPolicyOption, "normal") == 0) {
		return  new IoProtocolNormal();
	} else if (strcmp(ioPolicyOption, "none") == 0) {
			return  new IoProtocol();
	} else {
		return NULL;
	}
}

bool IoParameters::parseIoLevel(char *value) {
	if (strcmp(value, "global") == 0 || strcmp(value, "invpoint") == 0 || strcmp(value, "ioop") == 0 || strcmp(value, "none") == 0) {
		predictorType = new char[strlen(value)+1];
		strcpy(predictorType, value);
		return true;
	} else {
		return false;
	}
}


IoLoggingBehaviour *IoParameters::getGlobalLoggingBehaviour() {
	return globalLoggingBehaviour;
}

IoPredictor *IoParameters::createPredictor(PredictionMethodFactory *predictionMethodFactory) {
	if (predictorType != NULL) {
		if (strcmp(predictorType, "global") == 0) {
			predictor = new GlobalIoPrediction(predictionMethodFactory);
		} else if (strcmp(predictorType, "ioop") == 0) {
			predictor = new OperationAwareIoPrediction(predictionMethodFactory);
		} else if (strcmp(predictorType, "invpoint") == 0){
			predictor = new StateAwareIoPrediction(predictionMethodFactory);
		} else {
			predictor = new NoIoPrediction();
		}
	} else {
		predictor = new NoIoPrediction();
	}

	predictor->setGlobalLoggingBehaviour(globalLoggingBehaviour);
	predictor->setIoExcluder(excluder);
	predictor->setIoRecognizer(recognizer);

	return predictor;
}


IoPredictor *IoParameters::getIoPredictor(PredictionMethodFactory *predictionMethodFactory) {
	if (predictor == NULL) {
		predictor = createPredictor(predictionMethodFactory);
	}
	return predictor;
}


char *IoParameters::getModelFilename() {
	return modelFilename;
}

unsigned int IoParameters::getPredictionMethodBufferSize() {
	return predictionMethodBufferSize;
}

bool IoParameters::getLocalLogging() {
	return printIoBuffers;
}

char *IoParameters::getPredictionMethod() {
	return predictionMethod;
}

char *IoParameters::getPredictorType() {
	return predictorType;
}

char *IoParameters::getImportMeasurementsPrefix() {
	return ioImportMeasurementsFilesPrefix;
}

IoProtocol *IoParameters::getIoProtocol() {
	if (excluder == NULL && recognizer == NULL) {
		return protocol;
	} else {
		return new IoProtocolEnforcer(new IoProtocolSerial(), protocol);
	}
}

IoRecognizer *IoParameters::getIoRecognizer() {
	return recognizer;
}

void IoParameters::setLoggingBehaviour(IoLoggingBehaviour *_globalLoggingBehaviour, const bool &_printIoBuffers) {
	globalLoggingBehaviour = _globalLoggingBehaviour;
	printIoBuffers = _printIoBuffers;
}

bool IoParameters::parseRandomIoRecognition(char *value) {
	int acceptanceRate = atoi(value);
	if (acceptanceRate == 0) {
		excluder = new AllIoExcluder();
	} else {
		excluder = new IoExcluder(acceptanceRate);
	}
	return true;
}

bool IoParameters::parseThresholdIoRecognition(char *value) {
	long long lowestIoThreshold = atoi(value);
	recognizer = new EmpiricalRecognizer(lowestIoThreshold);
	return true;
}

bool IoParameters::setValue(char *attr, char *value) {
	if (strcmp(attr, "io") == 0) {
		protocol = parseIoPolicy(value);
		return protocol != NULL;

	} else if (strcmp(attr, "iopred") == 0) {
		predictionMethod = new char[strlen(value)+1];
		strcpy(predictionMethod, value);
		return true;

	} else if (strcmp(attr, "iolevel") == 0) {
		return parseIoLevel(value);

	} else if (strcmp(attr, "iobuffer") == 0) {
		predictionMethodBufferSize = atoi(value);
		return predictionMethodBufferSize>0;

	} else if (strcmp(attr, "iomodel") == 0) {
		 modelFilename = new char[strlen(value)+1];
		 strcpy(modelFilename, value);
		 return true;

	} else if (strcmp(attr, "iostats") == 0 && value != NULL) {
		cout << "Warning: rts_file do not support the \"stats\" parameter. The \"iostats\" parameter applies here as well." << endl;
		return false;

	} else if (strcmp(attr, "ioimport") == 0) {
		ioImportMeasurementsFilesPrefix = new char[strlen(value)+1];
		strcpy(ioImportMeasurementsFilesPrefix, value);
		return true;

	} else if (strcmp(attr, "ioexclude") == 0 && value != NULL) {
		return parseRandomIoRecognition(value);

	} else if (strcmp(attr, "iodisregard") == 0) {
		excluder = new AllIoExcluder();
		return true;
	} else if (strcmp(attr, "iothreshold") == 0 && value != NULL) {
		return parseThresholdIoRecognition(value);

	} else {
		return false;
	}
}

// TODO: Do it write: change all char * to const char * in here and remove this method
bool IoParameters::setValue(const char *attr, const char *value) {
	return setValue(const_cast<char *>(attr), const_cast<char *>(value));
}

IoParameters::~IoParameters() {
	if (predictionMethod != NULL) {
		delete[] predictionMethod;
	}
	if (predictorType != NULL) {
		delete[] predictorType;
	}
}


void IoParameters::printOptions() {
	cout << "--------------------------------------" << endl;
	cout << "Protocol: " << getIoProtocol()->getTitle() << endl;
	if (getPredictorType() != NULL) {
		cout << "Level: " << getPredictorType() << endl;
		PredictionMethodFactory *factory = new PredictionMethodFactory(this);
		cout << "Prediction: " << factory->getPredictionMethodTitle() << endl;
		delete factory;
	} else {
		cout << "Prediction: NO" << endl;
	}
	cout << "Filtering: " << ((getIoRecognizer()!=NULL)?"YES":"NO") << endl;
	cout << "Logging: " << ((getLocalLogging())?"YES":"NO") << endl;
	cout << "ImportPrefix: "<< ((getImportMeasurementsPrefix()!=NULL)?getImportMeasurementsPrefix():"NO") << endl;
	cout << "Model - filename: " << getModelFilename() << endl;
	cout << "Global statistics: " << ((getGlobalLoggingBehaviour() != NULL)?"YES":"NO") << endl;
	cout << "--------------------------------------" << endl;
	cout << endl << endl;

}

void IoParameters::test(char *argv) {
	cout << "PARSING " << argv << endl;
	IoParameters *parameters = new IoParameters(argv, "|");
	parameters->printOptions();
	delete parameters;
}

void IoParameters::test() {
	char test1[] = {"protocol-normal|prediction-median|buffer-16|level-invpoint|stats-invpoint"};
	char test2[] = {"protocol-lax|prediction-sampling|buffer-64|level-global|recogn_random-75|stats-global"};
	char test3[] = {"protocol-serial|level-ioop|prediction-cmean|buffer-128|recogn_thresh-1000000"};
	char test4[] = {"buffer-64|protocol-lax|level-global|prediction-aryulwalk|importPrefix-/data/files_previously-exported"};
	IoParameters::test(test1);
	IoParameters::test(test2);
	IoParameters::test(test3);
	IoParameters::test(test4);
}
