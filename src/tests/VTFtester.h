/*
 * VTFtester.h
 *
 *  Created on: 12 Oct 2010
 *      Author: root
 */

#ifndef VTFTESTER_H_
#define VTFTESTER_H_

#include "Tester.h"

class VTFtester: public Tester {
public:
	VTFtester();
	virtual ~VTFtester();

	string testName();
	bool test();
private:
	bool optionsParsing();
};

#endif /* VTFTESTER_H_ */
