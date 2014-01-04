#ifndef NETWORK_CUSTOMER_H
#define NETWORK_CUSTOMER_H

#include "Ordered.h"

#include <pthread.h>
#include <string>

class CinqsNode;

class Customer : public Ordered {
public:
	int getclass();
	void setclass( int t );

public:
	Customer() ;
	virtual ~Customer() ;
	Customer( int type ) ;
	Customer( int type, int priority ) ;
	std::string toString() ;
	int getId() ;
	double getArrivalTime() ;
	void setServiceDemand( double d ) ;
	double getServiceDemand() ;
	void setQueueInsertionTime( double d ) ;
	double getQueueInsertionTime() ;
	CinqsNode *getLocation() ;
	int getPriority() ;
	void setPriority( int p ) ;
	bool smallerThan( Customer *e ) ;

	virtual void lock();
	virtual void unlock();

	virtual const char *getThreadName();
	virtual bool service(const long &time);
	virtual bool blockAtQueue(pthread_mutex_t *);
	virtual void resume(const long long &resumeTime);
	virtual bool waitRemote(const long &duration);

	virtual void setEstimatedRealTime(const long long &estimatedRealTime);
	virtual long long getEstimatedRealTime();
	virtual void setFinished();
	virtual void clearFinishedFlag();

	virtual void setLocation( CinqsNode *n ) ;

	bool isFinished() {
		return finished;
	};
protected:
	CinqsNode *location;
	bool finished;

private:
	int id ;
	int type ;
	int priority ;
	double arrivalTime ;
	double serviceDemand ;
	double queueInsertionTime ;

	static int customerId ;
};

#endif
