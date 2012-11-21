#include "log.h"
#include <iostream>

bool
LogUnit::openLog(const char * filename)
{
	if (logFile.is_open()) {
		cerr << APPNAME
			<< ": opening a new Log file, while another is already open"
			<< endl;
		logFile.close();
	}
	logFile.open(filename, ios_base::app);

	return !(logFile.fail());
}

void
LogUnit::closeLog()
{
	if (logFile.is_open())
		logFile.close();
}
