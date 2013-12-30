//
//	Last Modified: $Date: 2010-07-20 09:48:13 $
//
//	$Log: Log.cpp,v $
//	Revision 1.2  2010-07-20 09:48:13  lgrave
//	corrected windows crlf to unix lf
//

//	Revision 1.1  2010-07-19 23:40:44  lgrave

//	1st version added to cvs

//
//

#include <time.h>
#include <iostream>
#include <fstream>
#include <string>

#include "Log.h"

void Log::Write(const std::string &prefix, const std::string &msg, int level)
{
	if(level > log_level)
	{
		// no message should be logged
		return;
	}
	time_t ltime = time(NULL);
	char buffer[TA_LOGBUFFER_SIZE];
	strftime(buffer, TA_LOGBUFFER_SIZE, TA_LOGDATETIMEFORMAT, localtime(&ltime));
	log_file.open(log_filename.c_str(), std::fstream::out | std::fstream::app);
	log_file << buffer << TA_LOGSEPARATOR;
	if(prefix.size() != 0)
	{
		log_file << prefix << TA_LOGSEPARATOR;
	}
	log_file << msg << std::endl;
	log_file.close();
}

Log::Log(std::string file, int level) : log_level(level)
{
	log_filename = file;
	log_file.exceptions(std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit);
	log_file.open(log_filename.c_str(), std::fstream::out | std::fstream::app);
	log_file.close();
	if(level < TA_LOGLEVEL_FATAL || level > TA_LOGLEVEL_DEBUG)
	{
		// invalid log level; assume TA_LOGLEVEL_DEFAULT
		log_level = TA_LOGLEVEL_DEFAULT;
	}	
}

void Log::Fatal(const std::string &msg)
{
	Write(TA_LOGSTR_FATAL, msg, TA_LOGLEVEL_FATAL);
}
void Log::Error(const std::string &msg)
{
	Write(TA_LOGSTR_ERROR, msg, TA_LOGLEVEL_ERROR);
}
void Log::Warning(const std::string &msg)
{
	Write(TA_LOGSTR_WARNING, msg, TA_LOGLEVEL_WARNING);
}
void Log::Default(const std::string &msg)
{
	Write(TA_LOGSTR_DEFAULT, msg, TA_LOGLEVEL_DEFAULT);
}
void Log::Details(const std::string &msg)
{
	Write(TA_LOGSTR_DETAILS, msg, TA_LOGLEVEL_DETAILS);
}
void Log::Debug(const std::string &msg)
{
	Write(TA_LOGSTR_DEBUG, msg, TA_LOGLEVEL_DEBUG);
}

Log *theLog = NULL;


