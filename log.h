#ifndef _LOG_H_
#define _LOG_H_

#ifdef USE_CONSOLEKIT
#include "Ck.h" 
#endif
#ifdef USE_PAM
#include "PAM.h"
#endif
#include "const.h"
#include <fstream>

using namespace std;

static class LogUnit {
	ofstream logFile;
public:
	bool openLog(const char * filename);
	void closeLog();

	~LogUnit() { closeLog(); }

	template<typename Type>
	LogUnit & operator<<(const Type & text) {
		logFile << text; logFile.flush();
		return *this;
	}

	LogUnit & operator<<(ostream & (*fp)(ostream&)) {
		logFile << fp; logFile.flush();
		return *this;
	}

	LogUnit & operator<<(ios_base & (*fp)(ios_base&)) {
		logFile << fp; logFile.flush();
		return *this;
	}
} logStream;

#endif /* _LOG_H_ */
