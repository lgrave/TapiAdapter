//
//	Last Modified: $Date: 2010-07-22 10:16:37 $
//
//	$Log: TestClient.cpp,v $
//	Revision 1.1.2.1  2010-07-22 10:16:37  lgrave
//	RT8: TapiAdapter: threads for reception/execution of paralel commands
//
//	Revision 1.1  2010-07-21 12:12:56  lgrave
//	TestClient for sending commands from the command line
//
//
//

#include "XmlRpc.h"
#include <iostream>
using namespace XmlRpc;

// not used
#include "Log.h"

XmlRpcClient *theClient = NULL;

void Send(std::string command, XmlRpcValue &args)
{
	XmlRpcValue result;
	std::cout << "SEND " << command << ": ";
	args.write(std::cout);
	std::cout << std::endl;
	std::cout << "args type=" << args.getType() << std::endl;
	try
	{
		theClient->execute(command.c_str(), args, result);
	}
	catch(char *errmsg)
	{
		std::cout << "Error calling method " << command << ": " << errmsg << std::endl;
	}
	std::cout << "Result: ";
	result.write(std::cout);
	std::cout << std::endl;
	theClient->close();
}

int main(int argc, char* argv[])
{
	if (argc != 3) {
		std::cerr << "Usage: TestClient serverHost serverPort\n";
		return -1;
	}
	int port = atoi(argv[2]);
	
	theClient = new XmlRpcClient(argv[1], port);
	
	std::string command;
	std::string argument;
	std::string value;
	XmlRpcValue args;
	
	std::cout << "Command: " << std::flush;
	std::cin >> command;
	while(command != ".")
	{
		args.clear();
		std::cout << "Argument: " << std::flush;
		std::cin >> argument;
		while(argument != ".")
		{
			std::cout << argument << "=" << std::flush;
			std::cin >> value;
			args[argument] = value;
			std::cout << "Argument: " << std::flush;
			std::cin >> argument;
		}
		Send(command, args);
		std::cout << "Command: " << std::flush;
		std::cin >> command;
	}
}