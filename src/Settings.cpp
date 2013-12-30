//
//	Last Modified: $Date: 2010-07-22 10:16:37 $
//
//	$Log: Settings.cpp,v $
//	Revision 1.2.2.1  2010-07-22 10:16:37  lgrave
//	RT8: TapiAdapter: threads for reception/execution of paralel commands
//
//	Revision 1.2  2010-07-20 09:48:13  lgrave
//	corrected windows crlf to unix lf
//
//	Revision 1.1  2010-07-19 23:40:44  lgrave
//	1st version added to cvs
//
//

#include <string>
#include <stdlib.h>
#include "safewindows.h"

#include "Settings.h"


std::string IniReadStr(const std::string &section, const std::string &key)
{
	char buffer[TA_SETTINGS_BUFFERSIZE];
	GetPrivateProfileString(section.c_str(), key.c_str(), TA_SETTINGS_DEFAULT, buffer, TA_SETTINGS_BUFFERSIZE, TA_SETTINGS_INIFILE);
	return buffer;
}

int IniReadInt(const std::string &section, const std::string &key)
{
	return atoi(IniReadStr(section, key).c_str());
}


