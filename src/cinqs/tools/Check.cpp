#include "tools/Check.h"

#include <cassert>

using namespace std;

void Check::check( bool check, const string &message ) {
	if (!check ) {
		cout <<  "\nERROR: " << message << endl;
		assert(false);
	}
}
