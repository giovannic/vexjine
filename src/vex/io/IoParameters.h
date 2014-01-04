/*
 * IoParameters.h
 *
 *  Created on: 26 Sep 2011
 *      Author: root
 */

#ifndef IOPARAMETERS_H_
#define IOPARAMETERS_H_

#include "IoProtocol.h"
#include "IoRecognizer.h"
#include "IoPredictor.h"

class PredictionMethodFactory;

class IoParameters {
public:
	IoParameters();
	IoParameters(char *parameters, const char *separator);
	virtual ~IoParameters();

	static IoProtocol *parseIoPolicy(char *ioPolicyOption);

	char *getPredictionMethod();
	char *getPredictorType();
	IoPredictor *getIoPredictor(PredictionMethodFactory *);
	char *getModelFilename();
	unsigned int getPredictionMethodBufferSize();
	bool getLocalLogging();
	IoProtocol *getIoProtocol();
	char *getImportMeasurementsPrefix();
	IoRecognizer *getIoRecognizer();
	void setLoggingBehaviour(IoLoggingBehaviour *_globalLoggingBehaviour, const bool &_printIoBuffers);
	IoLoggingBehaviour *getGlobalLoggingBehaviour();

	void printOptions();
	static void test();
	static void test(char *);
	bool setValue(char *attr, char *value);
	bool setValue(const char *attr, const char *value);


private:
	void printHelp(char *parameters);
	bool parseOption(char *option, char *delims);
	bool parseIoLevel(char *value);
	bool parseIoStats(char *value);


	IoPredictor *createPredictor(PredictionMethodFactory *);

	bool parseRandomIoRecognition(char *value);
	bool parseThresholdIoRecognition(char *value);
	void setDefaultValues();

	char *predictionMethod;
	char *predictorType;
	char *ioImportMeasurementsFilesPrefix;
	char *modelFilename;

	unsigned int predictionMethodBufferSize;
	bool printGlobalIoBuffers;
	bool printIoBuffers;
	IoProtocol *protocol;
	IoRecognizer *recognizer;
	IoExcluder *excluder;
	IoPredictor *predictor;
	IoLoggingBehaviour *globalLoggingBehaviour;
};

#endif /* IOPARAMETERS_H_ */
