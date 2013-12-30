//
//	Last Modified: $Date: 2010-07-22 10:16:37 $
//
//	$Log: Settings.h,v $
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

#ifndef TA_SETTINGS_H
#define TA_SETTINGS_H

#include <string>

#define TA_SETTINGS_INIFILE						"TapiAdapter.ini"

#define TA_SETTINGS_REGPATH						"SOFTWARE\\lgrave\\TapiAdapter\\"

#define TA_SETTINGS_GENERIC						"generic"
#define TA_SETTINGS_GENERIC_LOGLEVEL			"log_level"
#define TA_SETTINGS_GENERIC_LOGFILE				"log_file"
#define TA_SETTINGS_GENERIC_CENTRALID			"central_id"
#define TA_SETTINGS_GENERIC_EXTENSIONIDENTIFIER	"extension_identifier"
#define TA_SETTINGS_GENERIC_LINESTOMONITOR		"lines_to_monitor"
#define TA_SETTINGS_COMMANDS					"commands"
#define TA_SETTINGS_COMMANDS_LISTENPORT			"listen_port"
#define TA_SETTINGS_COMMANDS_THREADS			"threads"
#define TA_SETTINGS_COMMANDS_USERNAME			"username"
#define TA_SETTINGS_COMMANDS_PASSWORD			"password"
#define TA_SETTINGS_EVENTS						"events"
#define TA_SETTINGS_EVENTS_SERVERURL			"server_url"
#define TA_SETTINGS_EVENTS_SERVERNAME			"server_name"
#define TA_SETTINGS_EVENTS_SERVERPORT			"server_port"
#define TA_SETTINGS_EVENTS_METHODNAME			"method_name"

#define TA_SETTINGS_BUFFERSIZE					1024
#define TA_SETTINGS_DEFAULT						""

std::string IniReadStr(const std::string &section, const std::string &key);

int IniReadInt(const std::string &section, const std::string &key);



#endif //TA_SETTINGS_H
