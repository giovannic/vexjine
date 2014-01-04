/*
 * test.cpp
 *
 *  Created on: 9 Jun 2011
 *      Author: nb605
 */

#include "PapiProfiler.h"

#include <iostream>
#include <cmath>
#include <time.h>
#include <cstdlib>
#include <sys/time.h>
#include <pthread.h>

using namespace std;

static PapiProfiler *profiler;

double getE(int iterations) {
	double temp = 0.0;
	int k;
	for (double m =1 ; m<iterations; m++) {
		k = (rand() % iterations);
		temp += pow(1.0 + 1.0/m, m) + k;
	}

	return temp/(double)iterations;
}


double getE(double *predefinedElements, int size) {
	double temp = 0.0;
	int k;
	for (int m =1 ; m<size; m++) {
		k = (rand() % size);
		temp += pow(1.0 + 1.0/predefinedElements[m], predefinedElements[m]) + k;
	}

	return temp/(double)size;
}


double getEstrange(double *predefinedElements, int size) {
	double temp = 0.0;
	int k;
	for (int m =1 ; m<size; m++) {
		k = (rand() % size);
		temp += pow(1.0 + 1.0/predefinedElements[k], predefinedElements[k]) + k;
	}

	return temp/(double)size;
}



int myLoops(const float &limit) {

	double temp = 0;

	for (double m = 1; m<limit; m++) {
		temp += pow(1.0 + 1.0/m, m);		//e
	}

	int results = 100*temp/limit;

	return results;

}

void *threadRoutine(void *args) {
//	profiler->startMeasurement();
	profiler->onThreadStart();
	float limit = atof((char *)args);

	int result = myLoops(limit);
	cout << "is this the result " << result << endl;	// approx ~ e

	profiler->onThreadEnd();
//	profiler->stopMeasurement();

	pthread_exit(NULL);
	return NULL;
}


int main(int argc, char **argv) {

	if (argc < 4) {
		cerr << "Syntax error: ./test <threads> <limit> <mem_to_alloc in MB>" << endl;
		return -1;
	}
	srand(time(NULL));

	timeval start,end, diff;

	profiler = new PapiProfiler(true);

	cout << profiler->getHardwareInfo() << endl;

	profiler->onThreadStart();
	gettimeofday(&start, NULL);

	int threads = atoi(argv[1]);
	pthread_t pthreads[threads];
	for (int i = 0; i<threads;i++){
		pthread_create(&pthreads[i], NULL, threadRoutine, (void *)argv[2]);
	}

	for (int i = 0; i<threads;i++){
		pthread_join(pthreads[i], NULL);
	}

	cout << "Main allocating memory..." << endl;
    unsigned int memLength = atoi(argv[3]) * 1048576;
    cout << "Allocating %d memory..." << memLength << endl;
    unsigned char* p = new unsigned char[memLength];

    for (unsigned int i =0 ; i< memLength; i++) {
    	int k = rand() % memLength;
    	char v = rand() % 256;
    	p[k] = v;
    }
//    while (true) {
//        int i = rand() % memLength;
//        char v = rand() % 256;
//        p[i] = v;
//    }
    delete[] p;
	gettimeofday(&end, NULL);
	timersub(&end, &start, &diff);
//	cout << diff.tv_sec << "." << setw(6) << setfill('0') << diff.tv_usec << endl;
	cout << diff.tv_sec << "." << diff.tv_usec << endl;


//	profiler->stopMeasurement();
//	cout << profiler->getTotalMeasurements() << endl;

/*
	int size = atoi(argv[1]);
	profiler->startMeasurement();

	cout << "No array: " << getE(size) << endl;
	profiler->stopMeasurement();

	double *v = new double[size];
	for (int i =0 ; i<size; i++) {
		v[i] = i;
	}

	profiler->startMeasurement();
	cout << "Array: " << getEstrange(v, size) << endl;
	profiler->stopMeasurement();
*/

	profiler->onThreadEnd();
        profiler->getTotalMeasurements() ;

	delete profiler;



	return 0;
}
