/*
 * PredictionMethodFactory.h
 *
 *  Created on: 26 Sep 2011
 *      Author: root
 */

#ifndef PREDICTIONMETHODFACTORY_H_
#define PREDICTIONMETHODFACTORY_H_

#include "PredictionMethod.h"
#include "IoParameters.h"

/**
 * Prediction method factory
 */
class PredictionMethodFactory {
public:
	PredictionMethodFactory();
	PredictionMethodFactory(IoParameters *parameters);
	PredictionMethodFactory(const char *method, const unsigned int &_bufferSize);

	bool setPredictionMethod(const char *method, const unsigned int &_bufferSize);
	bool setPredictionMethod(const char *method);
	bool setPredictionBufferSize(const unsigned int &_bufferSize);
	PredictionMethod *getPredictionMethod(IoState &ioState);
	const char *getPredictionMethodTitle();

	void setLocalLogging(const bool &value);
	bool isLocalLoggingEnabled() {
		return localLogging;
	}
	void setIoImportMeasurementsFilesPrefix(char *value);

private:
	short type;
	unsigned int markovStates;
	unsigned int bufferSize;
	bool localLogging;
	char *ioImportMeasurementsFilesPrefix;
};


#endif /* PREDICTIONMETHODFACTORY_H_ */
