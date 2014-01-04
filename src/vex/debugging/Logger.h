
/*
 * Logger.h: Global VEX logger class used in combination with compiler flags:
 * - if DISABLE_LOGGING is enabled (-DDISABLE_LOGGING=1) then all logging is disabled
 * - otherwise logging messages are printed to the vtf_manager_log.
 * The logging levels are used to distinguish how the messages are printed,
 * not which of them are displayed
 *
 *  Created on: 28 Mar 2011
 *      Author: root
 */

#include <fstream>

#ifndef LOGGER_H_
#define LOGGER_H_
enum TLogLevel {logERROR, logWARNING, logINFO, logDEBUG, logDEBUG1, logDEBUG2, logDEBUG3, logDEBUG4};

class VirtualTimeline;	// forward declaration

class Log
{
public:
	Log(const char *filename, VirtualTimeline *_globalTimer);
	virtual ~Log();
	std::ofstream& Get(TLogLevel level = logINFO);
	void closeNow();

	static TLogLevel ReportingLevel() {
		return logDEBUG4;
	};

protected:
	std::ofstream os;

	Log(const Log&);
	Log& operator =(const Log&);
	TLogLevel messageLevel;
	long long startingTime;
	VirtualTimeline *virtualTimeline;
};


#ifndef DISABLE_LOGGING
#if GDB_USAGE == 1
#define LOG(logger, level) \
if (level > Log::ReportingLevel()) ; \
else logger->Get(level)
#else
#define LOG(logger, level) \
if (true) ; \
else logger->Get(level)
#endif

#else
#define LOG(logger, level) \
if (true) ; \
else logger->Get(level)
#endif


#endif /* LOGGER_H_ */
