/*
 * PerformanceTest.h
 *
 *  Created on: 12 Nov 2011
 *      Author: root
 */

#ifndef PERFORMANCETEST_H_
#define PERFORMANCETEST_H_

class PerformanceTest {
public:
	PerformanceTest();
	virtual void run(int loops) = 0;
	virtual ~PerformanceTest();
};

class MethodsPerformanceTest : public PerformanceTest {
public:
	void run(int loops);
};

class IoPerformanceTest : public PerformanceTest {
public:
	void run(int loops);
};

class InstrLoopsTest : public PerformanceTest {
public:
	void run(int loops);
private:
	virtual double getNext(int i);
	double e;
};

class LoopsTest : public PerformanceTest {
public:
	void run(int loops);
protected:
	double e;
	virtual double getNext(int i);

};

class InstrLoopsTest2 : public InstrLoopsTest {
protected:
	virtual double getNext(int i);
};

class LoopsTest2 : public LoopsTest {
protected:
	virtual double getNext(int i);
};


class InstrLoopsTest3 : public InstrLoopsTest {
protected:
	virtual double getNext(int i);
};

class LoopsTest3 : public LoopsTest {
protected:
	virtual double getNext(int i);
};
#endif /* PERFORMANCETEST_H_ */
