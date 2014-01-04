/*
 * PredictionMethod.cpp
 *
 *  Created on: 16 Aug 2011
 *      Author: root
 */

#include "PredictionMethod.h"

#include <cstdlib>
#include <ctime>
#include <sstream>
#include <cassert>

#ifdef USING_GSL
#include <gsl/gsl_histogram.h>
#else
struct GslNotUsedException : std::exception {
	const char *what() {return "GSL library not used in this installation of VEX.";}
};
#endif

PredictionMethod::PredictionMethod() {
	predictionLog = NULL;
}

PredictionMethod::~PredictionMethod() {

}


void PredictionMethod::import(char *filenamePrefix, IoState &ioState) {
	stringstream str;
	str << filenamePrefix;
	ioState.getIoPointFileSuffix(str);
	str << ".csv";
	importFromFile(str.str());
}

void PredictionMethod::exportMeasurements(const char *outputFilenamePrefix, IoState &ioState) {
	stringstream str;
	str << outputFilenamePrefix;
	ioState.getIoPointFileSuffix(str);
	str << ".csv";
	exportToFile(str.str());
}



CyclicBufferBased::CyclicBufferBased(const unsigned int &_circular_buffersize) {
	size  = 0;
	index = 0;
	circular_buffersize = _circular_buffersize;
	buffer = new long long[circular_buffersize];
	for (unsigned int i = 0; i<circular_buffersize; i++) {
		buffer[i] = 0;
	}
	predictionLog = NULL;
}

void CyclicBufferBased::importFromFile(const std::string &filename) {
	std::string line;
	ifstream previousResultsFile(filename.c_str());

	if (previousResultsFile.is_open()) {
		while (! previousResultsFile.eof() ) {
			getline (previousResultsFile, line);
			buffer[(size++) % circular_buffersize] = atoll(line.c_str());
		}
		previousResultsFile.close();
	} else {
		//cout << "Warning: No file " << filename << " found for previous predictions parsing" << endl;
	}
}

void CyclicBufferBased::exportToFile(const std::string &filename) {
	std::filebuf fb;
	fb.open(filename.c_str(), std::ios::out);
	std::ostream os(&fb);
	for (unsigned int i = 0; i<circular_buffersize; i++) {
		os << buffer[i] << endl;
	}
	fb.close();
}



long long CyclicBufferBased::getNext() {
	if (size > circular_buffersize) {
		long long prediction = getValue();
		if (predictionLog != NULL) {
			predictionLog->logOnIoStart(prediction, NULL, -1);
		}
		return prediction;
	} else {
		return 0;
	}
}

void CyclicBufferBased::addPredictionInfo(const long long &realTimeValueOnExit, VexThreadState *state, const unsigned int &threadsInIo) {
	buffer[(size++) % circular_buffersize] = realTimeValueOnExit;
	if (predictionLog != NULL) {
		predictionLog->logOnIoEnd(realTimeValueOnExit, state, threadsInIo);
	}
}

CyclicBufferBased::~CyclicBufferBased() {
	delete buffer;
}

ReplayCyclicBuffer::~ReplayCyclicBuffer() {

}

long long AverageCyclicBuffer::getAverage() {
	int max = getCurrentSize();

	long long sum = 0;
	for (int i = 0; i < max; i++) {
		sum += buffer[i];
	}
	return sum/max;
}

long long AverageCyclicBuffer::getValue() {
	long long average = getAverage();
	progressIndex();
	return average;

}
AverageCyclicBuffer::~AverageCyclicBuffer() {

}

/*
 * Qsort implementation from http://theory.stanford.edu/~amitp/rants/c++-vs-c/test4.c
 */
void MedianCyclicBuffer::sort(long long int *data, int N) {
  int i, j;
  long v, t;

  if(N <= 1) return;

  // Partition elements
  v = data[0];
  i = 0;
  j = N;

  for(;;) {
    while(data[++i] < v && i < N) { }
    while(data[--j] > v) { }
    if(i >= j) break;
    t = data[i]; data[i] = data[j]; data[j] = t;
  }

  t = data[i-1]; data[i-1] = data[0]; data[0] = t;
  sort(data, i-1);
  sort(data+i, N-i);

}


long long MedianCyclicBuffer::getValue() {
	long long tempValue;
	int max = getCurrentSize();
	if (max == 0) {
		return 0;
	}

	for (unsigned int i = 0; i<circular_buffersize; i++) {
		tempSortingBuffer[i] = buffer[i];
	}
	sort(tempSortingBuffer, max);

	if (max % 2 == 0) {
		tempValue = (tempSortingBuffer[max/2] + tempSortingBuffer[max/2 -1]) / 2;
	} else {
		tempValue = tempSortingBuffer[(max-1)/2];
	}

	progressIndex();

	return tempValue;
}
MedianCyclicBuffer::~MedianCyclicBuffer() {

}


long long SamplingCyclicBuffer::getStdev(const long long &avg) {
	int max = getCurrentSize();
	long stdev = 0, temp;
	for (int i = 0; i < max; i++) {
		temp = (avg - buffer[i]);
		stdev += (temp * temp);
	}

	stdev = (long)((double)sqrt(stdev)/(double)max);
	return stdev;
}


long long SamplingCyclicBuffer::getValue() {
	long long average = getAverage();
	long long stdev = getStdev(average);

	progressIndex();
	return (long long)generageNormallyDistributedSamples(average, stdev);
}

double SamplingCyclicBuffer::generageNormallyDistributedSamples(double mu, double sigma) {

	static bool deviateAvailable=false;        //        flag
	static float storedDeviate;                        //        deviate from previous calculation
	double polar, rsquared, var1, var2;

	//        If no deviate has been stored, the polar Box-Muller transformation is
	//        performed, producing two independent normally-distributed random
	//        deviates.  One is stored for the next round, and one is returned.
	if (!deviateAvailable) {
		//        choose pairs of uniformly distributed deviates, discarding those
		//        that don't fall within the unit circle
		do {
			var1=2.0*( double(rand())/double(RAND_MAX) ) - 1.0;
			var2=2.0*( double(rand())/double(RAND_MAX) ) - 1.0;
			rsquared=var1*var1+var2*var2;
		} while ( rsquared>=1.0 || rsquared == 0.0);

		//        calculate polar tranformation for each deviate
		polar=sqrt(-2.0*log(rsquared)/rsquared);

		//        store first deviate and set flag
		storedDeviate=var1*polar;
		deviateAvailable=true;

		//        return second deviate
		return var2*polar*sigma + mu;
	} else {
		//        If a deviate is available from a previous call to this function, it is
		//        returned, and the flag is set to false.
		deviateAvailable=false;
		return storedDeviate*sigma + mu;
	}

}

SamplingCyclicBuffer::~SamplingCyclicBuffer() {

}




long long MinimumCyclicBuffer::getValue() {
	int max = getCurrentSize();
	if (max == 0) {
		return 0;
	}

	long long minValue = LONG_MAX;
	for (int i = 0; i < max; i++) {
		if (buffer[i] < minValue) {
			minValue = buffer[i];
		}
	}

	progressIndex();
	return minValue;
}
MinimumCyclicBuffer::~MinimumCyclicBuffer() {

}



long long MaximumCyclicBuffer::getValue() {
	int max = getCurrentSize();
	if (max == 0) {
		return 0;
	}

	long long maxValue = 0;
	for (int i = 0; i < max; i++) {
		if (buffer[i] > maxValue) {
			maxValue = buffer[i];
		}
	}

	progressIndex();
	return maxValue;
}
MaximumCyclicBuffer::~MaximumCyclicBuffer() {

}


/***
 * MarkovModulatedProcess: generate a N-states Markov Process,
 * train it to identify state limits and transition probabilities
 * and then starting from state currentState
 */
void MarkovModulatedProcess::alloc(const unsigned int &_circular_buffersize, unsigned int _states) {
	circular_buffersize = _circular_buffersize;
	states = _states;
	if (states < 2) {
		states = 2;
	}
	if (states > circular_buffersize) {
		states = circular_buffersize;
	}

	transitionMatrix = new double*[states];
	bucketOfElement = new int[circular_buffersize];	// used to copy the measurements for the states of current model
	bucketSizes = new unsigned int[states];
	for (unsigned int i = 0; i<states; i++) {
		transitionMatrix[i] = new double[states];
	}
	currentState = -1;
}


void MarkovModulatedProcess::init() {

	for (unsigned int i = 0; i<states; i++) {
		for (unsigned int j = 0; j < states; j++) {
			transitionMatrix[i][j] = 0;
		}
	}
	for (unsigned int i = 0; i<circular_buffersize; i++) {
		bucketOfElement[i] = -1;
	}

	for (unsigned int i = 0; i<states; i++) {
		bucketSizes[i] = 0;		// while stateLimits[0] is 0, then we have not yet set the state limits
	}

}

MarkovModulatedProcess::~MarkovModulatedProcess() {

	delete[] bucketSizes;
	delete[] bucketOfElement;
	for (unsigned int i = 0; i<states; i++) {
		delete[] transitionMatrix[i];
	}
	delete[] transitionMatrix;
}


void MarkovModulatedProcess::distributeAllInBucket(int bucketId, const int &newBucketsToCreate, const int &lowestBucketId) {

	long long min = LLONG_MAX;
	long long max = 0;

	for (unsigned int i = 0; i<circular_buffersize; i++) {
		if (bucketId == bucketOfElement[i]) {
			if (buffer[i] > max) {
				max = buffer[i];
			}
			if (buffer[i] < min) {
				min = buffer[i];
			}

		}
	}

	long long step = (max - min)/newBucketsToCreate;

	// what if all elements of the bucket are equal - add 1...n ns to all elements to differentiate them
	if (step == 0) {
		int addCount = 0;
		for (unsigned int i = 0; i<circular_buffersize; i++) {
			if (bucketId == bucketOfElement[i]) {
				buffer[i] += addCount;
				++addCount;
			}
		}
		max = min + addCount;
		step = (max - min)/newBucketsToCreate;
	}

	for (unsigned int i = 0; i<circular_buffersize; i++) {
		if (bucketId == bucketOfElement[i]) {
			bucketOfElement[i] = lowestBucketId + (buffer[i]-min) / step;

			if (bucketOfElement[i] >= (lowestBucketId + newBucketsToCreate-1)) {
				bucketOfElement[i] = lowestBucketId + newBucketsToCreate-1;
			}

			if (lowestBucketId == 0) {
				++bucketSizes[bucketOfElement[i]];
			} else {
				if (lowestBucketId != bucketOfElement[i]) {
					++bucketSizes[bucketOfElement[i]];
					--bucketSizes[lowestBucketId];
				}
			}
		}
	}

}

const long long &MarkovModulatedProcess::sampleMeasurementOfState(const int &currentState) {
	int element = rand() % bucketSizes[currentState];
	for (unsigned int k = 0; k<circular_buffersize; k++) {
		if (bucketOfElement[k] == currentState) {
			if (element-- == 0) {
				return buffer[k];
			}
		}
	}

	assert(false);
	return buffer[0];
}

long long MarkovModulatedProcess::getValue() {
	// If getValue() gets called it means that the cyclic buffer is full and the Markov chain can be generated.
//	cout << currentState << " " << size << " " <<circular_buffersize<< endl;
	if (currentState == -1 || ((size-1) % circular_buffersize == 0)) {
//		cout << "initializing" << endl;
		init();
		setStateLimits();
		populateTransitionMatrix();
		currentState = bucketOfElement[circular_buffersize-2];	// TODO: quite arbitrary selection of current state - not selecting the last one to avoid getting stuck in the last state of an acyclic chain
	}
//	cout << "getvalu" << endl;

	long long value = sampleMeasurementOfState(currentState);
	currentState = getRandomTransitionFrom(currentState);
	return value;
}

int MarkovModulatedProcess::getRandomTransitionFrom(const int &_currentState) {
	double r = (double)rand() / (double)RAND_MAX;
	for (unsigned int j = 0; j<states; j++) {
		if (r < transitionMatrix[_currentState][j]) {
			return j;
		}
	}
	return states-1;
}


// Define the state limits: state-i is smaller than stateLimits[i] and greater than stateLimits[i-1] (if i>1)
void MarkovModulatedProcess::setStateLimits() {

	int fullBuckets = 1;
	unsigned int nullBuckets = states - 1;
	int largestBucket = -1;
	do {
		distributeAllInBucket(largestBucket, nullBuckets + 1, fullBuckets  - 1);
		nullBuckets = 0;
		fullBuckets = 0;
		largestBucket = -1;

		for (unsigned int i = 0; i<states-1; i++) {
			if (bucketSizes[i] == 0) {
				for (unsigned int j = states-1; j>i; j--) {
					if (bucketSizes[j] != 0) {
						for (unsigned int k = 0; k<circular_buffersize; k++) {
							if ((bucketOfElement[k] - j) == 0) {
								bucketOfElement[k] = i;
							}
						}
						bucketSizes[i] = bucketSizes[j];
						bucketSizes[j] = 0;
						break;
					}
				}
			}
		}

		for (unsigned int i= 0; i<states; i++) {
			if (bucketSizes[i] == 0) {
				++nullBuckets;
			} else {
				++fullBuckets;
				if (largestBucket == -1 || bucketSizes[i] > bucketSizes[largestBucket]) {
					largestBucket = i;
				}
			}
		}

		if (nullBuckets != 0) {
			for (unsigned int k = 0; k<circular_buffersize; k++) {
				if (bucketOfElement[k] == largestBucket) {
					bucketOfElement[k] = fullBuckets - 1;
				} else if (bucketOfElement[k] == (fullBuckets-1)) {
					bucketOfElement[k] = largestBucket;
				}
			}
			int temp = bucketSizes[largestBucket];
			bucketSizes[largestBucket] = bucketSizes[fullBuckets - 1];
			bucketSizes[fullBuckets - 1] = temp;
			largestBucket = fullBuckets -1;
		}
	} while (nullBuckets != 0);
}


void MarkovModulatedProcess::populateTransitionMatrix() {

	// Set occurences of transitions from state-i to state-j
	for (unsigned int i = 0; i<circular_buffersize -1; i++) {
		++transitionMatrix[bucketOfElement[i]][bucketOfElement[i+1]];
	}


	for (unsigned int i = 0; i<states; i++) {
		unsigned int allTransitions = 0;
		for (unsigned int j = 0; j<states; j++) {
			allTransitions += transitionMatrix[i][j];	// stateValuesLimits need to be saved for sampling previous measurements according to the model state
		}


		double totalProbUpToj = 0;
		for (unsigned int j = 0; j<states; j++) {
			totalProbUpToj += transitionMatrix[i][j] / allTransitions;
			transitionMatrix[i][j] = totalProbUpToj;	// cumulative nature of transition matrix
		}
	}

}

void MarkovModulatedProcess::printTransitionMatrix() {
	cout << "The transition matrix is: " << endl;
	for (unsigned int i = 0; i<states; i++) {
		cout << transitionMatrix[i][0] << " ";
		for (unsigned int j = 1; j < states; j++) {
			cout << (transitionMatrix[i][j] - transitionMatrix[i][j-1]) << " ";
		}
		cout << "\t --> print elements";
		cout << endl;
	}
}


void MarkovModulatedProcess::addPredictionInfo(const long long &realTimeValueOnExit, VexThreadState *state, const unsigned int &threadsInIo) {
	CyclicBufferBased::addPredictionInfo(realTimeValueOnExit, state, threadsInIo);
}



/***
 * Simple linear regression
 */
SimpleLinearRegressionCyclicBuffer::~SimpleLinearRegressionCyclicBuffer() {
	delete regression;
}

long long SimpleLinearRegressionCyclicBuffer::getValue() {
	return regression->apply(buffer);
}

/***
 * Auto regression
 */
long long AutoRegressionCyclicBuffer::getValue() {
	return (long long)ar->getNextPrediction(buffer);
}

/***
 * GSL History Predictions based on previously stored histogram file
 */
GslHistogramPredictor::GslHistogramPredictor() {
	srand(time(NULL));
}
long long GslHistogramPredictor::getNext() {
	long long prediction = 0;
#ifdef USING_GSL
	if (histogram_pdf != NULL) {
		prediction = (long long)gsl_histogram_pdf_sample(histogram_pdf, (double)rand()/RAND_MAX);
	}

	if (predictionLog != NULL) {
		predictionLog->logOnIoStart(prediction, NULL, -1);
	}
#endif
	return prediction;

}
void GslHistogramPredictor::addPredictionInfo(const long long &realTimeValueOnExit, VexThreadState *state, const unsigned int &threadsInIo) {
#ifdef USING_GSL
	if (predictionLog != NULL) {
		predictionLog->logOnIoEnd(realTimeValueOnExit, state, threadsInIo);
	}
#else
	throw GslNotUsedException();
#endif
}

void GslHistogramPredictor::importFromFile(const std::string &filename) {
#ifdef USING_GSL
	FILE *fd = fopen(filename.c_str(), "r");

	if (fd != NULL) {
// 		cout << "Loading GSL-Histogram predictor from " << filename << endl;
		gsl_histogram *hg = gsl_histogram_alloc(74);	// hard-coded bucket value
		gsl_histogram_fread(fd, hg);
		histogram_pdf = gsl_histogram_pdf_alloc(74);
		gsl_histogram_pdf_init(histogram_pdf, hg);
	} else {
//		cout << "Warning: No file " << filename << " found for previous predictions parsing" << endl;
		histogram_pdf = NULL;
	}
#else
	throw GslNotUsedException();
#endif
}

void GslHistogramPredictor::exportToFile(const std::string &filename) {

}

/***
 * Constant mean: always return the mean of all previous measurements
 */
long long ConstantMeanPredictor::getNext() {
	if (size == 0) {
		return 0;
	} else {
		return total/size;
	}
}
void ConstantMeanPredictor::addPredictionInfo(const long long &realTimeValueOnExit, VexThreadState *state, const unsigned int &threadsInIo) {
	total += realTimeValueOnExit;
	++size;
}

void ConstantMeanPredictor::importFromFile(const std::string &filename) {
	std::string line;
	ifstream previousResultsFile(filename.c_str());

	if (previousResultsFile.is_open()) {
		getline (previousResultsFile, line);
		total = atoll(line.c_str());
		getline (previousResultsFile, line);
		size = atoi(line.c_str());
		previousResultsFile.close();
	} else {
//		cout << "Warning: No file " << filename << " found for previous predictions parsing" << endl;
	}

}

void ConstantMeanPredictor::exportToFile(const std::string &filename) {
	std::filebuf fb;
	fb.open(filename.c_str(), std::ios::out);
	std::ostream os(&fb);
	os << total << endl;
	os << size << endl;
	fb.close();
}
