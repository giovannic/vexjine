/*
 * QStats.h: Class for quick generation of basic statistics (mean, stdev) from a number of samples
 *
 *  Created on: 12 Aug 2011
 *      Author: nb605
 */

#ifndef QSTATS_H_
#define QSTATS_H_

#include <iostream>
#include <cmath>

template<typename sampleType>
class QStats {
public:

	QStats() {
		samples = 0;
		varianceTerm = 0;
		sum = 0;
	}

	virtual ~QStats() {

	}

	void addSample(const sampleType &sample) {
		++samples;
		if (samples > 1) {
			sampleType term = (sum - (samples-1) * sample);
			varianceTerm += (term / (double)samples) * (term / (double)(samples-1.0));
		}
		sum += sample;
	}

	double getMean() {
		if (samples > 0) {
			return (double)sum /(double)samples;
		} else {
			return 0;
		}
	}

	double getStdev() {
		if (samples > 1) {
			return (double)sqrt(varianceTerm /(double)(samples - 1.0));
		} else {
			return 0;
		}
	}

	int getSamples() {
		return samples;
	}

	friend std::ostream & operator <<(std::ostream &outs, QStats<sampleType> &stat) {
		outs << stat.samples << "," << stat.getMean() << "," << stat.getStdev();
		return outs;
	};

private:
	sampleType sum;
	double varianceTerm;
	int samples;
};

#endif /* QSTATS_H_ */
