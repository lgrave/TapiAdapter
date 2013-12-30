//
//	Last Modified: $Date: 2010-07-20 09:48:13 $
//
//	$Log: Log.h,v $
//	Revision 1.2  2010-07-20 09:48:13  lgrave
//	corrected windows crlf to unix lf
//

//	Revision 1.1  2010-07-19 23:40:44  lgrave

//	1st version added to cvs

//
//

#ifndef TA_LOG_H
#define TA_LOG_H

#define xBEGIN	theLog->Debug(format("%ld: %s(%d): %s begin", GetCurrentThreadId(), __FILE__, __LINE__, __FUNCDNAME__));
#define xEND	theLog->Debug(format("%ld: %s(%d): %s end", GetCurrentThreadId(), __FILE__, __LINE__, __FUNCDNAME__));
#define _x		theLog->Debug(format("%ld: %s(%d): %s", GetCurrentThreadId(), __FILE__, __LINE__, __FUNCDNAME__));

#define ASS(cond)	if(!(cond)) theLog->Error(format("ASS %s FAILED", #cond));


#include <iostream>
#include <fstream>
#include <string>
#include <stdarg.h>

#include "safewindows.h"

#define TA_LOGLEVEL_FATAL		0
#define TA_LOGLEVEL_ERROR		1
#define TA_LOGLEVEL_WARNING		2
#define TA_LOGLEVEL_DEFAULT		3
#define TA_LOGLEVEL_DETAILS		4
#define TA_LOGLEVEL_DEBUG		5

#define TA_LOGSTR_FATAL			"FATAL ERROR"
#define TA_LOGSTR_ERROR			"ERROR"
#define TA_LOGSTR_WARNING		"WARNING"
#define TA_LOGSTR_DEFAULT		""
#define TA_LOGSTR_DETAILS		""
#define TA_LOGSTR_DEBUG			"DEBUG"

#define TA_LOGBUFFER_SIZE		512

#define TA_LOGDATETIMEFORMAT	"%Y-%m-%d %H:%M:%S"
#define TA_LOGSEPARATOR			": "

typedef char *ERRSTR;


class Log
{
	std::ofstream log_file;
	std::string log_filename;
	int log_level;
	
	void Write(const std::string &prefix, const std::string &msg, int level);
	
public:
	
	Log(std::string file, int level);
	
	void Fatal(const std::string &msg);
	void Error(const std::string &msg);
	void Warning(const std::string &msg);
	void Default(const std::string &msg);
	void Details(const std::string &msg);
	void Debug(const std::string &msg);
};

extern Log *theLog;


inline std::string format(const char *fmt, ...)
{
    va_list va; 
    va_start(va, fmt);

    char buffer[2048];
    vsprintf(buffer, fmt, va);

    va_end(va);
    
    return buffer;
}

inline std::string scan(const char *src, const char *fmt)
{
	char buffer[2048] = {0};

	sscanf(src, fmt, buffer);
	
	return buffer;
}


#endif //TA_LOG_H