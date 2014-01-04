/*
 * EventLogger.cpp
 *
 */

#include "EventLogger.h"
#include "ThreadState.h"
#include "MethodData.h"
#include "PerformanceMeasure.h"
#include "DebugUtil.h"
#include "ProfilingInvalidationPolicy.h"
#include "Constants.h"

#include <list>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cassert>

const int EventLogger::TYPICAL_VTF_OUTPUT = 0;
const int EventLogger::HPROF_LIKE_OUTPUT  = 1;

/*************************************************************************
 ****
 **** CONSTRUCTOR / DESTRUCTOR
 ****
 ************************************************************************/
EventLogger::EventLogger() {

	pthread_cond_init(&cond, NULL);
	pthread_mutex_init(&mutex, NULL);
	
	serializedExecutionTime = 0;

	allThreadsInvocationPoints = 0;
	maximumStackDepth = 0;

	prunePercentage = 0.0;	// used to remove results with less than prunePercentage
	totalThreadsControlled = 0;

	startingRealTime = Time::getRealTime();

#ifndef NDEBUG
	remove("/data/vtf_registry.csv");
#endif
}

void EventLogger::setPrunePercentage(double pp) {
	prunePercentage = pp;
}

void EventLogger::reset() {
	measuresLock();
	measures.clear();
	allThreadMeasures.clear();
	allThreadsInvocationPoints = 0;
	measuresUnlock();
}

EventLogger::~EventLogger() {
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&mutex);
}


/*************************************************************************
 ****
 **** LOCK RELATED METHODS
 ****
 ************************************************************************/
int EventLogger::measuresLock() {
	return pthread_mutex_lock(&mutex);
}
int EventLogger::measuresUnlock() {
	return pthread_mutex_unlock(&mutex);
}

void EventLogger::setRegistryStats(const unsigned int & _totalThreads, const unsigned long &_posixSignalsSent, const unsigned long &_allThreadsInvocationPoints) {
	totalThreadsControlled 	= _totalThreads;
	posixSignalsSent 		= _posixSignalsSent;
	allThreadsInvocationPoints = _allThreadsInvocationPoints;
}


/*************************************************************************
 ****
 **** SIMULATION METHOD PROFILING FUNCTIONS
 ****
 ************************************************************************/
/*
 * Method registering function: all methods are registered either prior to loading the JVM
 * or by the classloading (instrumenting) main thread, so no sync needed for registerMethodNames
 */
void EventLogger::registerMethod(const char *_fqName, int methodId) {
	registeredMethodNames[methodId] = new MethodData(_fqName, methodId);
}

void EventLogger::registerMethod(const char *_fqName, int methodId, int method0) {
	registeredMethodNames[methodId] = new MethodData(_fqName, methodId, method0);

//	if (visualizer != NULL) visualizer->registerMethod(_fqName, methodId);
//	vtflog(true, stdout, "Method Registry: adding %d -> %s [%d]\n", methodId, _fqName, method0);
}


/*
 * Get the method id of a registered method from its fqn
 */
int EventLogger::getRegisteredMethodId(const char *_fqName) {

	unordered_map<int, MethodData *>::iterator fqn_iterator = registeredMethodNames.begin();
	MethodData *tempMethod;
	while  (fqn_iterator != registeredMethodNames.end()) {
		tempMethod = (MethodData *)fqn_iterator -> second;
		if (strcmp(tempMethod -> getName(), _fqName) == 0) {
			return (int)fqn_iterator -> first;
		}
		++fqn_iterator;
	}
	return 0;
}


/*
 * Getter
 */
//unordered_map<int, MethodData *> *EventLogger::getRegisteredMethodNames() {
//	return (unordered_map<int, MethodData *> *)(&registeredMethodNames);
//}

const char *EventLogger::getMethodName(const int &methodId) {
	if (registeredMethodNames[methodId] != NULL) {
		return ((MethodData *)registeredMethodNames[methodId])->getName();
	} else {
		std::stringstream str;
		str << "Unknown method " << methodId;
		return str.str().c_str();
	}
}

MethodData *EventLogger::getMethodDataOf(const int &methodId) {
	if (registeredMethodNames[methodId] != NULL) {
		return registeredMethodNames[methodId];
	}
	return NULL;
}

int EventLogger::getMethod0(const int &methodId) {
	if (registeredMethodNames[methodId] != 0) {
		return ((MethodData *)registeredMethodNames[methodId])->getMethodIdOfFirstInstrumentedMethodOfClass();
	}
	return 0;
}

void EventLogger::pmethod(int methodId) {
	if (registeredMethodNames[methodId] != NULL) {

		fprintf(stderr, "%d -> %s\n", methodId, ((MethodData *)registeredMethodNames[methodId])->getName());fflush(stderr);
	} else {
		fprintf(stderr, "Method %d not found\n", methodId);fflush(stderr);
	}
}

void EventLogger::printMethodRegistry(const char *filename, ProfilingInvalidationPolicy *invalidationPolicyUsed) {
	ofstream myfile;
	myfile.open(filename, ios::out);
	unordered_map<int, MethodData *>::iterator it = registeredMethodNames.begin();

	unsigned long total_methods = 0;
	unsigned long recursive_methods = 0;
	unsigned long invalidated_methods = 0;
	myfile << "Id\tName\tInvalidated\tRecursive" << endl;
	while (it != registeredMethodNames.end()) {
		++total_methods;
		MethodData *md = it->second;
		if (md != NULL) {
			if (md->getName() != NULL) {
				myfile << it->first << "\t" << md->getName() << "\t";
			} else {
				myfile << it->first << "\tUnknown method\t";
			}
			if (md->isInvalidated()) {
				myfile << "invalidated";
				++invalidated_methods;
			}
			myfile << "\t";

			if (md->isRecursive()) {
				myfile << "recursive";
				++recursive_methods;
			}
			myfile << endl;
		}
		++it;
	}

	myfile << endl << endl;
	if (invalidationPolicyUsed != NULL) {
		invalidationPolicyUsed -> printInfo(myfile);
	}
	myfile << "Total profiled methods: " << total_methods << endl;
	myfile << "Invalidated methods: " << invalidated_methods << " (" << fixed << setprecision(3) << (100.0 * (float)invalidated_methods/(float)total_methods) << "%)" << endl;
	myfile << "Recursive methods: " << recursive_methods << " (" << fixed << setprecision(3) << (100.0 * (float)recursive_methods/(float)total_methods) << "%)" << endl;
	myfile.close();
}


/*
 * Actions to be taken when the thread with ThreadState state terminates execution
 */
void EventLogger::onThreadEnd(VexThreadState *state) {
	createMeasuresForThread(state);
}


void StackTraceLogger::reset() {
	measuresLock();
	allThreadMeasures.clear();
	allThreadsInvocationPoints = 0;
	measuresRoots = NULL;
	measuresUnlock();
}

/*
 * Actions to be taken when the thread with ThreadState state terminates execution
 */
void StackTraceLogger::onThreadEnd(VexThreadState *state) {
	createMeasuresForThread(state);
}


void StackTraceLogger::outputStackTraceLevel(vector<PerformanceMeasure *> *submethodsMeasures, unsigned int level, ofstream & output) {

	if (submethodsMeasures != NULL) {
		vector<PerformanceMeasure *>::iterator measureIt = submethodsMeasures->begin();

		while (measureIt != submethodsMeasures->end()) {
			PerformanceMeasure *measure = (PerformanceMeasure *)(*measureIt);
			//				assert(measure != NULL);
			double percentage = (100.0 * ((double)(measure->getCpuTimeIncludingCalleeMethods())/(double)serializedExecutionTime));

			if (percentage > prunePercentage && measure != NULL) {
				for (unsigned int i =0; i<level; i++) {
					output << "\t";
				}

				output << fixed << setprecision(2) << percentage << "% - ";
				Time::concisePrinting(output, measure->getCpuTimeIncludingCalleeMethods(), false);

				if (measure->getMethodName() != NULL) {
					output << " - " << measure->getMethodInvocations() << " inv " << measure->getMethodName() << " [" << measure->getMethodId() << "] ";
				} else {
					output << " - " << measure->getMethodInvocations() << " inv Unknown method [" << measure->getMethodId() << "] ";
				}

				output << "," << fixed << setprecision(3) << *measure << endl;

				outputStackTraceLevel(measure->getSubMethods(), level+1, output);
			}
			++measureIt;
		}

	}
}


void StackTraceLogger::toHTML(vector<PerformanceMeasure *> *submethodsMeasures, unsigned int & lastNodeId, ofstream & output) {

	if (submethodsMeasures != NULL) {
		vector<PerformanceMeasure *>::iterator measureIt = submethodsMeasures->begin();
		char tempMethodName[1024];

		while (measureIt != submethodsMeasures->end()) {
			PerformanceMeasure *measure = (PerformanceMeasure *)(*measureIt);
			//				assert(measure != NULL);
			double percentage = (100.0 * ((double)(measure->getCpuTimeIncludingCalleeMethods())/(double)serializedExecutionTime));

			if (percentage > prunePercentage) {
				output << "<li class=\"dhtmlgoodies_sheet.gif\"><a href=\"#\" id=\"node_" << lastNodeId++ << "\">";

				output << fixed << setprecision(2) << percentage << "% - ";
				Time::concisePrinting(output, measure->getCpuTimeIncludingCalleeMethods(), true);
				output << " - " << measure->getMethodInvocations() << " inv ";


				if (measure->getMethodName() != NULL) {
					//						cout << measure->getMethodName() << " ==> ";
					size_t strlength = strlen(tempMethodName);

					strcpy(tempMethodName, measure->getMethodName());

					//						cout << strlength << " " << tempMethodName << endl;

					for (size_t z = 0; z<strlength; z++) {
						if (tempMethodName[z] == '<' || tempMethodName[z] == '>') {
							tempMethodName[z] = '|';
						}
					}
					output << tempMethodName << endl;
				} else {
					output << "Unknown method";
				}

				//				output << fixed << setprecision(40) << " (" << measure->myStackTraceId << ", " << measure->callingMethodStackTraceId << "," << fixed << setprecision(0) << measure->getMethodId() << ") ";
				//				if (measure->getSubMethods() != NULL) {
				//					output << measure->getSubMethods()->size() << " submethods ";
				//				}

				output << "</a>" << endl;

				if (measure->getSubMethods() != NULL) {
					output << "<ul>";
					toHTML(measure->getSubMethods(), lastNodeId, output);
					output << "</ul>";
				}
				output << "</li>";
			}
			++measureIt;
		}

	}
}



/*
 * Comparator function
 */ 
bool compare_PerformanceMeasure(PerformanceMeasure* first, PerformanceMeasure* second) {
	if (first == NULL && second == NULL)
		return true;
	if (first == NULL)
		return false;
	if (second == NULL)
		return false;

	double firstVal = first->getMeanERT();
	double secondVal = second->getMeanERT();
	if (firstVal > secondVal)
		return true;

	return false;
}

/*
 * Use only the times in a method (and not its called methods)
 */
bool compare_PerformanceMeasureExclusiveTimes(PerformanceMeasure* first, PerformanceMeasure* second) {
	if (first == NULL && second == NULL)
		return true;
	if (first == NULL)
		return false;
	if (second == NULL)
		return false;

	long long firstVal = first->getCpuTimeExcludingCalleeMethods();
	long long secondVal = second->getCpuTimeExcludingCalleeMethods();
	if (firstVal > secondVal)
		return true;

	return false;
}

bool compare_PerformanceMeasureInvocations(PerformanceMeasure* first, PerformanceMeasure* second) {
	if (first == NULL && second == NULL)
		return true;
	if (first == NULL)
		return false;
	if (second == NULL)
		return false;

	long long firstVal = first->getMethodInvocations();
	long long secondVal = second->getMethodInvocations();
	if (firstVal > secondVal)
		return true;

	return false;
}


bool compare_PerformanceMeasureTotalTime(PerformanceMeasure* first, PerformanceMeasure* second) {
	if (first == NULL && second == NULL)
		return true;
	if (first == NULL)
		return false;
	if (second == NULL)
		return false;

	long long firstVal = first->getCpuTimeIncludingCalleeMethods();//IncludingCalleeMethods();
	long long secondVal = second->getCpuTimeIncludingCalleeMethods();//IncludingCalleeMethods();
	if (firstVal > secondVal)
		return true;

	return false;
}

/************
 * 
 * MAIN OUTPUT FUNCTIONS
 * 
 * writeData: Stores execution results into a file
 * Exports all measurements into a .csv file
 * writeMethods: Stores methods according to method invocation percentage
 ***/
long long EventLogger::createMeasures() {
	/*
	unordered_map<int, PerformanceMeasure*>* localThreadMethods;
	unordered_map<int, PerformanceMeasure*>::iterator it;
	unordered_map<int, PerformanceMeasure*>::iterator it_main;
	unordered_map<int, string *>::iterator methodNamesIterator;

	measuresLock();
	
	unordered_map<long, unordered_map<int, PerformanceMeasure*>* >::iterator alit = allThreadMeasures.begin();

	PerformanceMeasure *pm1, *pm2;
	int methodId;
	while (alit != allThreadMeasures.end()) {

		localThreadMethods = alit->second;
		it = localThreadMethods->begin();
		while (it != localThreadMethods->end()) {
			methodId = (int)it->first;
			pm1 = (PerformanceMeasure*)it->second;

			pm1->setMethodId(methodId);

			it_main = measures.find(methodId);
			if (it_main != measures.end()) {
				pm2 = (PerformanceMeasure*)it_main->second;
				(*pm2) += (*pm1);
				delete pm1;
			} else {

				methodNamesIterator = registeredMethodNames.find(methodId);
				if (methodNamesIterator != registeredMethodNames.end()) {
					pm1->methodName = (string *)methodNamesIterator->second;
					measures[methodId] = pm1; 	// just copy the first performanceMeasure you found for this method
				}
			}
			it++;
		}
		delete localThreadMethods;
		localThreadMethods = NULL;	// so that not thread accidentally writes the data again
		alit++;

	}

	measuresUnlock();
*/
	return serializedExecutionTime;

}

void EventLogger::createMeasuresForThread(VexThreadState *state) {
	unordered_map<int, PerformanceMeasure*>* threadMeasures = state->getMethodMeasures();

	unordered_map<int, PerformanceMeasure*>::iterator it;
	unordered_map<int, PerformanceMeasure*>::iterator it_main;
	unordered_map<int, MethodData *>::iterator methodNamesIterator;

	// DO NOT HOLD SCHEDULER LOCK WHILE DOING THIS
	measuresLock();

	PerformanceMeasure *pm1, *pm2;
	int methodId;
	if (threadMeasures != NULL) {

		it = threadMeasures->begin();
		while (it != threadMeasures->end()) {
			methodId = (int)it->first;
			pm1 = (PerformanceMeasure*)it->second;
			if (pm1 != NULL) {
				pm1->setMethodId(methodId);

				it_main = measures.find(methodId);
				if (it_main != measures.end()) {
					pm2 = (PerformanceMeasure*)it_main->second;
					(*pm2) += (*pm1);
					delete pm1;	// remove the original thread-local performance measure
					pm1 = NULL;
				} else {
					methodNamesIterator = registeredMethodNames.find(methodId);
					if (methodNamesIterator != registeredMethodNames.end()) {
						MethodData *methodData = (MethodData *)(methodNamesIterator->second);
						if (methodData != NULL) {
							pm1->setMethodName(methodData->getName());
						} else {
							pm1->setMethodName(getMethodName(methodId));
						}
						measures[methodId] = pm1; 	// just copy the first performanceMeasure you found for this method
					}
				}

			}
			it++;
		}

		delete threadMeasures;
		threadMeasures = NULL;
	}

	measuresUnlock();

}



void StackTraceLogger::mergeMeasures(vector<PerformanceMeasure *> &globalNode, vector<PerformanceMeasure *> *threadNode) {

	if (threadNode != NULL) {
//		if (globalNode == NULL) {
//			globalNode = *threadNode;
//		} else {
			vector<PerformanceMeasure *>::iterator threadIter = threadNode->begin();
			while (threadIter != threadNode->end()) {
				int threadNodeId = (*threadIter)->getMethodId();
				vector<PerformanceMeasure *>::iterator globalIter = globalNode.begin();
				bool found = false;
				while (globalIter != globalNode.end()) {
					PerformanceMeasure *globalPm = (*globalIter);
					if (globalPm->getMethodId() == threadNodeId) {
						*globalPm += *(*threadIter);

						if (globalPm->getMethodName() == NULL) {
							globalPm->setMethodName(getMethodName(threadNodeId));
						}

						assert(globalPm->getMethodName() != NULL);
						if (globalPm->getSubMethods() != NULL) {
							mergeMeasures(*(globalPm->getSubMethods()), (*threadIter)->getSubMethods());
						} else {
							globalPm->setSubMethods((*threadIter)->getSubMethods());
						}
						found = true;

//						if ((*threadIter)->canBeCleanedUp()) {
							delete (*threadIter);						// remove the original thread-local performance measure
							(*threadIter) = NULL;
//						} else {
//							(*threadIter)->setMethodName(globalPm->getMethodName());
//						}

						break;
					}
					++globalIter;//					(*threadIter)->setCallingMethod(globalPm->getCallingMethod());

				}

				if (!found) {
//					(*threadIter)->setCallingMethod(globalPm->getCallingMethod());
					if ((*threadIter)->getMethodName() == NULL) {
						(*threadIter)->setMethodName(getMethodName(threadNodeId));
					}
					assert((*threadIter)->getMethodName() != NULL);
					globalNode.push_back((*threadIter));
				}
				++threadIter;
			}
//		}
	}
}

void StackTraceLogger::createMeasuresForThread(VexThreadState *state) {
	vector<PerformanceMeasure*>* threadMeasureRoots = ((StackTraceThreadState *)state)->getRootStackTraceInfos();

	measuresLock();
	if (measuresRoots == NULL) {
		measuresRoots = threadMeasureRoots;
	} else {
		mergeMeasures(*measuresRoots, threadMeasureRoots);
	}
	measuresUnlock();

/*

	PerformanceMeasure *pm1, *pm2;
	int methodId;
	long double traceId;

	if (threadMeasures != NULL) {
		it = threadMeasures->begin();
		while (it != threadMeasures->end()) {

			traceId = (long double)it->first;
			pm1 = (PerformanceMeasure*)it->second;
			methodId = pm1->getMethodId();

			if (pm1 != NULL) {
				it_main = measures.find(traceId);
				if (it_main != measures.end()) {
					pm2 = (PerformanceMeasure*)it_main->second;
					(*pm2) += (*pm1);
					delete pm1;						// remove the original thread-local performance measure
					pm1 = NULL;

				} else {

					methodNamesIterator = registeredMethodNames.find(methodId);
					if (methodNamesIterator != registeredMethodNames.end()) {
						pm1->methodName = (string *)methodNamesIterator->second;
						measures[traceId] = pm1; 	// just copy the first performanceMeasure you found for this method
					}
				}

			}
			++it;
		}

		delete threadMeasures;
		threadMeasures = NULL;
	}
	measuresUnlock();
*/
}


void EventLogger::writeData(const char *filename, const int &outputFormat, long long estimatedExecutionTime) {
	createMeasures();

	if (outputFormat == EventLogger::HPROF_LIKE_OUTPUT) {
		writeDataHprofStyle(filename, estimatedExecutionTime);
	} else {
		writeDataVTFStyle(filename, estimatedExecutionTime);
	}
}


/*
 * Output VTF data in a format similar to the one used by Hprof, like:
 * rank   	self  	accum   	count 	trace 	method
 * 1 		80.88% 	80.88%      2 		301093 	simples.parallelCpuPerfTester.myLoops
 *
 * We have no traces, otherwise the same
 *
 */
void EventLogger::writeDataHprofStyle(const char *filename, long long estimatedExecutionTime) {
	ofstream myfile;
	if (filename) {
		myfile.open(filename);
		if (myfile.fail()) {
			fprintf(stderr, "Problem opening output file %s. Exiting", filename);
			fflush(stderr);
			return;
		}
	} else {
		return;
	}

	serializedExecutionTime = 0;
	list<PerformanceMeasure*> outputList;
	unordered_map<int, PerformanceMeasure*>::iterator it = measures.begin();
	while (it != measures.end()) {
		PerformanceMeasure *measure = it->second;
		serializedExecutionTime += measure->getCpuTimeExcludingCalleeMethods();

		outputList.push_back(it->second);
		it++;
	}
	outputList.sort(compare_PerformanceMeasureExclusiveTimes);


	myfile << setw(4) << "rank" << setw(12) << "self%" << setw(12) << "accum%" << setw(10) << "count" << "  " << "method" << endl;
	int rank = 1;
	list<PerformanceMeasure*>::iterator list_it = outputList.begin();
	double percentage_of_sum_t = 0;
	while (list_it != outputList.end()) {
		PerformanceMeasure *measure = *list_it;

		double percentage = (100.0 * ((double)(measure->getCpuTimeExcludingCalleeMethods())/(double)serializedExecutionTime));
		percentage_of_sum_t += percentage;

		myfile << setw(4) << rank++ << setw(12) << fixed << setprecision(2) << percentage << setw(12) << percentage_of_sum_t << setw(10) << measure->getMethodInvocations() << "  " << measure->getMethodName() << endl;
		++list_it;
	}
	myfile.close();

}


void EventLogger::printExperimentLegend(ofstream &myfile, const long long &estimatedExecutionTime, const unsigned long &summedInvocations) {

	myfile << "Simulation execution time in ms,"  << fixed << setprecision(3) << (Time::getRealTime() - startingRealTime)/1e6 << endl;
	myfile << "Estimated Execution Time (EET) in ms," << fixed << setprecision(3) << estimatedExecutionTime/1e6 << endl;
	myfile << "Aggregate-Serialized Execution Time (AET) in ms," << fixed << setprecision(3) << serializedExecutionTime/1e6 << endl;
	myfile << "Average Parallelism (AET/EET)," << (double)serializedExecutionTime/(double)estimatedExecutionTime << endl;
	Time::printDelaysToFile(myfile);

	myfile << "Maximum stack depth," << maximumStackDepth << endl;
	myfile << "Threads controlled," << totalThreadsControlled << endl;
	myfile << "VEX signals sent," << posixSignalsSent << endl;
	myfile << "Total profiled method invocations," << summedInvocations;

	myfile << "-" << allThreadsInvocationPoints << endl;
	myfile << endl;

}


/*
 * Typical VTF output
 * Rank						: profiler ranking
 * Method					: method full name
 * Invocations				: how many the method was invoked
 * self%					: percent of method ERT on AET time
 * accum%					: self% until this point
 * CPU Time					: average VT from all method invocations 	(excluding callee method times)
 * Estimated Real Time		: average ERT from all method invocations	(excluding callee method times)
 * CPU Time (incl)			: average VT from all method invocations 	(including callee method times)
 * Estimated Real Time (incl):average ERT from all method invocations 	(including callee method times)
 * Average response time	: ERT / invocations in seconds
 *
 * Estimated Execution Time (EET): time of execution of all methods from all threads, taking into account parallelism
 * Aggregate Execution Time (AET): serialized time of execution of all methods from all threads (>= estimated execution time, > if we have parallelism)
 * Average Parallelism 	   		 : AET / EET
 */
void EventLogger::writeDataVTFStyle(const char *filename, long long estimatedExecutionTime) {
	unordered_map<int, PerformanceMeasure*>::iterator it = measures.begin();
	list<PerformanceMeasure*> outputList;
	serializedExecutionTime = 0;
	while (it != measures.end()) {
		PerformanceMeasure *measure = it->second;
		serializedExecutionTime += measure->getCpuTimeExcludingCalleeMethods();
		outputList.push_back(it->second);
		++it;
	}

	unsigned long summedInvocations = 0;

	ofstream myfile;

	//sort by mean execution time
	outputList.sort(compare_PerformanceMeasure);
	if (filename) {
		myfile.open(filename);
		if (myfile.fail()) {
			fprintf(stderr, "Problem opening output file %s. Exiting", filename);
			fflush(stderr);
			return;	
		}
	} else {
		return;
	}

	myfile << ",*Mean* per method times in ms" << endl;
	myfile << "Rank,Method name,Invocations,self%,accum%,Total time,CPU Time,Waiting,I/O,Runnable,Total time-inclusive,CPU Time-inclusive,Waiting-inclusive,I/O-inclusive,Runnable-inclusive" << endl;

	std::string temp;
	std::string method;
	std::string thread;

	int count;

	outputList.sort(compare_PerformanceMeasureExclusiveTimes);
	count = 0;
	double summedPercentage = 0;

	list<PerformanceMeasure*>::iterator list_it = outputList.begin();
	while (list_it != outputList.end()) {
		PerformanceMeasure *measure = *list_it;

		if (!measure->isTheInstrumentationProfilingMethod()) {
			// Method name

			myfile << (++count)<< ","<< measure->getMethodName() << "," << measure->getMethodInvocations() << "," ;
			summedInvocations += measure->getMethodInvocations();
			double percentage = (100.0 * ((double)(measure->getCpuTimeExcludingCalleeMethods())/(double)serializedExecutionTime));
			summedPercentage += percentage;

			myfile << fixed << setprecision(2) << percentage << "," << summedPercentage << "," << *measure;
//			measure->printSamples(myfile);
			myfile <<endl;
		}
		++list_it;
	}


	myfile << endl;
	printExperimentLegend(myfile, estimatedExecutionTime, summedInvocations);

	myfile.close();

}



unsigned long StackTraceLogger::sortSubmethodsAndGetSamples(vector<PerformanceMeasure *> *methodsMeasures) {
	unsigned long totalMethods = 0;
	if (methodsMeasures != NULL) {
		std::sort(methodsMeasures->begin(), methodsMeasures->end(), compare_PerformanceMeasureTotalTime);
		vector<PerformanceMeasure *>::iterator vectorIt = methodsMeasures->begin();
		while (vectorIt != methodsMeasures->end()) {
			totalMethods += (*vectorIt)->getMethodInvocations();

			(*vectorIt)->setMethodName(getMethodName((*vectorIt)->getMethodId()));
			totalMethods += sortSubmethodsAndGetSamples((*vectorIt)->getSubMethods());
			++vectorIt;
		}
	}
	return totalMethods;
}

/*
 * VTF output for stack trace mode
 * Rank						: profiler ranking
 * Trace					: method full name
 * Invocations				: how many the method was invoked
 * self%					: percent of method ERT on AET time
 * accum%					: self% until this point
 * CPU Time					: average VT from all method invocations 	(excluding callee method times)
 * Estimated Real Time		: average ERT from all method invocations	(excluding callee method times)
 * CPU Time (incl)			: average VT from all method invocations 	(including callee method times)
 * Estimated Real Time (incl):average ERT from all method invocations 	(including callee method times)
 * Average response time	: ERT / invocations in seconds
 *
 * Estimated Execution Time (EET): time of execution of all methods from all threads, taking into account parallelism
 * Aggregate Execution Time (AET): serialized time of execution of all methods from all threads (>= estimated execution time, > if we have parallelism)
 * Average Parallelism 	   		 : AET / EET
 */
void StackTraceLogger::writeDataVTFStyle(const char *filename, long long estimatedExecutionTime) {

	serializedExecutionTime = estimatedExecutionTime;

	unsigned long summedInvocations = sortSubmethodsAndGetSamples(measuresRoots);

	long long aggregateRootLevelCpuTime = 0;
	long long aggregateRootLevelEstimatedRealTime = 0;

	if (measuresRoots != NULL) {
		vector<PerformanceMeasure *>::iterator viter = measuresRoots->begin();
		while (viter != measuresRoots->end()) {
			assert((*viter) != NULL);
			aggregateRootLevelCpuTime 			+= (*viter)->getCpuTimeIncludingCalleeMethods();
			aggregateRootLevelEstimatedRealTime += (*viter)->getEstimatedRealTimeIncludingCalleeMethods();
			++viter;
		}

		serializedExecutionTime = aggregateRootLevelCpuTime;
		// TODO: cannot determine pre-main execution time because we don't know which root called whom
//		long long preMainExecutionTotal = estimatedExecutionTime - aggregateRootLevelEstimatedRealTime;
//		cout << estimatedExecutionTime << " " << aggregateRootLevelEstimatedRealTime << endl;
//		if (preMainExecutionTotal > 0) {
//			PerformanceMeasure *premainTime = new PerformanceMeasure(0);
//			premainTime -> add(preMainExecutionTotal, preMainExecutionTotal, preMainExecutionTotal, preMainExecutionTotal);
//			premainTime -> methodName = "Pre-main execution";
//			measuresRoots -> push_back(premainTime);
//		}
//		serializedExecutionTime = aggregateRootLevelCpuTime + preMainExecutionTotal;

		std::sort(measuresRoots->begin(), measuresRoots->end(), compare_PerformanceMeasureTotalTime);

	}

	ofstream myfile;

	//sort by mean execution time
	if (filename) {
		myfile.open(filename);
		if (myfile.fail()) {
			fprintf(stderr, "Problem opening output file %s. Exiting", filename);
			fflush(stderr);
			return;
		}
	} else {
		return;
	}


	myfile << ",*Mean* per stack trace times in ms" << endl;
	myfile << "Stack trace (self% - Total aggr. time - Invocations),Total time,CPU Time,Waiting,I/O,Runnable,Total time-inclusive,CPU Time-inclusive,Waiting-inclusive,I/O-inclusive,Runnable-inclusive" << endl;

	outputStackTraceLevel(measuresRoots, 0, myfile);

	myfile << endl;

	serializedExecutionTime = aggregateRootLevelCpuTime;
	printExperimentLegend(myfile, estimatedExecutionTime, summedInvocations);

	myfile.close();




	ofstream myfile2;
	char htmlfilename[256];
	sprintf(htmlfilename, "%s.html", filename);
	//sort by mean execution time

	myfile2.open(htmlfilename);
	if (myfile2.fail()) {
		fprintf(stderr, "Problem opening output file %s. Exiting", htmlfilename);
		fflush(stderr);
		return;
	}

	myfile2 << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n<html>\n<head>\n<title>VTF results</title>\n<link rel=\"stylesheet\" href=\"/homes/nb605/VTF/htmlresults/css/folder-tree-static.css\" type=\"text/css\">\n	<link rel=\"stylesheet\" href=\"/homes/nb605/VTF/htmlresults/css/context-menu.css\" type=\"text/css\">\n<script type=\"text/javascript\" src=\"/homes/nb605/VTF/htmlresults/js/folder-tree-static.js\"></script>\n<script type=\"text/javascript\" src=\"/homes/nb605/VTF/htmlresults/js/context-menu.js\"></script>\n</head>\n<body>\n<p>Experiment results</p>\n<ul id=\"dhtmlgoodies_tree2\" class=\"dhtmlgoodies_tree\"><a href=\"#\" onclick=\"expandAll('dhtmlgoodies_tree2');return false\">Expand all</a>\n<a href=\"#\" onclick=\"collapseAll('dhtmlgoodies_tree2');return false\">Collapse all</a>\n<br><br>" << endl;
	unsigned int nodeId =0;
	toHTML(measuresRoots, nodeId, myfile2);
	myfile2 << "</ul>" << endl;
	myfile2 << "<script type=\"text/javascript\">\ninitContextMenu();\n</script>\n</body>\n</html>" << endl;
	myfile2.close();

}


StackTraceLogger::~StackTraceLogger() {

}


/*
 * Write methods
 *
 * Output the profiled methods together with the percentage of invocation points and their invocation points count
 */
void EventLogger::writeMethods(const char *filename) {

	std::string method;
	std::string temp;

	//createMeasures();
	unordered_map<int, PerformanceMeasure*>::iterator it = measures.begin();

	list<PerformanceMeasure*> outputList;
	double sum = 0.0;
	while (it != measures.end()) {
		outputList.push_back(it->second);
		sum += it->second->getMethodInvocations();
		it++;
	}
	//sort by mean execution time
	outputList.sort(compare_PerformanceMeasureInvocations);

	ofstream myfile;

	if (filename) {
		myfile.open(filename);
		if (myfile.fail()) {
			fprintf(stderr, "Problem opening output file %s. Exiting", filename);
			fflush(stderr);
			return;
		}
	} else {
		return;
	}

	myfile << "#Method FQN	Invocation percentage	Invocations" << endl;
	std::string thread;

	list<PerformanceMeasure*>::iterator list_it = outputList.begin();
	while (list_it != outputList.end()) {
		PerformanceMeasure *measure = *list_it;

		// Method name
		myfile << measure->getMethodName() << "," << ((measure->getMethodInvocations()/sum)*100) << "%," << measure->getMethodInvocations() << endl;

		++list_it;
	}

	myfile.close();

}
