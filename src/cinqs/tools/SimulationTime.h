/*
 * Time.h
 *
 *  Created on: 19 May 2011
 *      Author: root
 */

#ifndef SIMULATIONTIME_H_
#define SIMULATIONTIME_H_

class SimulationTime {

public:

	static double now() {
		return currentTime;
	}
	static double *timePtr() {
		return &currentTime;
	}

private:
	static double currentTime;
};

#endif /* SIMULATIONTIME_H_ */
