#include "network/Customer.h"
#include "Network.h"

#include "Check.h"
#include "SimulationTime.h"

int Customer::customerId = 0;
#include <sstream>
#include <cassert>

using namespace std;

Customer::Customer() {
	id = customerId++ ;
	serviceDemand = 0.0;
	queueInsertionTime= 0.0;
	priority = 0 ;
	location = Network::nullNode;
	this->type = 0;
	finished = false;
	arrivalTime = SimulationTime::now() ;
}

Customer::Customer( int type ) {
	id = customerId++ ;
	queueInsertionTime= 0.0;
	serviceDemand = 0.0;
	priority = 0 ;
	location = Network::nullNode;
	Check::check( type >= 0 && type <= Network::maxClasses, "ERROR: Customer class out of bounds" );
	this->type = type;
	finished = false;
	arrivalTime = SimulationTime::now() ;
}

Customer::Customer( int type, int priority ) {
	location = Network::nullNode;
	queueInsertionTime= 0.0;
	serviceDemand = 0.0;
	id = customerId++ ;
    Check::check( type >= 0 && type <= Network::maxClasses, "ERROR: Customer class out of bounds" ) ;//id
    Check::check( priority >= 0 && priority <= Network::maxPriorities, "ERROR: Customer priority out of bounds" ) ;
    this->type = type ;
	this->priority = priority ;
	finished = false;
	arrivalTime = SimulationTime::now() ;
}

Customer::~Customer() {

}


string Customer::toString() {
	stringstream str;
	str << "Customer " << id << " (class " << type << ", priority " << priority << ")";
    return (str.str()) ;
}

int Customer::getId() {
	return id ;
}

double Customer::getArrivalTime() {
	return arrivalTime ;
}

void Customer::setServiceDemand( double d ) {
	serviceDemand = d ;
}

double Customer::getServiceDemand() {
	return serviceDemand ;
}

void Customer::setQueueInsertionTime( double d ) {
	queueInsertionTime = d ;
}

double Customer::getQueueInsertionTime() {
	return queueInsertionTime ;
}

CinqsNode *Customer::getLocation() {
	return location ;
}

void Customer::setLocation( CinqsNode *n ) {
	location = n ;
}

int Customer::getclass() {
	return type ;
}

void Customer::setclass( int t ) {
	type = t ;
}

int Customer::getPriority() {
	return priority ;
}

void Customer::setPriority( int p ) {
	priority = p ;
}

//
// Implement abstract method from Ordered. By default, the ordering
// in based on the customer's priority.
//
bool Customer::smallerThan( Customer *e ) {
	return type <= e->getclass() ;
}


// Methods added here for elegance (to avoid casting to subclass)
void Customer::lock() {}
void Customer::unlock() {}

const char *Customer::getThreadName() {
	return "thread";
}
// There is a distinction in the way we handle remote and local waits:
// remote waits should not be synchronized to GVT upon waking up
bool Customer::service(const long &time) {return true;}
bool Customer::blockAtQueue(pthread_mutex_t *) {return true;}
void Customer::resume(const long long &resumeTime) {}
bool Customer::waitRemote(const long &duration) {return true;}

long long Customer::getEstimatedRealTime() {
	cout << "Should never try to access ERT of simple customer object" << endl;
	assert(false);
	return 0;
}
void Customer::setEstimatedRealTime(const long long &estimatedRealTime) {}
void Customer::setFinished() {}
void Customer::clearFinishedFlag() {}
