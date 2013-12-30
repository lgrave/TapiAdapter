//
//	Last Modified: $Date: 2010-07-22 10:16:38 $
//
//	$Log: XmlRpcMethods.h,v $
//	Revision 1.2.2.1  2010-07-22 10:16:38  lgrave
//	RT8: TapiAdapter: threads for reception/execution of paralel commands
//
//	Revision 1.2  2010-07-20 09:48:14  lgrave
//	corrected windows crlf to unix lf
//
//	Revision 1.1  2010-07-19 23:40:45  lgrave
//	1st version added to cvs
//
//

#ifndef TA_XMLRPCMETHODS_H
#define TA_XMLRPCMETHODS_H

#include "XmlRpc.h"
#include "MultithreadXmlRpcServer.h"
#include <string>
#include <stdexcept>
#include "Log.h"

#define XML_RPC_METHOD_DECLARE(name)							class name : public XmlRpc::XmlRpcServerMethod\
																{\
																public:\
																	name(XmlRpc::XmlRpcServer* s) : XmlRpc::XmlRpcServerMethod(#name, s) {}\
																	std::string help();\
																	void executeThrow(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result);\
																	void execute(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result)\
																	{\
																		std::string error_msg;\
																		try\
																		{\
																			executeThrow(params, result);\
																		}\
																		catch(std::string ex)\
																		{\
																			error_msg = ex;\
																		}\
																		catch(char *ex)\
																		{\
																			error_msg = ex;\
																		}\
																		catch(std::exception ex)\
																		{\
																			error_msg = ex.what();\
																		}\
																		catch(...)\
																		{\
																			error_msg = "Unspecified error";\
																		}\
																		if(error_msg == "")\
																		{\
																			result = "SUCCESS";\
																			theLog->Default("SUCCESS");\
																		}\
																		else\
																		{\
																			result = "ERROR: " + error_msg;\
																			theLog->Error(error_msg);\
																		}\
																	}\
																};
																
																
																

#define XML_RPC_METHOD_IMPLEMENT(name, helptxt)					std::string name::help() { return std::string(helptxt); }\
																void name::executeThrow(XmlRpc::XmlRpcValue& params, XmlRpc::XmlRpcValue& result)


#define XML_RPC_METHOD_REGISTER(name, server)					static name the##name(server)



extern XmlRpc::MultithreadXmlRpcServer *theServer;


XML_RPC_METHOD_DECLARE(Logging_ON)
XML_RPC_METHOD_DECLARE(Logging_OFF)
XML_RPC_METHOD_DECLARE(Do_Not_Disturb_ON)
XML_RPC_METHOD_DECLARE(Do_Not_Disturb_OFF)
XML_RPC_METHOD_DECLARE(MakeCall)
XML_RPC_METHOD_DECLARE(Listen)
XML_RPC_METHOD_DECLARE(Answer)
XML_RPC_METHOD_DECLARE(Hold)
XML_RPC_METHOD_DECLARE(Unhold)
XML_RPC_METHOD_DECLARE(Hangup)
XML_RPC_METHOD_DECLARE(BlindTransfer)
XML_RPC_METHOD_DECLARE(Culpa)





#endif // TA_XMLRPCMETHODS_H
