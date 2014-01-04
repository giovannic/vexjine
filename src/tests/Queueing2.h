/*
 * PerformanceTest.h
 *
 *  Created on: 12 Nov 2011
 *      Author: root
 */

#ifndef QUEUEING_H_
#define QUEUEING_H_

#include <cstdlib>
#include <cmath>
#include <pthread.h>
#include <deque>
#include <iostream>
#include "AgentTester.h"


using namespace std;
class QueueExp  {

public:

  QueueExp( double r ) {
    rate = r ;
  }

  double next() {
    return -log( (double)rand() / (double)RAND_MAX) / rate ;
  }

  static double exp( double lam ) {
    return -log( (double)rand() / (double)RAND_MAX) / lam ;
  }

private:
  double rate;

};

class Job {
public:
	Job() {
		serviced = false;
		pthread_mutex_init(&lock, NULL);
		pthread_cond_init(&cond, NULL);
	}

	void waitToBeServiced() {
		if(AgentTester::virtualExecution) {
			//int monitorId = 0; memcpy(&monitorId, this, sizeof(int));
			threadEventsBehaviour->onRequestingLock((int &)lock);
		}
		pthread_mutex_lock(&lock);

		while (!serviced) {
			if(AgentTester::virtualExecution) {
				//int monitorId = 0; memcpy(&monitorId, this, sizeof(int));
				if (threadEventsBehaviour->beforeTimedWaitingOn((int &)lock, true)) {

					pthread_mutex_unlock(&lock);
					threadEventsBehaviour->onTimedWaitingOn((int &)lock, 0, 0, true);	// stay blocked in here until the virtual timeout expires
					pthread_mutex_lock(&lock);
				}
			} else {
				pthread_cond_wait(&cond, &lock);
			}

		}
		pthread_mutex_unlock(&lock);
	}

	void setServiced() {
		if(AgentTester::virtualExecution) {
			//int monitorId = 0; memcpy(&monitorId, this, sizeof(int));
			threadEventsBehaviour->onRequestingLock((int &)lock);
		}
		pthread_mutex_lock(&lock);
		serviced = true;

		if(AgentTester::virtualExecution) {
			//int monitorId = 0; memcpy(&monitorId, this, sizeof(int));
			threadEventsBehaviour->onSignallingOnObject((int &)lock);
		}
		pthread_cond_signal(&cond);

		if(AgentTester::virtualExecution) {
			//int monitorId = 0; memcpy(&monitorId, this, sizeof(int));
			threadEventsBehaviour->onReleasingLock((int &)lock);
		}
		pthread_mutex_unlock(&lock);
	}

private:
    bool serviced;
	pthread_mutex_t lock;
	pthread_cond_t cond;
};



class ThreadSafeQueue {
public:
	ThreadSafeQueue() {
		queue = new deque<Job *>();
		pthread_mutex_init(&lock, NULL);
		pthread_cond_init(&cond, NULL);
	}

	Job *getNextJob() {
		Job *job;
		if(AgentTester::virtualExecution) {
			//int monitorId = 0; memcpy(&monitorId, this, sizeof(int));
			threadEventsBehaviour->onRequestingLock((int &)lock);
		}
		pthread_mutex_lock(&lock);

		while (queue->empty()) {
			if(AgentTester::virtualExecution) {
				//int monitorId = 0; memcpy(&monitorId, this, sizeof(int));
				if (threadEventsBehaviour->beforeTimedWaitingOn((int &)lock, true)) {

					pthread_mutex_unlock(&lock);
					threadEventsBehaviour->onTimedWaitingOn((int &)lock, 0, 0, true);	// stay blocked in here until the virtual timeout expires
					pthread_mutex_lock(&lock);
				}
			} else {
				pthread_cond_wait(&cond, &lock);
			}
		}
		job = queue->front();
		queue->pop_front();
		//System.out.print(queue.size());

		if(AgentTester::virtualExecution) {
			//int monitorId = 0; memcpy(&monitorId, this, sizeof(int));
			threadEventsBehaviour->onReleasingLock((int &)lock);
		}
		pthread_mutex_unlock(&lock);
		return job;
	}

	void submitJob(Job *job) {

		if(AgentTester::virtualExecution) {
			//int monitorId = 0; memcpy(&monitorId, this, sizeof(int));
			threadEventsBehaviour->onRequestingLock((int &)lock);
		}
		pthread_mutex_lock(&lock);

		queue->push_back(job);

		// VEX INSTRUMENTATION: Note here how that signal is replaced altogether
		if(AgentTester::virtualExecution) {
			//int monitorId = 0; memcpy(&monitorId, this, sizeof(int));
			threadEventsBehaviour->onSignallingOnObject((int &)lock);
		} else {

			pthread_cond_signal(&cond);
		}

		if(AgentTester::virtualExecution) {
			//int monitorId = 0; memcpy(&monitorId, this, sizeof(int));
			threadEventsBehaviour->onReleasingLock((int &)lock);
		}
		pthread_mutex_unlock(&lock);
	}

private:
	deque<Job *> *queue;
	pthread_mutex_t lock;
	pthread_cond_t cond;
};


class Server {
public:
	/** Initialize with the requests per second - Was 400000 */
	Server(int rps, ThreadSafeQueue * q) {
		jobSizeDist = new QueueExp(((double) 1.0 / (double)rps));
		queue = q;
	}

	/**
	 * Starts servicing removing jobs from the queue and servicing them.
	 *
	 * @param q
	 */
	void start(string threadName) {
		Job *job;
		while (true) {
			job = queue->getNextJob();
			serviceRequest(job);
			job->setServiced();

		}
	}

	/**
	 * Submits a request to the queue, then waits until the job is serviced.
	 */
	void serviceRequest(Job *job) {
		if (job != NULL) {
			int length = sampleJobSize();
			doWork(M_PI / 2, length);
		}
	}

	void doWork(double y, int iterations) {
		double x = y, a = 0;
		while (a < iterations) {
			a += sin(x);
			x += cos(x);
		}
	}

	/**
	 * Get a sample of the jobs size.
	 *
	 */
	int sampleJobSize() {
		double serviceTime = jobSizeDist->next();
		if (serviceTime < 1) {
			serviceTime = 1;
		}
		return (int) (serviceTime);
	}


private:
	/** The queue of waiting jobs. */
	ThreadSafeQueue * queue;

	/** The job size distribution. */
	QueueExp *jobSizeDist;


};


class Client {
private:
	/**The queue of waiting jobs.*/
    ThreadSafeQueue *queue;

    /** Inter-arrival time distribution.*/
    QueueExp *interArrivalDist;

public:
    /**
     * Create a test harness with the given mean inter-arrival time.
     * @param mean_interArrivalTime
     */
    Client(double mean_interArrivalTime, ThreadSafeQueue *q) {
        interArrivalDist = new QueueExp((double)1 / mean_interArrivalTime);
        queue = q;
    }

    void think() {
		double nextThinkTime = sampleInterArrivalTime();
//		long thinkingTimeMilliSeconds = (long)nextThinkTime;
//		int thinkingTimeNanoSeconds = (int)((nextThinkTime - thinkingTimeMilliSeconds) * 1000000);
		timespec delay, rem;
		delay.tv_sec = (int)(nextThinkTime/1000) ;
		delay.tv_nsec = nextThinkTime * 1000000 - (delay.tv_sec * 1e9);	//500,000 = 1/2 milliseconds
		//cout << "will be sleeping for " << nextThinkTime << " = " << delay.tv_sec << "," << delay.tv_nsec << endl;

		// VEX INSTRUMENTATION: Note here how sleeping is replaced
		if(AgentTester::virtualExecution) {
			//int monitorId = 0; memcpy(&monitorId, this, sizeof(int));
			long thinkingTimeMilliSeconds = (long)nextThinkTime;
			int thinkingTimeNanoSeconds = (int)((nextThinkTime - thinkingTimeMilliSeconds) * 1000000);
			threadEventsBehaviour->onSleepingStart(thinkingTimeMilliSeconds, thinkingTimeNanoSeconds);
		} else {
			nanosleep(&delay, &rem);
		}
//		usleep(thinkingTimeMilliSeconds, thinkingTimeNanoSeconds);
    }


    /**
     * Submits a request, waits until the job is serviced before returning.
     * The execution time of this method is the response time for the queue.
     */
    void makeRequest() {
    	Job *job = new Job();
//    	System.out.println("new request!");
    	queue->submitJob(job);
    	job->waitToBeServiced();
    }

    /**
     * Returns the next inter-arrival time.
     * @return
     */
	double sampleInterArrivalTime() {
        return interArrivalDist->next();
    }

};
#endif /* QUEUEING_H_ */
