/*
 * AutoRegression.h: Used for Burg autoregressive method for I/O prediction
 *
 *  Created on: 27 Aug 2011
 *      Author: root
 */

#ifndef AUTOREGRESSION_H_
#define AUTOREGRESSION_H_

class AutoRegression {
public:
	AutoRegression(int _length);
	double getNextPrediction(long long *inputseries);
	virtual bool autoregress() = 0;
	virtual ~AutoRegression();

protected:
	int degree;
	int length;

	double *normalizedVector;
	double *coefficients;
	double **rMatrix;
};

/**
 * Max entropy method based on Burg
 */
class BurgAutoRegression : public AutoRegression {
public:
	BurgAutoRegression(int _length) : AutoRegression(_length) {
		pef = new double[length+1];
		per = new double[length+1];

		g = new double[degree+2];
		h = new double[degree+1];

		ar = new double*[degree+1];
		for (int i=0; i<degree+1; i++) {
			ar[i] = new double[degree+1];
		}
	};
	bool autoregress();
	virtual ~BurgAutoRegression();

private:
	double *h;
	double *g;
	double *per;
	double *pef;
	double **ar;

};


/**
 * Least squares method based on Yules-Walker
 */
class YulesWalkerAutoRegression : public AutoRegression {
public:
	YulesWalkerAutoRegression(int _length) : AutoRegression(_length) {
		vec = new double[length];

	};
	bool autoregress();
	virtual ~YulesWalkerAutoRegression();

private:
	double *vec;

	bool SolveLE();
};

#endif /* AUTOREGRESSION_H_ */
