//
//	Last Modified: $Date: 2010-07-22 10:16:37 $
//
//	$Log: XmlRpcMethods.cpp,v $
//	Revision 1.4.2.1  2010-07-22 10:16:37  lgrave
//	RT8: TapiAdapter: threads for reception/execution of paralel commands
//
//	Revision 1.4  2010-07-21 12:29:48  lgrave
//	commands to TapiAdapter as an hash
//
//	Revision 1.3  2010-07-21 12:12:56  lgrave
//	TestClient for sending commands from the command line
//
//	Revision 1.2  2010-07-20 09:48:13  lgrave
//	corrected windows crlf to unix lf
//
//	Revision 1.1  2010-07-19 23:40:44  lgrave
//	1st version added to cvs
//
//

//#include "XmlRpc.h"
#include "XmlRpcMethods.h"
#include "Log.h"
#include "TapiConn.h"

using namespace XmlRpc;

XmlRpc::MultithreadXmlRpcServer *theServer = NULL;


XML_RPC_METHOD_IMPLEMENT(Logging_ON, "1st arg string line_extension, 2nd arg string user_extension")
{
	xBEGIN
	std::string line_extension;
	std::string user_extension;
	
	std::stringstream ss;
	params.write(ss);
	theLog->Debug(ss.str());
	
	if(params.getType() == XmlRpcValue::TypeArray && params[0].getType() == XmlRpcValue::TypeStruct)
	{
		line_extension = std::string(params[0]["line_extension"]);
		user_extension = std::string(params[0]["user_extension"]);
	}
	else
	{
		line_extension = std::string(params[0]);
		user_extension = std::string(params[1]);
	}
	theLog->Default(std::string("Received command: Logging_ON, ") + line_extension + ", " + user_extension);
	theTapiConn->Logging_ON(line_extension, user_extension);
	result = "SUCCESS";
	xEND
}

XML_RPC_METHOD_IMPLEMENT(Logging_OFF, "1st arg string line_extension, 2nd arg string user_extension")
{
	xBEGIN
	std::string line_extension;
	std::string user_extension;
	
	std::stringstream ss;
	params.write(ss);
	theLog->Debug(ss.str());
	
	if(params.getType() == XmlRpcValue::TypeArray && params[0].getType() == XmlRpcValue::TypeStruct)
	{
		line_extension = std::string(params[0]["line_extension"]);
		user_extension = std::string(params[0]["user_extension"]);
	}
	else
	{
		line_extension = std::string(params[0]);
		user_extension = std::string(params[1]);
	}
  	theLog->Default(std::string("Received command: Logging_OFF, ") + line_extension + ", " + user_extension);
	theTapiConn->Logging_OFF(line_extension, user_extension);
    result = "SUCCESS";
	xEND
}

XML_RPC_METHOD_IMPLEMENT(Do_Not_Disturb_ON, "1st arg string line_extension, 2nd arg string user_extension")
{
	xBEGIN
	std::string line_extension;
	std::string user_extension;
	
	std::stringstream ss;
	params.write(ss);
	theLog->Debug(ss.str());
	
	if(params.getType() == XmlRpcValue::TypeArray && params[0].getType() == XmlRpcValue::TypeStruct)
	{
		line_extension = std::string(params[0]["line_extension"]);
		user_extension = std::string(params[0]["user_extension"]);
	}
	else
	{
		line_extension = std::string(params[0]);
		user_extension = std::string(params[1]);
	}
	theLog->Default(std::string("Received command: Do_Not_Disturb_ON, ") + line_extension + ", " + user_extension);
	theTapiConn->Do_Not_Disturb_ON(line_extension, user_extension);
	result = "SUCCESS";
	xEND
}

XML_RPC_METHOD_IMPLEMENT(Do_Not_Disturb_OFF, "1st arg string line_extension, 2nd arg string user_extension")
{
	xBEGIN
	std::string line_extension;
	std::string user_extension;
	
	std::stringstream ss;
	params.write(ss);
	theLog->Debug(ss.str());
	
	if(params.getType() == XmlRpcValue::TypeArray && params[0].getType() == XmlRpcValue::TypeStruct)
	{
		line_extension = std::string(params[0]["line_extension"]);
		user_extension = std::string(params[0]["user_extension"]);
	}
	else
	{
		line_extension = std::string(params[0]);
		user_extension = std::string(params[1]);
	}
  	theLog->Default(std::string("Received command: Do_Not_Disturb_OFF, ") + line_extension + ", " + user_extension);
	theTapiConn->Do_Not_Disturb_OFF(line_extension, user_extension);
    result = "SUCCESS";
	xEND
}

XML_RPC_METHOD_IMPLEMENT(MakeCall, "1st arg string line_extension, 2nd arg string user_extension, 3rd arg string destination")
{
	xBEGIN
	std::string line_extension;
	std::string user_extension;
	std::string destination;
	
	std::stringstream ss;
	params.write(ss);
	theLog->Debug(ss.str());
	
	if(params.getType() == XmlRpcValue::TypeArray && params[0].getType() == XmlRpcValue::TypeStruct)
	{
		line_extension = std::string(params[0]["line_extension"]);
		user_extension = std::string(params[0]["user_extension"]);
		destination = std::string(params[0]["destination"]);
	}
	else
	{
		line_extension = std::string(params[0]);
		user_extension = std::string(params[1]);
		destination = std::string(params[2]);
	}
	theLog->Default(std::string("Received command: MakeCall, ") + line_extension + ", " + user_extension + ", " + destination);
	theTapiConn->MakeCall(line_extension, user_extension, destination);
	result = "SUCCESS";
	xEND
}

XML_RPC_METHOD_IMPLEMENT(Listen, "1st arg string line_extension, 2nd arg string user_extension, 3rd arg extension 2 listen")
{
	xBEGIN
	std::string line_extension;
	std::string user_extension;
	std::string extension_2_listen;
	
	std::stringstream ss;
	params.write(ss);
	theLog->Debug(ss.str());
	
	if(params.getType() == XmlRpcValue::TypeArray && params[0].getType() == XmlRpcValue::TypeStruct)
	{
		line_extension = std::string(params[0]["line_extension"]);
		user_extension = std::string(params[0]["user_extension"]);
		extension_2_listen = std::string(params[0]["extension_2_listen"]);
	}
	else
	{
		line_extension = std::string(params[0]);
		user_extension = std::string(params[1]);
		extension_2_listen = std::string(params[2]);
	}
	theLog->Default(std::string("Received command: Listen, ") + line_extension + ", " + user_extension + ", " + extension_2_listen);
	theTapiConn->Listen(line_extension, user_extension, extension_2_listen);
	result = "SUCCESS";
	xEND
}

XML_RPC_METHOD_IMPLEMENT(Answer, "1st arg call_id, 2nd arg string line_extension, 3rd arg string user_extension")
{
	xBEGIN
	std::string call_id;
	std::string line_extension;
	std::string user_extension;
	
	std::stringstream ss;
	params.write(ss);
	theLog->Debug(ss.str());
	
	if(params.getType() == XmlRpcValue::TypeArray && params[0].getType() == XmlRpcValue::TypeStruct)
	{
		call_id = std::string(params[0]["call_id"]);
		line_extension = std::string(params[0]["line_extension"]);
		user_extension = std::string(params[0]["user_extension"]);
	}
	else
	{
		call_id = std::string(params[0]);
		line_extension = std::string(params[1]);
		user_extension = std::string(params[2]);
	}
	theLog->Default(std::string("Received command: Answer, ") + call_id + ", " + line_extension + ", " + user_extension);
	theTapiConn->Answer(call_id, line_extension, user_extension);
	result = "SUCCESS";
	xEND
}
XML_RPC_METHOD_IMPLEMENT(Hold, "1st arg call_id, 2nd arg string line_extension, 3rd arg string user_extension")
{
	xBEGIN
	std::string call_id;
	std::string line_extension;
	std::string user_extension;
	
	std::stringstream ss;
	params.write(ss);
	theLog->Debug(ss.str());
	
	if(params.getType() == XmlRpcValue::TypeArray && params[0].getType() == XmlRpcValue::TypeStruct)
	{
		call_id = std::string(params[0]["call_id"]);
		line_extension = std::string(params[0]["line_extension"]);
		user_extension = std::string(params[0]["user_extension"]);
	}
	else
	{
		call_id = std::string(params[0]);
		line_extension = std::string(params[1]);
		user_extension = std::string(params[2]);
	}
	theLog->Default(std::string("Received command: Hold, ") + call_id + ", " + line_extension + ", " + user_extension);
	theTapiConn->Hold(call_id, line_extension, user_extension);
	result = "SUCCESS";
	xEND
}
XML_RPC_METHOD_IMPLEMENT(Unhold, "1st arg call_id, 2nd arg string line_extension, 3rd arg string user_extension")
{
	xBEGIN
	std::string call_id;
	std::string line_extension;
	std::string user_extension;
	
	std::stringstream ss;
	params.write(ss);
	theLog->Debug(ss.str());
	
	if(params.getType() == XmlRpcValue::TypeArray && params[0].getType() == XmlRpcValue::TypeStruct)
	{
		call_id = std::string(params[0]["call_id"]);
		line_extension = std::string(params[0]["line_extension"]);
		user_extension = std::string(params[0]["user_extension"]);
	}
	else
	{
		call_id = std::string(params[0]);
		line_extension = std::string(params[1]);
		user_extension = std::string(params[2]);
	}
	theLog->Default(std::string("Received command: Unhold, ") + call_id + ", " + line_extension + ", " + user_extension);
	theTapiConn->Unhold(call_id, line_extension, user_extension);
	result = "SUCCESS";
	xEND
}
XML_RPC_METHOD_IMPLEMENT(Hangup, "1st arg call_id, 2nd arg string line_extension, 3rd arg string user_extension")
{
	xBEGIN
	std::string call_id;
	std::string line_extension;
	std::string user_extension;
	
	std::stringstream ss;
	params.write(ss);
	theLog->Debug(ss.str());
	
	if(params.getType() == XmlRpcValue::TypeArray && params[0].getType() == XmlRpcValue::TypeStruct)
	{
		call_id = std::string(params[0]["call_id"]);
		line_extension = std::string(params[0]["line_extension"]);
		user_extension = std::string(params[0]["user_extension"]);
	}
	else
	{
		call_id = std::string(params[0]);
		line_extension = std::string(params[1]);
		user_extension = std::string(params[2]);
	}
	theLog->Default(std::string("Received command: Hangup, ") + call_id + ", " + line_extension + ", " + user_extension);
	theTapiConn->Hangup(call_id, line_extension, user_extension);
	result = "SUCCESS";
	xEND
}
XML_RPC_METHOD_IMPLEMENT(BlindTransfer, "1st arg call_id, 2nd arg string line_extension, 3rd arg string user_extension, 4rd arg string extension to transfer")
{
	xBEGIN
	std::string call_id;
	std::string line_extension;
	std::string user_extension;
	std::string extension_2_transfer;
	
	std::stringstream ss;
	params.write(ss);
	theLog->Debug(ss.str());
	
	if(params.getType() == XmlRpcValue::TypeArray && params[0].getType() == XmlRpcValue::TypeStruct)
	{
		call_id = std::string(params[0]["call_id"]);
		line_extension = std::string(params[0]["line_extension"]);
		user_extension = std::string(params[0]["user_extension"]);
		extension_2_transfer = std::string(params[0]["extension_2_transfer"]);
	}
	else
	{
		call_id = std::string(params[0]);
		line_extension = std::string(params[1]);
		user_extension = std::string(params[2]);
		extension_2_transfer = std::string(params[3]);
	}

	theLog->Default(std::string("Received command: BlindTransfer, ") + call_id + ", " + line_extension + ", " + user_extension + ", " + extension_2_transfer);
	theTapiConn->BlindTransfer(call_id, line_extension, user_extension, extension_2_transfer);
	result = "SUCCESS";
	xEND
}

XML_RPC_METHOD_IMPLEMENT(Culpa, "")
{
	xBEGIN
	theLog->Default(std::string("Received command: Culpa"));
	Sleep(60000);
	result = "A culpa é do CARECA!";
	xEND
}



