/*
 * JVMStatisics.cpp: File to capture the transitions between lower-JINE methods
 *
 *  Created on: 11 May 2011
 *      Author: nb605
 */

#include "JVMStatistics.h"
#include <fstream>
#include <cstring>
#include <unistd.h>

using namespace std;

unsigned int jineMethodTransitions[JINE_TOTAL_METHODS][JINE_TOTAL_METHODS];
unsigned int jineMethodTransitions_currentMethodCalled;

void initializeJineMethodTransitionsMap() {
	for (int i=0; i<JINE_TOTAL_METHODS; i++) {
		for (int j=0; j<JINE_TOTAL_METHODS; j++) {
			jineMethodTransitions[i][j] = 0;
		}
	}
	jineMethodTransitions_currentMethodCalled = 0;
}

/*
 * The output is printed out as a dot file that is then converted into a PostScript one
 */
void printJineMethodTransitionsMap(const char *filename) {
	ofstream dotOutput;

	char threadFilename[strlen(filename)+6];

	sprintf(threadFilename, "%s.dot", filename);
	dotOutput.open(threadFilename, ios::out);

	string methodNames[JINE_TOTAL_METHODS] = {"CALLBACKVMINIT", "RESETPERFORMANCEMEASURES", "WRITEPERFORMANCEMEASURES", "GETTHREADVIRTUALTIME", "GETPAPIREALTIME", "GETTIMEBEFORECREATINGTHREAD", "GETTIMEAFTERCREATINGTHREAD", "LOGJOININGTHREAD", "STARTTIMING", "ENDTIMING", "GETVTFTIME", "GETVTFTIMEINMILLIS", "SETMETHODENTRYDELAY", "SETINTERACTIONPOINTDELAY", "SETTIMEMONITORING", "SETINSTRUMENTATIONTIME", "SIMULATEIODURATION", "ENABLESTACKTRACEMODE", "ONINSTRUMENTATIONSTART", "ONINSTRUMENTATIONEND", "INVALIDATEPROFILINGOF", "REINSTRUMENTRECURSIVEMETHOD", "REGISTERIOINVOCATIONPOINT", "REGISTERMETHOD", "REGISTERMETHODVIRTUALTIME", "REGISTERINTERPRETEDMODE", "REGISTERLOGGERBARRIER", "REGISTERPROFILINGINVALIDATIONPOILCY", "REGISTERTHREAD", "INTERACTIONPOINT", "YIELD", "NOTIFYTIMEDWAITINGTHREADS", "NOTIFYALLTIMEDWAITINGTHREADS", "INTERRUPTONVIRTUALTIMEOUT", "BEFORETHREADINTERRUPT", "REGISTERTHREADBEINGSPAWNED", "SETWAITING", "UNSETWAITING", "CALLBACKTHREADSTART", "CALLBACKMONITORWAIT", "CALLBACKMONITORWAITED", "CALLBACKMONITORCONTENDEDENTER", "CALLBACKMONITORCONTENDEDENTERED", "CALLBACKTHREADEND", "CALLBACKGCSTART", "CALLBACKGCFINISH", "CALLBACKVMDEATH", "REQUESTINGLOCKS", "RELEASINGLOCKS"};

	dotOutput << "digraph jine_method_transition_graph {" << endl;

	for (int i =0; i<JINE_TOTAL_METHODS; i++) {
		for (int j =0; j<JINE_TOTAL_METHODS; j++) {
			if (jineMethodTransitions[i][j] != 0) {
				dotOutput << "\t" << methodNames[i] << " -> " << methodNames[j] << " [label=\"" << jineMethodTransitions[i][j] << "\"];" << endl;
			}
		}
	}

	dotOutput << "}" << endl;
	dotOutput.close();

	// Create ps file
	char psFilename[strlen(filename)+6];
	sprintf(psFilename, "-o%s.ps" , filename);
	char dotExecutable[] = {"dot"};
	char outputFlag[] = {"-Tps"};
	char * const arguments[] = {dotExecutable, threadFilename, psFilename, outputFlag, NULL};
	if (fork() == 0) {
		execv( "/usr/bin/dot", arguments);
	}
}
