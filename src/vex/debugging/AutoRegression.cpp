/*
 * AutoRegression.cpp
 *
 *  Created on: 27 Aug 2011
 *      Author: root
 */

#include "AutoRegression.h"

#include <iostream>
#include <cstdlib>

using namespace std;

#include <cmath>

AutoRegression::AutoRegression(int _length) {
	length = _length;
	degree = length - 2;
	coefficients = new double[degree];
	normalizedVector = new double[_length];
	rMatrix = new double*[degree];
	for (int i = 0; i<degree; i++) {
		rMatrix[i] = new double[degree];
	}
}

AutoRegression::~AutoRegression() {
	delete[] normalizedVector;
	delete[] coefficients;
	for (int i = 0; i<degree; i++) {
		delete[] rMatrix[i];
	}
	delete[] rMatrix;
}


double AutoRegression::getNextPrediction(long long *inputseries) {

	/* Determine and subtract the mean from the input series */
	//cout <<" ================ " << endl;

//	cout << "x = c(";
	double mean = 0.0;
	for (int t=0; t<length; t++) {
		if (t != length-1) {
//			cout << inputseries[t] << ", ";
		} else {
//			cout << inputseries[t] << ")" << endl;
		}
		mean += inputseries[t];
	}
	mean /= (double)length;

	for (int t=0;t<length;t++) {
		normalizedVector[t] = inputseries[t] - mean;
	}

	for (int i=0;i<degree;i++) {
		coefficients[i] = 0.0;
		for (int j=0;j<degree;j++) {
			rMatrix[i][j] = 0.0;
		}
	}

	if (!autoregress()) {
		return 0;
	}

	//cout << "coefficients = c(";
	for (int i=0;i<degree;i++) {
		if (i != degree-1) {
	//		cout << coefficients[i] << ", ";
		} else {
	//		cout << coefficients[i] << ")" << endl;
		}
	}

	double prediction = mean;
	//cout << "prediction = " << mean << endl;
	for (int i =0 ; i<degree; i++) {
	//	cout << "prediction += normalizedVector[" << (length - 1 - i) << "] * coefficients[" << i << "] = " << (normalizedVector[length - 1 - i]) << " * " << coefficients[i] << " = " << (normalizedVector[length - 1 - i] * coefficients[i]) << " = ";
		prediction += normalizedVector[length - 1 - i] * coefficients[i];
	//	cout << prediction << endl;
	}

	if (prediction <= 0) {
//		int originalDegree= degree;
//		--degree;
//		prediction = getNextPrediction(inputseries);
//		if (prediction <= 0) {
//			return 0;
//		}
//		degree = originalDegree;
		return 0;
	}

//	for (int i =0 ; i<length; i++) {
//		cout << inputseries[i] << ", ";
//	}
//	cout << " --> " << prediction << endl;
	return prediction;
}



bool YulesWalkerAutoRegression::autoregress() {

	int i,j,k,hj,hi;
	for (i=degree-1;i<length-1;i++) {
		hi = i + 1;
		for (j=0;j<degree;j++) {
			hj = i - j;
			coefficients[j] += (normalizedVector[hi] * normalizedVector[hj]);
			for (k=j;k<degree;k++) {
				rMatrix[j][k] += (normalizedVector[hj] * normalizedVector[i-k]);
			}
		}
	}
	for (i=0;i<degree;i++) {
		coefficients[i] /= (length - degree);
		for (j=i;j<degree;j++) {
			rMatrix[i][j] /= (length - degree);
			rMatrix[j][i] = rMatrix[i][j];
		}
	}

	/* Solve the linear equations */
	return SolveLE();

}



/*
   Gaussian elimination solver
   Author: Rainer Hegger Last modified: Aug 14th, 1998
   Modified (for personal style and context) by Paul Bourke
 */
bool YulesWalkerAutoRegression::SolveLE() {
	int i,j,k,maxi;
	double vswap,*mswap,*hvec,max,h,pivot,q;

	for (i=0;i<degree-1;i++) {
		max = fabs(rMatrix[i][i]);
		maxi = i;
		for (j=i+1;j<degree;j++) {
			if ((h = fabs(rMatrix[j][i])) > max) {
				max = h;
				maxi = j;
			}
		}
		if (maxi != i) {
			mswap     = rMatrix[i];
			rMatrix[i]    = rMatrix[maxi];
			rMatrix[maxi] = mswap;
			vswap     = coefficients[i];
			coefficients[i]    = coefficients[maxi];
			coefficients[maxi] = vswap;
		}

		hvec = rMatrix[i];
		pivot = hvec[i];
		if (fabs(pivot) == 0.0) {
			return false;
		}
		for (j=i+1;j<degree;j++) {
			q = - rMatrix[j][i] / pivot;
			rMatrix[j][i] = 0.0;
			for (k=i+1;k<degree;k++) {
				rMatrix[j][k] += q * hvec[k];
			}
			coefficients[j] += (q * coefficients[i]);
		}
	}
	coefficients[degree-1] /= rMatrix[degree-1][degree-1];
	for (i=degree-2;i>=0;i--) {
		hvec = rMatrix[i];
		for (j=degree-1;j>i;j--) {
			coefficients[i] -= (hvec[j] * coefficients[j]);
		}
		coefficients[i] /= hvec[i];
	}

	return true;
}

YulesWalkerAutoRegression::~YulesWalkerAutoRegression() {
	delete[] vec;
}




bool BurgAutoRegression::autoregress() {

	int j,n,nn,jj;
	double sn,sd;
	double t1,t2;

	for (j=1;j<=length;j++) {
		pef[j] = 0;
		per[j] = 0;
	}

	for (nn=2;nn<=degree+1;nn++) {
		n  = nn - 2;
		sn = 0.0;
		sd = 0.0;
		jj = length - n - 1;
		for (j=1;j<=jj;j++) {
			t1 = normalizedVector[j+n] + pef[j];
			t2 = normalizedVector[j-1] + per[j];
			sn -= 2.0 * t1 * t2;
			sd += (t1 * t1) + (t2 * t2);
		}
		g[nn] = sn / sd;
		t1 = g[nn];
		if (n != 0) {
			for (j=2;j<nn;j++)
				h[j] = g[j] + (t1 * g[n - j + 3]);
			for (j=2;j<nn;j++)
				g[j] = h[j];
			jj--;
		}
		for (j=1;j<=jj;j++) {
			per[j] += (t1 * pef[j]) + (t1 * normalizedVector[j+nn-2]);
			pef[j]  = pef[j+1] + (t1 * per[j+1]) + (t1 * normalizedVector[j]);
		}

		for (j=2;j<=nn;j++) {
			ar[nn-1][j-1] = g[j];
		}
	}

	for (int i=1;i<=degree;i++) {
		coefficients[i-1] = -ar[degree][i];
	}

	return true;
}

BurgAutoRegression::~BurgAutoRegression() {
	delete[] pef;
	delete[] pef;
	delete[] per;

	delete[] g;
	delete[] h;

	for (int i=0; i<degree+1; i++) {
		delete[] ar[i];
	}
	delete[] ar;
}



/*
int main(int argc, char **argv) {
//	double input[10] = {14, 13, 15, 15, 12, 87, 95, 97, 94, 96};//24,23,25,32,19,27,167,34,26,24};
//	int length = 10;

	//double input[4] = {167,34,26,24};//95, 97, 94, 96, 24,23,25,32,19,27,
	double input[16] = {233759,229692,239948,234007,45713120,229172,223208,72854,222998,227758,46484696,234101,247344,235267,222322,214504};
	int length = 16;

	int method;// = LEASTSQUARES;

	method = atoi(argv[1]);
	AutoRegression *ar = NULL;
	if (method == 0) {
		ar = new YulesWalkerAutoRegression(length);
	} else {
		ar = new BurgAutoRegression(length);
	}

	cout << ar->getNextPrediction(input) << endl;
	return 0;
}
*/
