/*
 * RunRandomProgram.h
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#ifndef RUNRANDOMPROGRAM_H_
#define RUNRANDOMPROGRAM_H_

#include "Tester.h"
#include "MethodSimulation.h"


class RunRandomProgram : public Tester {
public:
	RunRandomProgram(int _threads) : threads(_threads) {};
	virtual ~RunRandomProgram();

	bool test();
	string testName();

private:
	int threads;

};

#endif /* RUNRANDOMPROGRAM_H_ */
