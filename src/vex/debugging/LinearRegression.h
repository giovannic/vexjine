/*
 * LinearRegression.h: Class to apply linear regression on an array of long long - used for I/O prediction
 *
 *  Created on: 27 Aug 2011
 *      Author: root
 */

#ifndef LINEARREGRESSION_H_
#define LINEARREGRESSION_H_

class LinearRegression {
public:
	LinearRegression(const unsigned int &_size);
	virtual ~LinearRegression();

	long long apply(const long long *buffer);

protected:
	unsigned int size;
	long long Sx, Sy, Sxx, Sxy, Syy;
};

#endif /* LINEARREGRESSION_H_ */
