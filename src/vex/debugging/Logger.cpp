#include "Logger.h"
#include "VirtualTimeline.h"
#include "Time.h"

#include <iomanip>

Log::Log(const char *filename, VirtualTimeline *_gvt) {
	virtualTimeline = _gvt;
	startingTime = Time::getRealTime();
	os.open(filename, std::ios::out);
}

std::ofstream& Log::Get(TLogLevel level) {
	//os << setw(12) << (globalTimer->getRealTime()-startingTime) << ": ";
	os << std::string(level > logDEBUG ? level - logDEBUG : 0, '\t');
	messageLevel = level;
	return os;
}

void Log::closeNow() {
	os.close();
}

Log::~Log() {
	os.close();
}

/*
int main(int argc, char **argv) {
	Log *test = new Log("mytestlog.txt");
	LOG(test, logERROR) << "Hello woo" << endl;
	LOG(test, logDEBUG4) << "Hello worl4" << endl;
	LOG(test, logDEBUG1) << "Hello world2" << endl;

}
*/
