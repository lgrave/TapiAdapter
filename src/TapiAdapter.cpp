//
//	Last Modified: $Date: 2010-07-22 10:16:37 $
//
//	$Log: TapiAdapter.cpp,v $
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

#include <stdio.h>

#include "Log.h"
#include "Settings.h"
#include "TapiConn.h"
#include "XmlRpc.h"
#include "XmlRpcMethods.h"
#include "MultithreadXmlRpcServer.h"


using namespace XmlRpc;

#define TA_SERVICE_NAME		"TapiAdapter"


SERVICE_STATUS ServiceStatus; 
SERVICE_STATUS_HANDLE hStatus; 
 
void  ServiceMain(int argc, char** argv); 
void  ControlHandler(DWORD request); 
int InitService();



int main(void) 
{
    SERVICE_TABLE_ENTRY ServiceTable[2];
    ServiceTable[0].lpServiceName = TA_SERVICE_NAME;
    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

    ServiceTable[1].lpServiceName = NULL;
    ServiceTable[1].lpServiceProc = NULL;
    
    StartServiceCtrlDispatcher(ServiceTable);  
}


void ServiceMain(int argc, char** argv) 
{
	std::string log_file = IniReadStr(TA_SETTINGS_GENERIC, TA_SETTINGS_GENERIC_LOGFILE);
	int log_level = IniReadInt(TA_SETTINGS_GENERIC, TA_SETTINGS_GENERIC_LOGLEVEL);
	theLog = new Log(log_file, log_level);

	theLog->Default("Log started");
	theLog->Details("Tapi Adapter v1.0 - Copyright lgrave 2010");

	theLog->Debug("Registering Control Handler");
    ServiceStatus.dwServiceType = SERVICE_WIN32; 
    ServiceStatus.dwCurrentState = SERVICE_START_PENDING; 
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    ServiceStatus.dwWin32ExitCode = 0; 
    ServiceStatus.dwServiceSpecificExitCode = 0; 
    ServiceStatus.dwCheckPoint = 0; 
    ServiceStatus.dwWaitHint = 0;  
    hStatus = RegisterServiceCtrlHandler(TA_SERVICE_NAME, (LPHANDLER_FUNCTION)ControlHandler); 
    if (hStatus == (SERVICE_STATUS_HANDLE)0) 
    { 
        theLog->Fatal("Registering Control Handler failed");
        return; 
    }  
    
    theLog->Debug("Initialize Service");
    int error = InitService(); 
    if (error) 
    {
		theLog->Fatal("Initialization failed");
        ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
        ServiceStatus.dwWin32ExitCode = -1; 
        SetServiceStatus(hStatus, &ServiceStatus); 
        return; 
    } 
    
    theLog->Debug("report the running status to SCM");
    ServiceStatus.dwCurrentState = SERVICE_RUNNING; 
    SetServiceStatus (hStatus, &ServiceStatus);
    
    theLog->Debug("Creating the TapiConn class");
    theTapiConn = new TapiConn();

	theLog->Debug("Creating the Multithread XmlRpc server");
	theServer = new MultithreadXmlRpcServer(IniReadInt(TA_SETTINGS_COMMANDS, TA_SETTINGS_COMMANDS_THREADS));
	
	theLog->Debug("Registering the methods");
	XML_RPC_METHOD_REGISTER(Logging_ON, theServer);
	XML_RPC_METHOD_REGISTER(Logging_OFF, theServer);
	XML_RPC_METHOD_REGISTER(Do_Not_Disturb_ON, theServer);
	XML_RPC_METHOD_REGISTER(Do_Not_Disturb_OFF, theServer);
	XML_RPC_METHOD_REGISTER(MakeCall, theServer);
	XML_RPC_METHOD_REGISTER(Listen, theServer);
	XML_RPC_METHOD_REGISTER(Answer, theServer);
	XML_RPC_METHOD_REGISTER(Hold, theServer);
	XML_RPC_METHOD_REGISTER(Unhold, theServer);
	XML_RPC_METHOD_REGISTER(Hangup, theServer);
	XML_RPC_METHOD_REGISTER(BlindTransfer, theServer);
	XML_RPC_METHOD_REGISTER(Culpa, theServer);
	
	theLog->Debug("Setting verbosity to 0");
	XmlRpc::setVerbosity(5);

	theLog->Debug("Create the server socket on the specified port");
	theServer->bindAndListen(IniReadInt(TA_SETTINGS_COMMANDS, TA_SETTINGS_COMMANDS_LISTENPORT));

	theLog->Debug("Enable introspection");
	theServer->enableIntrospection(true);

	theLog->Debug("Connecting Tapi");
	theTapiConn->Connect();

    theLog->Debug("The worker loop of a service");
    while (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		theLog->Default("Still running...");
		theLog->Debug("Wait for requests ~10min");
		theServer->work(300.0);
	}
	theLog->Default("Finishing service");
    return; 
}
 
// Service initialization
int InitService() 
{
    int result = 0;
    theLog->Default("Monitoring started.");
    return(result); 
} 

// Control handler function
void ControlHandler(DWORD request) 
{
	theLog->Debug("Entering ControlHandler()");
    switch(request) 
    { 
        case SERVICE_CONTROL_STOP: 
            theLog->Warning("Monitoring stopped.");
            ServiceStatus.dwWin32ExitCode = 0; 
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
            SetServiceStatus (hStatus, &ServiceStatus);
            
            return; 
 
        case SERVICE_CONTROL_SHUTDOWN: 
            theLog->Warning("Monitoring stopped.");
            ServiceStatus.dwWin32ExitCode = 0; 
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
            SetServiceStatus (hStatus, &ServiceStatus);
            return; 
        
        default:
            break;
    } 
 
    theLog->Debug("Report current status");
    SetServiceStatus (hStatus,  &ServiceStatus);
 
    return; 
} 

