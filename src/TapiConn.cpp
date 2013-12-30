//
//	Last Modified: $Date: 2010-07-20 10:36:49 $
//
//	$Log: TapiConn.cpp,v $
//	Revision 1.3  2010-07-20 10:36:49  lgrave
//	turned off OnAddressStateChange events
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
#include <map>
#include <string.h>
#include <set>

#include "TapiConn.h"
#include "Atapi.h"
#include "Settings.h"
#include "Log.h"
#include "XmlRpc.h"
#include "XmlRpcMethods.h"

using namespace XmlRpc;



CMyLine::CMyLine()
{
	init = false;
}

void CMyLine::Init(void)
{
	if(init) return;
	init = true;
	central_id = IniReadStr(TA_SETTINGS_GENERIC, TA_SETTINGS_GENERIC_CENTRALID);
	line_extension = scan((LPCTSTR)GetLineName(), IniReadStr(TA_SETTINGS_GENERIC, TA_SETTINGS_GENERIC_EXTENSIONIDENTIFIER).c_str());
	LPLINEDEVSTATUS p = GetLineStatus();
	line_dev_status = LineDevStatus((unsigned char *)p + p->dwDevSpecificOffset, p->dwDevSpecificSize);
	the_client1 = new XmlRpcClient(IniReadStr(TA_SETTINGS_EVENTS, TA_SETTINGS_EVENTS_SERVERNAME).c_str(), IniReadInt(TA_SETTINGS_EVENTS, TA_SETTINGS_EVENTS_SERVERPORT));
	the_client2 = new XmlRpcClient(IniReadStr(TA_SETTINGS_EVENTS, TA_SETTINGS_EVENTS_SERVERNAME).c_str(), IniReadInt(TA_SETTINGS_EVENTS, TA_SETTINGS_EVENTS_SERVERPORT));
	std::string method = IniReadStr(TA_SETTINGS_EVENTS, TA_SETTINGS_EVENTS_METHODNAME);
	order = 0;
}

void CMyLine::Send(std::map<std::string, std::string> arguments, int client /* = 1 */, std::string oldord /* = "" */)
{
	xBEGIN
	XmlRpcValue args;
	XmlRpcValue result;
	std::string details;
	
	XmlRpcClient *the_client = client == 1 ? the_client1 : the_client2;

	std::string ord = client == 1 ? format("%d", order++) : oldord;
	args["order"] = ord;
	details += "order=>" + ord + ", ";

	for(std::map<std::string, std::string>::iterator it = arguments.begin(); it != arguments.end(); ++it)
	{
		args[it->first] = it->second;
		details += it->first + "=>" + it->second + ", ";
	}
	theLog->Details(details);
	//theLog->Debug(args.toXml());
	
	try
	{
		the_client->execute(method.c_str(), args, result);
	}
	catch(char *msg)
	{
		if(client == 1 && msg == std::string("_executing"))
		{
			theLog->Warning("1st try failed, resending...");
			Send(arguments, 2, ord);
			xEND return;
		}
		theLog->Debug(std::string("XMLRPC ERROR: ") + msg);
	}
	//theLog->Debug(result.toXml());
	xEND
}


void CMyLine::OnAddressStateChange (DWORD dwAddressID, DWORD dwState)
{
// 	Init();
// 
// 	std::string addstate;
// 	switch(dwState)
// 	{
// 		case LINEADDRESSSTATE_OTHER: addstate = "LINEADDRESSSTATE_OTHER"; break;
// 		case LINEADDRESSSTATE_DEVSPECIFIC: addstate = "LINEADDRESSSTATE_DEVSPECIFIC"; break;
// 		case LINEADDRESSSTATE_INUSEZERO: addstate = "LINEADDRESSSTATE_INUSEZERO"; break;
// 		case LINEADDRESSSTATE_INUSEONE: addstate = "LINEADDRESSSTATE_INUSEONE"; break;
// 		case LINEADDRESSSTATE_INUSEMANY: addstate = "LINEADDRESSSTATE_INUSEMANY"; break;
// 		case LINEADDRESSSTATE_NUMCALLS: addstate = "LINEADDRESSSTATE_NUMCALLS"; break;
// 		case LINEADDRESSSTATE_FORWARD: addstate = "LINEADDRESSSTATE_FORWARD"; break;
// 		case LINEADDRESSSTATE_TERMINALS: addstate = "LINEADDRESSSTATE_TERMINALS"; break;
// 		case LINEADDRESSSTATE_CAPSCHANGE: addstate = "LINEADDRESSSTATE_CAPSCHANGE"; break;
// 		default: addstate = "unknown"; break;
// 	}
// 		
// 	std::map<std::string, std::string> args;
// 	args["IC_device_id"] = central_id;
// 	args["event"] = "AddressStateChange";
// 	args["event_detail"] = addstate;
// 	args["address_id"] = format("%ld", dwAddressID);
// 	args["line_extension"] = line_extension;
// 	args["user_extension"] = UserExtension();
// 	args["this"] = format("%p", this);
// 	Send(args);
}
void CMyLine::OnAgentStateChange (DWORD dwAddressID, DWORD dwFields, DWORD dwState){xSHOW("OnAgentStateChange"); xSHOW(line_extension);}
void CMyLine::OnCallInfoChange (HCALL hCall, DWORD dwCallInfoState)
{
	xBEGIN
	Init();
	
	if(dwCallInfoState & LINECALLINFOSTATE_CALLERID)
	{
		CTapiCall *pCall = GetCallFromHandle(hCall);
		if(!pCall)
		{
			theLog->Error("OnCallInfoChange: GetCallFromHandle failed");
			xEND return;
		}
		pCall->GetCallInfo(TRUE);
		std::string caller_id = (LPCTSTR)pCall->GetCallerIDNumber();

		std::map<std::string, std::string> args;
		args["IC_device_id"] = central_id;
		args["event"] = "CallerID";
		args["call_id"] = HCALL2string(hCall);
		args["line_extension"] = line_extension;
		args["user_extension"] = UserExtension();
		args["caller_id"] = caller_id;
		args["caller_id_name"] = (LPCTSTR)pCall->GetCallerIDName();
		args["this"] = format("%p", this);
		Send(args);
	}

	if(dwCallInfoState & LINECALLINFOSTATE_CALLEDID)
	{
		CTapiCall *pCall = GetCallFromHandle(hCall);
		if(!pCall)
		{
			theLog->Error("OnCallInfoChange: GetCallFromHandle failed");
			xEND return;
		}
		pCall->GetCallInfo(TRUE);
		std::string called_id = (LPCTSTR)pCall->GetCalledIDNumber();

		std::map<std::string, std::string> args;
		args["IC_device_id"] = central_id;
		args["event"] = "CalledID";
		args["call_id"] = HCALL2string(hCall);
		args["line_extension"] = line_extension;
		args["user_extension"] = UserExtension();
		args["called_id"] = called_id;
		args["called_id_name"] = (LPCTSTR)pCall->GetCalledIDName();
		args["this"] = format("%p", this);
		Send(args);
	}
	xEND
}

void CMyLine::OnCallStateChange (HCALL hCall, DWORD dwState, DWORD dwStateDetail, DWORD dwPrivilage)
{
	xBEGIN
	Init();

	std::string callstate;
	switch(dwState)
	{
		case LINECALLSTATE_IDLE: callstate = "LINECALLSTATE_IDLE"; break;
		case LINECALLSTATE_OFFERING: callstate = "LINECALLSTATE_OFFERING"; break;
		case LINECALLSTATE_ACCEPTED: callstate = "LINECALLSTATE_ACCEPTED"; break;
		case LINECALLSTATE_DIALTONE: callstate = "LINECALLSTATE_DIALTONE"; break;
		case LINECALLSTATE_DIALING: callstate = "LINECALLSTATE_DIALING"; break;
		case LINECALLSTATE_RINGBACK: callstate = "LINECALLSTATE_RINGBACK"; break;
		case LINECALLSTATE_BUSY: callstate = "LINECALLSTATE_BUSY"; break;
		case LINECALLSTATE_SPECIALINFO: callstate = "LINECALLSTATE_SPECIALINFO"; break;
		case LINECALLSTATE_CONNECTED: callstate = "LINECALLSTATE_CONNECTED"; break;
		case LINECALLSTATE_PROCEEDING: callstate = "LINECALLSTATE_PROCEEDING"; break;
		case LINECALLSTATE_ONHOLD: callstate = "LINECALLSTATE_ONHOLD"; break;
		case LINECALLSTATE_CONFERENCED: callstate = "LINECALLSTATE_CONFERENCED"; break;
		case LINECALLSTATE_ONHOLDPENDCONF: callstate = "LINECALLSTATE_ONHOLDPENDCONF"; break;
		case LINECALLSTATE_ONHOLDPENDTRANSFER: callstate = "LINECALLSTATE_ONHOLDPENDTRANSFER"; break;
		case LINECALLSTATE_DISCONNECTED: callstate = "LINECALLSTATE_DISCONNECTED"; break;
		case LINECALLSTATE_UNKNOWN: callstate = "LINECALLSTATE_UNKNOWN"; break;
		default: callstate = "unknown"; break;
	}
		
	std::map<std::string, std::string> args;
	args["IC_device_id"] = central_id;
	args["event"] = "CallStateChange";
	args["event_detail"] = callstate;
	args["call_id"] = HCALL2string(hCall);
	args["line_extension"] = line_extension;
	args["user_extension"] = UserExtension();
	args["this"] = format("%p", this);
	Send(args);
	xEND
}
void CMyLine::OnClose(){xSHOW("OnClose");}
void CMyLine::OnDevSpecific (DWORD dwHandle, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3){xSHOW("OnDevSpecific"); xSHOW(line_extension);}
void CMyLine::OnDevSpecificFeature (DWORD dwHandle, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3){xSHOW("OnDevSpecificFeature"); xSHOW(line_extension);}
void CMyLine::OnGatherDigitsComplete (HCALL hCall, DWORD dwReason){xSHOW("OnGatherDigitsComplete"); xSHOW(line_extension);}
void CMyLine::OnGenerateComplete (HCALL hCall, DWORD dwReason){xSHOW("OnGenerateComplete"); xSHOW(line_extension);}
void CMyLine::OnDeviceStateChange (DWORD dwDeviceState, DWORD dwStateDetail1, DWORD dwStateDetail2)
{
	xBEGIN
	Init();

	std::string linestate;
	LineDevStatus line_dev_status_OLD(line_dev_status);
	LPLINEDEVSTATUS p = GetLineStatus();
	if(!p)
	{
		theLog->Error("OnDeviceStateChange: GetLineStatus failed");
		xEND return;
	}
	line_dev_status = LineDevStatus((unsigned char *)p + p->dwDevSpecificOffset, p->dwDevSpecificSize);
	
	if(dwDeviceState & LINEDEVSTATE_OUTOFSERVICE)
	{
		dwDeviceState &= ~LINEDEVSTATE_OUTOFSERVICE;
		
		std::map<std::string, std::string> args;
		args["IC_device_id"] = central_id;
		args["event"] = "Logging_OFF";
		args["line_extension"] = line_extension;
		args["user_extension"] = UserExtension();
		args["this"] = format("%p", this);
		Send(args);
	}

	if(dwDeviceState == 0)
	{
		xEND return;
	}

	if(dwDeviceState & LINEDEVSTATE_INSERVICE)
	{
		dwDeviceState &= ~LINEDEVSTATE_INSERVICE;

		std::map<std::string, std::string> args;
		args["IC_device_id"] = central_id;
		args["event"] = "Logging_ON";
		args["line_extension"] = line_extension;
		args["user_extension"] = UserExtension();
		args["this"] = format("%p", this);
		Send(args);
	}

	if(dwDeviceState == 0) return;

	if(dwDeviceState & LINEDEVSTATE_DEVSPECIFIC)
	{
		dwDeviceState &= ~LINEDEVSTATE_DEVSPECIFIC;
		if(line_dev_status_OLD.Do_Not_Disturb != line_dev_status.Do_Not_Disturb)
		{
			std::string event = line_dev_status.Do_Not_Disturb ? "Do_Not_Disturb_ON" : "Do_Not_Disturb_OFF";
			std::map<std::string, std::string> args;
			args["IC_device_id"] = central_id;
			args["event"] = event;
			args["line_extension"] = line_extension;
			args["user_extension"] = UserExtension();
			args["absent_message_id"] = format("%d", line_dev_status.Absent_message_id);
			args["absent_message_set_flag"] = line_dev_status.Absent_message_set_flag ? "true" : "false";
			args["absent_text"] = line_dev_status.Absent_text;
			args["this"] = format("%p", this);
			Send(args);
		}
		
	}

	if(dwDeviceState == 0)
	{
		xEND return;
	}

	if(dwDeviceState & LINEDEVSTATE_OTHER) {linestate = "LINEDEVSTATE_OTHER"; dwDeviceState &= ~LINEDEVSTATE_OTHER; }
	else if(dwDeviceState & LINEDEVSTATE_RINGING) {linestate = "LINEDEVSTATE_RINGING"; dwDeviceState &= ~LINEDEVSTATE_RINGING; }
	else if(dwDeviceState & LINEDEVSTATE_CONNECTED) {linestate = "LINEDEVSTATE_CONNECTED"; dwDeviceState &= ~LINEDEVSTATE_CONNECTED; }
	else if(dwDeviceState & LINEDEVSTATE_DISCONNECTED) {linestate = "LINEDEVSTATE_DISCONNECTED"; dwDeviceState &= ~LINEDEVSTATE_DISCONNECTED; }
	else if(dwDeviceState & LINEDEVSTATE_MSGWAITON) {linestate = "LINEDEVSTATE_MSGWAITON"; dwDeviceState &= ~LINEDEVSTATE_MSGWAITON; }
	else if(dwDeviceState & LINEDEVSTATE_MSGWAITOFF) {linestate = "LINEDEVSTATE_MSGWAITOFF"; dwDeviceState &= ~LINEDEVSTATE_MSGWAITOFF; }
	else if(dwDeviceState & LINEDEVSTATE_MAINTENANCE) {linestate = "LINEDEVSTATE_MAINTENANCE"; dwDeviceState &= ~LINEDEVSTATE_MAINTENANCE; }
	else if(dwDeviceState & LINEDEVSTATE_OPEN) {linestate = "LINEDEVSTATE_OPEN"; dwDeviceState &= ~LINEDEVSTATE_OPEN; }
	else if(dwDeviceState & LINEDEVSTATE_CLOSE) {linestate = "LINEDEVSTATE_CLOSE"; dwDeviceState &= ~LINEDEVSTATE_CLOSE; }
	else if(dwDeviceState & LINEDEVSTATE_NUMCALLS) {linestate = "LINEDEVSTATE_NUMCALLS"; dwDeviceState &= ~LINEDEVSTATE_NUMCALLS; }
	else if(dwDeviceState & LINEDEVSTATE_NUMCOMPLETIONS) {linestate = "LINEDEVSTATE_NUMCOMPLETIONS"; dwDeviceState &= ~LINEDEVSTATE_NUMCOMPLETIONS; }
	else if(dwDeviceState & LINEDEVSTATE_TERMINALS) {linestate = "LINEDEVSTATE_TERMINALS"; dwDeviceState &= ~LINEDEVSTATE_TERMINALS; }
	else if(dwDeviceState & LINEDEVSTATE_ROAMMODE) {linestate = "LINEDEVSTATE_ROAMMODE"; dwDeviceState &= ~LINEDEVSTATE_ROAMMODE; }
	else if(dwDeviceState & LINEDEVSTATE_BATTERY) {linestate = "LINEDEVSTATE_BATTERY"; dwDeviceState &= ~LINEDEVSTATE_BATTERY; }
	else if(dwDeviceState & LINEDEVSTATE_SIGNAL) {linestate = "LINEDEVSTATE_SIGNAL"; dwDeviceState &= ~LINEDEVSTATE_SIGNAL; }
	else if(dwDeviceState & LINEDEVSTATE_REINIT) {linestate = "LINEDEVSTATE_REINIT"; dwDeviceState &= ~LINEDEVSTATE_REINIT; }
	else if(dwDeviceState & LINEDEVSTATE_LOCK) {linestate = "LINEDEVSTATE_LOCK"; dwDeviceState &= ~LINEDEVSTATE_LOCK; }
	else if(dwDeviceState & LINEDEVSTATE_CAPSCHANGE) {linestate = "LINEDEVSTATE_CAPSCHANGE"; dwDeviceState &= ~LINEDEVSTATE_CAPSCHANGE; }
	else if(dwDeviceState & LINEDEVSTATE_CONFIGCHANGE) {linestate = "LINEDEVSTATE_CONFIGCHANGE"; dwDeviceState &= ~LINEDEVSTATE_CONFIGCHANGE; }
	else if(dwDeviceState & LINEDEVSTATE_TRANSLATECHANGE) {linestate = "LINEDEVSTATE_TRANSLATECHANGE"; dwDeviceState &= ~LINEDEVSTATE_TRANSLATECHANGE; }
	else if(dwDeviceState & LINEDEVSTATE_COMPLCANCEL) {linestate = "LINEDEVSTATE_COMPLCANCEL"; dwDeviceState &= ~LINEDEVSTATE_COMPLCANCEL; }
	else if(dwDeviceState & LINEDEVSTATE_REMOVED) {linestate = "LINEDEVSTATE_REMOVED"; dwDeviceState &= ~LINEDEVSTATE_REMOVED; }
	else {linestate = "unknown"; dwDeviceState = 0; }

	std::map<std::string, std::string> args;
	args["IC_device_id"] = central_id;
	args["event"] = "LineDevStateChange";
	args["event_detail"] = linestate;
	args["line_extension"] = line_extension;
	args["user_extension"] = UserExtension();
	args["this"] = format("%p", this);
	Send(args);

	if(dwDeviceState)
	{
		OnDeviceStateChange (dwDeviceState, dwStateDetail1, dwStateDetail2);
	}
	xEND
}
void CMyLine::OnDigitDetected (HCALL hCall, DWORD dwDigit, DWORD dwDigitMode){xSHOW("OnDigitDetected"); xSHOW(line_extension);}
void CMyLine::OnCallMediaModeChange (HCALL hCall, DWORD dwMediaMode){xSHOW("OnCallMediaModeChange"); xSHOW(line_extension);}
void CMyLine::OnToneDetected (HCALL hCall, DWORD dwAppSpecific){xSHOW("OnToneDetected"); xSHOW(line_extension);}
void CMyLine::OnNewCall (CTapiCall* pCall)
{
	xBEGIN
	Init();

	pCall->GetCallInfo(TRUE);
	
	std::map<std::string, std::string> args;
	args["IC_device_id"] = central_id;
	args["event"] = "NewCall";
	args["call_id"] = HCALL2string(pCall->GetCallHandle());
	args["line_extension"] = line_extension;
	args["user_extension"] = UserExtension();
	args["called_id"] = (LPCTSTR)pCall->GetCalledIDNumber();
	args["called_id_name"] = (LPCTSTR)pCall->GetCalledIDName();
	args["caller_id"] = (LPCTSTR)pCall->GetCallerIDNumber();
	args["caller_id_name"] = (LPCTSTR)pCall->GetCallerIDName();
	args["hunt_group"] = "?";
	args["this"] = format("%p", this);
	Send(args);
	xEND

}
void CMyLine::OnDynamicCreate(){xSHOW("OnDynamicCreate"); xSHOW(line_extension);}
void CMyLine::OnDynamicRemove(){xSHOW("OnDynamicRemove"); xSHOW(line_extension);}
void CMyLine::OnForceClose(){xSHOW("OnForceClose"); xSHOW(line_extension);}

IMPLEMENT_DYNCREATE (CMyLine, CTapiLine)




void TapiConn::Connect()
{
	xBEGIN
	theLog->Default("Opening TAPI connection");
	int res = GetTAPIConnection()->Init ("TapiCallMonitor", RUNTIME_CLASS(CMyLine));
	if(res)
	{
		theLog->Fatal("Tapi connection init failed");
		Disconnect();
		xEND
		exit(0);
	}
	
	std::string fmt = IniReadStr(TA_SETTINGS_GENERIC, TA_SETTINGS_GENERIC_EXTENSIONIDENTIFIER);
	std::string lines_str = IniReadStr(TA_SETTINGS_GENERIC, TA_SETTINGS_GENERIC_LINESTOMONITOR);
	theLog->Debug(lines_str);
	char delim = ',';
	std::stringstream ss(lines_str);
	std::string line;
	std::set<std::string> lines_to_monitor;

	while(std::getline(ss, line, delim))
	{
        lines_to_monitor.insert(line);
    }
    for(std::set<std::string>::iterator it = lines_to_monitor.begin(); it != lines_to_monitor.end(); ++it)
    {
    	theLog->Debug(*it);
    }
	
	for (DWORD dwLine = 0; dwLine < GetTAPIConnection()->GetLineDeviceCount(); dwLine++)
	{
		CTapiLine* pLine = GetTAPIConnection()->GetLineFromDeviceID(dwLine);
		std::string name((LPCTSTR)pLine->GetLineName());
		theLog->Debug(name);
		char buffer[512] = {0};
		if(sscanf(name.c_str(), fmt.c_str(), buffer) && (lines_to_monitor.find(buffer) != lines_to_monitor.end() || lines_to_monitor.find("*") != lines_to_monitor.end()))
		{
			DWORD dwMediaMode = 0xffffffff;
			const LPLINEDEVCAPS lpCaps = pLine->GetLineCaps();
			if (lpCaps != NULL)
			{
				dwMediaMode &= lpCaps->dwMediaModes;
				LONG lResult = pLine->Open (LINECALLPRIVILEGE_MONITOR+LINECALLPRIVILEGE_OWNER, dwMediaMode);
				if (lResult == 0)
				{
					theLog->Default(std::string("Opened line ") + buffer);
					if(pLine->SetStatusMessages(LINEDEVSTATE_ALL, LINEADDRESSSTATE_ALL) == 0)
					{
						lines[buffer] = pLine;
					}
					else
					{
						theLog->Warning(std::string("Unable to SetStatusMessages for line ") + buffer);
					}
				}
				else
				{
					theLog->Warning(std::string("Unable to open line ") + buffer);
				}
			}
			else
			{
				theLog->Warning(std::string("Unable to GetLineCaps for line ") + buffer);
			}
		}
	}
	xEND
}
void TapiConn::Disconnect()
{
	xBEGIN
	theLog->Default("Closing TAPI connection");
	GetTAPIConnection()->Shutdown();
	lines.clear();
	xEND
}
void TapiConn::Logging_ON(std::string line_extension, std::string user_extension)
{
	xBEGIN
	if(line_extension == "" || user_extension == "")
	{
		xEND
		throw std::string("Logging_ON invalid arguments");
	}
	std::map<std::string, CTapiLine*>::iterator it = lines.find(line_extension);
	if(it == lines.end())
	{
		xEND
		throw std::string("Logging_ON received for a non-monitored line: ") + line_extension;
	}
	char buf[1024] = {8, 0};
	strcat(buf, user_extension.c_str());
	SHOW(buf);
	int res = it->second->GetAddress((DWORD)0)->DevSpecific(buf, strlen(buf) + 1);
	if(res <= 0)
	{
		xEND
		throw std::string("Logging_ON DevSpecific failed ") + TapiErrMsg(res);
	}
}
void TapiConn::Logging_OFF(std::string line_extension, std::string user_extension)
{
	xBEGIN
	if(line_extension == "" || user_extension == "")
	{
		xEND
		throw std::string("Logging_OFF invalid arguments");
	}
	std::map<std::string, CTapiLine*>::iterator it = lines.find(line_extension);
	if(it == lines.end())
	{
		xEND
		throw std::string("Logging_OFF received for a non-monitored line ") + line_extension;
	}
	char buf[] = {9, 47, 0};
	int res = it->second->GetAddress((DWORD)0)->DevSpecific(buf, strlen(buf) + 1);
	if(res <= 0)
	{
		xEND
		throw std::string("Logging_OFF DevSpecific failed ") + TapiErrMsg(res);
	}
}
void TapiConn::Do_Not_Disturb_ON(std::string line_extension, std::string user_extension)
{
	xBEGIN
	if(line_extension == "" || user_extension == "")
	{
		xEND
		throw std::string("Do_Not_Disturb_ON invalid arguments");
	}
	std::map<std::string, CTapiLine*>::iterator it = lines.find(line_extension);
	if(it == lines.end())
	{
		xEND
		throw std::string("Do_Not_Disturb_ON received for a non-monitored line ") + line_extension;
	}
	char buf[] = {9, 7, 0};
	int res = it->second->GetAddress((DWORD)0)->DevSpecific(buf, strlen(buf) + 1);
	if(res <= 0)
	{
		xEND
		throw std::string("Do_Not_Disturb_ON DevSpecific failed ") + TapiErrMsg(res);
	}
}
void TapiConn::Do_Not_Disturb_OFF(std::string line_extension, std::string user_extension)
{
	xBEGIN
	if(line_extension == "" || user_extension == "")
	{
		xEND
		throw std::string("Do_Not_Disturb_OFF invalid arguments");
	}
	std::map<std::string, CTapiLine*>::iterator it = lines.find(line_extension);
	if(it == lines.end())
	{
		xEND
		throw std::string("Do_Not_Disturb_OFF received for a non-monitored line ") + line_extension;
	}
	char buf[] = {9, 8, 0};
	int res = it->second->GetAddress((DWORD)0)->DevSpecific(buf, strlen(buf) + 1);
	if(res <= 0)
	{
		xEND
		throw std::string("Do_Not_Disturb_OFF DevSpecific failed ") + TapiErrMsg(res);
	}
	xEND
}
void TapiConn::Listen(std::string line_extension, std::string user_extension, std::string extension_2_listen)
{
	xBEGIN
	if(line_extension == "" || user_extension == "" || extension_2_listen == "")
	{
		xEND
		throw std::string("Listen invalid arguments");
	}
	std::map<std::string, CTapiLine*>::iterator it = lines.find(line_extension);
	if(it == lines.end())
	{
		xEND
		throw std::string("Listen received for a non-monitored line ") + line_extension;
	}
	std::string buf;
	buf += char(9);
	buf += char(100);
	buf += extension_2_listen;
	int res = it->second->GetAddress((DWORD)0)->DevSpecific((LPVOID)buf.c_str(), buf.size() + 1);
	if(res <= 0)
	{
		xEND
		throw std::string("Listen: DevSpecific failed ") + TapiErrMsg(res);
	}
	xEND
}
void TapiConn::MakeCall(std::string line_extension,  std::string user_extension, std::string destination)
{
	xBEGIN
	if(line_extension == "" || user_extension == "" || destination == "")
	{
		xEND
		throw std::string("MakeCall invalid arguments");
	}
	std::map<std::string, CTapiLine*>::iterator it = lines.find(line_extension);
	if(it == lines.end())
	{
		xEND
		throw std::string("MakeCall received for a non-monitored line ") + line_extension;
	}
	CTapiCall *pCall = 0;
	int res = it->second->MakeCall(&pCall, destination.c_str());
	if(res != 0 || pCall == 0)
	{
		xEND
		throw std::string("MakeCall: CTapiLine::MakeCall failed ") + TapiErrMsg(res);
	}
	xEND
}
void TapiConn::Answer(std::string call_id, std::string line_extension,  std::string user_extension)
{
	xBEGIN
	if(line_extension == "" || user_extension == "" || call_id == "")
	{
		xEND
		throw std::string("Answer invalid arguments");
	}
	std::map<std::string, CTapiLine*>::iterator it = lines.find(line_extension);
	if(it == lines.end())
	{
		xEND
		throw std::string("Answer received for a non-monitored line ") + line_extension;
	}
	CTapiCall *pCall = it->second->GetCallFromHandle(CMyLine::string2HCALL(call_id));
	if(!pCall)
	{
		xEND
		throw std::string("Answer: invalid call_id");
	}
	int res = pCall->Answer();
	if(res <= 0)
	{
		xEND
		throw std::string("Answer: CTapiCall::Answer failed ") + TapiErrMsg(res);
	}
	xEND
}
void TapiConn::Hold(std::string call_id, std::string line_extension,  std::string user_extension)
{
	xBEGIN
	if(line_extension == "" || user_extension == "" || call_id == "")
	{
		xEND
		throw std::string("Hold: invalid arguments");
	}
	std::map<std::string, CTapiLine*>::iterator it = lines.find(line_extension);
	if(it == lines.end())
	{
		xEND
		throw std::string("Hold received for a non-monitored line ") + line_extension;
	}
	CTapiCall *pCall = it->second->GetCallFromHandle(CMyLine::string2HCALL(call_id));
	if(!pCall)
	{
		xEND
		throw std::string("Hold: invalid call_id");
	}
	int res = pCall->Hold();
	if(res <= 0)
	{
		xEND
		throw std::string("Hold: CTapiCall::Hold failed ") + TapiErrMsg(res);
	}
	xEND
}
void TapiConn::Unhold(std::string call_id, std::string line_extension, std::string user_extension)
{
	xBEGIN
	if(line_extension == "" || user_extension == "" || call_id == "")
	{
		xEND
		throw std::string("Unhold invalid arguments");
	}
	std::map<std::string, CTapiLine*>::iterator it = lines.find(line_extension);
	if(it == lines.end())
	{
		xEND
		throw std::string("Unhold received for a non-monitored line ") + line_extension;
	}
	CTapiCall *pCall = it->second->GetCallFromHandle(CMyLine::string2HCALL(call_id));
	if(!pCall)
	{
		xEND
		throw std::string("Unhold: invalid call_id");
	}
	int res = pCall->Unhold();
	if(res <= 0)
	{
		xEND
		throw std::string("Unhold: CTapiCall::Unhold failed ") + TapiErrMsg(res);
	}
	xEND
}
void TapiConn::Hangup(std::string call_id, std::string line_extension, std::string user_extension)
{
	xBEGIN
	if(line_extension == "" || user_extension == "" || call_id == "")
	{
		xEND
		throw std::string("Hangup invalid arguments");
	}
	std::map<std::string, CTapiLine*>::iterator it = lines.find(line_extension);
	if(it == lines.end())
	{
		xEND
		throw std::string("Hangup received for a non-monitored line ") + line_extension;
	}
	CTapiCall *pCall = it->second->GetCallFromHandle(CMyLine::string2HCALL(call_id));
	if(!pCall)
	{
		xEND
		throw std::string("Hangup: invalid call_id");
	}
	int res = pCall->Drop();
	if(res <= 0)
	{
		xEND
		throw std::string("Hangup: CTapiCall::Drop failed ") + TapiErrMsg(res);
	}
	xEND
}
void TapiConn::BlindTransfer(std::string call_id, std::string line_extension,  std::string user_extension, std::string extension_2_transfer)
{
	xBEGIN
	if(line_extension == "" || user_extension == "" || call_id == "" || extension_2_transfer == "")
	{
		xEND
		throw std::string("BlindTransfer invalid arguments");
	}
	std::map<std::string, CTapiLine*>::iterator it = lines.find(line_extension);
	if(it == lines.end())
	{
		xEND
		throw std::string("BlindTransfer received for a non-monitored line ") + line_extension;
	}
	CTapiCall *pCall = it->second->GetCallFromHandle(CMyLine::string2HCALL(call_id));
	if(!pCall)
	{
		xEND
		throw std::string("BlindTransfer: invalid call_id");
	}
	int res = pCall->BlindTransfer(extension_2_transfer.c_str());
	if(res <= 0)
	{
		xEND
		throw std::string("BlindTransfer: CTapiCall::BlindTransfer failed ") + TapiErrMsg(res);
	}
	xEND
}


TapiConn *theTapiConn = NULL;


LineDevStatus::LineDevStatus(unsigned char *buffer, int size)
{
	xBEGIN
	unsigned char *p = buffer;
	
	if(size == 0)
	{
		theLog->Warning("No LineDevStatus");
		xEND
		return;
	}
	
	while(*p)
	{
		Phone_Extension += *p++;
	}
	Forward_on_busy = *++p;
	Forward_on_no_answer = *++p;
	Forward_unconditional = *++p;
	Forward_hunt_group_flag = *++p;
	Do_Not_Disturb = *++p;
	Outgoing_call_bar_flag = *++p;
	Call_waiting_on_flag = *++p;
	Voicemail_on_flag = *++p;
	Voicemail_ring_back_flag = *++p;
	Number_of_read_voicemail_messages = *++p;
	Number_of_unread_voicemail_messages = *++p;
	Outside_call_sequence_number = *++p;
	Inside_call_sequence_number = *++p;
	Ring_back_sequence_number = *++p;
	No_answer_timeout_period = *++p;
	Wrap_up_time_period = *++p;
	Can_intrude_flag = *++p;
	Cannot_be_intruded_upon_flag = *++p;
	X_directory_flag = *++p;
	Force_login_flag = *++p;
	Forced_account_code_flag = *++p;
	Login_code_flag = *++p;
	System_phone_flag = *++p;
	Absent_message_id = *++p;
	Absent_message_set_flag = *++p;
	Voicemail_email_mode = *++p;
	++p;
	while(*p)
	{
		Extn += *p++;
	}
	++p;
	while(*p)
	{
		locale += *p++;
	}
	++p;
	while(*p)
	{
		Forward_destination += *p++;
	}
	++p;
	while(*p)
	{
		Follow_me_number += *p++;
	}
		++p;
	while(*p)
	{
		Absent_text += *p++;
	}
	++p;
	while(*p)
	{
		Do_not_disturb_exception_list += *p++;
	}
	++p;
	while(*p)
	{
		Forward_on_busy_number += *p++;
	}
	++p;
	User_s_priority = *++p;
	Group_membership = *++p;
	Groups_out_of_time = *++p;
	Disabled_groups = *++p;
	Groups_out_of_service = *++p;
	Night_service_groups = *++p;
	
	++p;
	while(*p)
	{
		Working_Hours_User_Rights_Group_Name += *p++;
	}
	++p;
	while(*p)
	{
		Out_of_Hours_User_Rights_Group_Name += *p++;
	}
	++p;
	while(*p)
	{
		User_Restriction_Name_List += *p++;
	}
	if(p > buffer + size)
	{
		SHOW(p >= buffer + size);
		SHOW((long)p);
		SHOW((long)(buffer + size));
	}
	//Show();
	xEND
}


std::string TapiErrMsg(DWORD err)
{
	xBEGIN
	switch(err)
	{
	case LINEERR_ALLOCATED: return "LINEERR_ALLOCATED";
	case LINEERR_BADDEVICEID: return "LINEERR_BADDEVICEID";
	case LINEERR_BEARERMODEUNAVAIL: return "LINEERR_BEARERMODEUNAVAIL";
	case LINEERR_CALLUNAVAIL: return "LINEERR_CALLUNAVAIL";
	case LINEERR_COMPLETIONOVERRUN: return "LINEERR_COMPLETIONOVERRUN";
	case LINEERR_CONFERENCEFULL: return "LINEERR_CONFERENCEFULL";
	case LINEERR_DIALBILLING: return "LINEERR_DIALBILLING";
	case LINEERR_DIALDIALTONE: return "LINEERR_DIALDIALTONE";
	case LINEERR_DIALPROMPT: return "LINEERR_DIALPROMPT";
	case LINEERR_DIALQUIET: return "LINEERR_DIALQUIET";
	case LINEERR_INCOMPATIBLEAPIVERSION: return "LINEERR_INCOMPATIBLEAPIVERSION";
	case LINEERR_INCOMPATIBLEEXTVERSION: return "LINEERR_INCOMPATIBLEEXTVERSION";
	case LINEERR_INIFILECORRUPT: return "LINEERR_INIFILECORRUPT";
	case LINEERR_INUSE: return "LINEERR_INUSE";
	case LINEERR_INVALADDRESS: return "LINEERR_INVALADDRESS";
	case LINEERR_INVALADDRESSID: return "LINEERR_INVALADDRESSID";
	case LINEERR_INVALADDRESSMODE: return "LINEERR_INVALADDRESSMODE";
	case LINEERR_INVALADDRESSSTATE: return "LINEERR_INVALADDRESSSTATE";
	case LINEERR_INVALAPPHANDLE: return "LINEERR_INVALAPPHANDLE";
	case LINEERR_INVALAPPNAME: return "LINEERR_INVALAPPNAME";
	case LINEERR_INVALBEARERMODE: return "LINEERR_INVALBEARERMODE";
	case LINEERR_INVALCALLCOMPLMODE: return "LINEERR_INVALCALLCOMPLMODE";
	case LINEERR_INVALCALLHANDLE: return "LINEERR_INVALCALLHANDLE";
	case LINEERR_INVALCALLPARAMS: return "LINEERR_INVALCALLPARAMS";
	case LINEERR_INVALCALLPRIVILEGE: return "LINEERR_INVALCALLPRIVILEGE";
	case LINEERR_INVALCALLSELECT: return "LINEERR_INVALCALLSELECT";
	case LINEERR_INVALCALLSTATE: return "LINEERR_INVALCALLSTATE";
	case LINEERR_INVALCALLSTATELIST: return "LINEERR_INVALCALLSTATELIST";
	case LINEERR_INVALCARD: return "LINEERR_INVALCARD";
	case LINEERR_INVALCOMPLETIONID: return "LINEERR_INVALCOMPLETIONID";
	case LINEERR_INVALCONFCALLHANDLE: return "LINEERR_INVALCONFCALLHANDLE";
	case LINEERR_INVALCONSULTCALLHANDLE: return "LINEERR_INVALCONSULTCALLHANDLE";
	case LINEERR_INVALCOUNTRYCODE: return "LINEERR_INVALCOUNTRYCODE";
	case LINEERR_INVALDEVICECLASS: return "LINEERR_INVALDEVICECLASS";
	case LINEERR_INVALDEVICEHANDLE: return "LINEERR_INVALDEVICEHANDLE";
	case LINEERR_INVALDIALPARAMS: return "LINEERR_INVALDIALPARAMS";
	case LINEERR_INVALDIGITLIST: return "LINEERR_INVALDIGITLIST";
	case LINEERR_INVALDIGITMODE: return "LINEERR_INVALDIGITMODE";
	case LINEERR_INVALDIGITS: return "LINEERR_INVALDIGITS";
	case LINEERR_INVALEXTVERSION: return "LINEERR_INVALEXTVERSION";
	case LINEERR_INVALGROUPID: return "LINEERR_INVALGROUPID";
	case LINEERR_INVALLINEHANDLE: return "LINEERR_INVALLINEHANDLE";
	case LINEERR_INVALLINESTATE: return "LINEERR_INVALLINESTATE";
	case LINEERR_INVALLOCATION: return "LINEERR_INVALLOCATION";
	case LINEERR_INVALMEDIALIST: return "LINEERR_INVALMEDIALIST";
	case LINEERR_INVALMEDIAMODE: return "LINEERR_INVALMEDIAMODE";
	case LINEERR_INVALMESSAGEID: return "LINEERR_INVALMESSAGEID";
	case LINEERR_INVALPARAM: return "LINEERR_INVALPARAM";
	case LINEERR_INVALPARKID: return "LINEERR_INVALPARKID";
	case LINEERR_INVALPARKMODE: return "LINEERR_INVALPARKMODE";
	case LINEERR_INVALPOINTER: return "LINEERR_INVALPOINTER";
	case LINEERR_INVALPRIVSELECT: return "LINEERR_INVALPRIVSELECT";
	case LINEERR_INVALRATE: return "LINEERR_INVALRATE";
	case LINEERR_INVALREQUESTMODE: return "LINEERR_INVALREQUESTMODE";
	case LINEERR_INVALTERMINALID: return "LINEERR_INVALTERMINALID";
	case LINEERR_INVALTERMINALMODE: return "LINEERR_INVALTERMINALMODE";
	case LINEERR_INVALTIMEOUT: return "LINEERR_INVALTIMEOUT";
	case LINEERR_INVALTONE: return "LINEERR_INVALTONE";
	case LINEERR_INVALTONELIST: return "LINEERR_INVALTONELIST";
	case LINEERR_INVALTONEMODE: return "LINEERR_INVALTONEMODE";
	case LINEERR_INVALTRANSFERMODE: return "LINEERR_INVALTRANSFERMODE";
	case LINEERR_LINEMAPPERFAILED: return "LINEERR_LINEMAPPERFAILED";
	case LINEERR_NOCONFERENCE: return "LINEERR_NOCONFERENCE";
	case LINEERR_NODEVICE: return "LINEERR_NODEVICE";
	case LINEERR_NODRIVER: return "LINEERR_NODRIVER";
	case LINEERR_NOMEM: return "LINEERR_NOMEM";
	case LINEERR_NOREQUEST: return "LINEERR_NOREQUEST";
	case LINEERR_NOTOWNER: return "LINEERR_NOTOWNER";
	case LINEERR_NOTREGISTERED: return "LINEERR_NOTREGISTERED";
	case LINEERR_OPERATIONFAILED: return "LINEERR_OPERATIONFAILED";
	case LINEERR_OPERATIONUNAVAIL: return "LINEERR_OPERATIONUNAVAIL";
	case LINEERR_RATEUNAVAIL: return "LINEERR_RATEUNAVAIL";
	case LINEERR_RESOURCEUNAVAIL: return "LINEERR_RESOURCEUNAVAIL";
	case LINEERR_REQUESTOVERRUN: return "LINEERR_REQUESTOVERRUN";
	case LINEERR_STRUCTURETOOSMALL: return "LINEERR_STRUCTURETOOSMALL";
	case LINEERR_TARGETNOTFOUND: return "LINEERR_TARGETNOTFOUND";
	case LINEERR_TARGETSELF: return "LINEERR_TARGETSELF";
	case LINEERR_UNINITIALIZED: return "LINEERR_UNINITIALIZED";
	case LINEERR_USERUSERINFOTOOBIG: return "LINEERR_USERUSERINFOTOOBIG";
	case LINEERR_REINIT: return "LINEERR_REINIT";
	case LINEERR_ADDRESSBLOCKED: return "LINEERR_ADDRESSBLOCKED";
	case LINEERR_BILLINGREJECTED: return "LINEERR_BILLINGREJECTED";
	case LINEERR_INVALFEATURE: return "LINEERR_INVALFEATURE";
	case LINEERR_NOMULTIPLEINSTANCE: return "LINEERR_NOMULTIPLEINSTANCE";
	case LINEERR_INVALAGENTID: return "LINEERR_INVALAGENTID";
	case LINEERR_INVALAGENTGROUP: return "LINEERR_INVALAGENTGROUP";
	case LINEERR_INVALPASSWORD: return "LINEERR_INVALPASSWORD";
	case LINEERR_INVALAGENTSTATE: return "LINEERR_INVALAGENTSTATE";
	case LINEERR_INVALAGENTACTIVITY: return "LINEERR_INVALAGENTACTIVITY";
	case LINEERR_DIALVOICEDETECT: return "LINEERR_DIALVOICEDETECT";
	case LINEERR_USERCANCELLED: return "LINEERR_USERCANCELLED";
	case LINEERR_INVALADDRESSTYPE: return "LINEERR_INVALADDRESSTYPE";
	case LINEERR_INVALAGENTSESSIONSTATE: return "LINEERR_INVALAGENTSESSIONSTATE";
	case LINEERR_DISCONNECTED: return "LINEERR_DISCONNECTED";
	case LINEERR_SERVICE_NOT_RUNNING: return "LINEERR_SERVICE_NOT_RUNNING";
	default: return format("Unknown TAPI error (0x%X)", err);
	}
}