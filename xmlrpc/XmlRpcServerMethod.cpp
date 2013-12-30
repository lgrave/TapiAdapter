//
//	Last Modified: $Date: 2010-07-20 09:48:14 $
//
//	$Log: XmlRpcServerMethod.cpp,v $
//	Revision 1.2  2010-07-20 09:48:14  lgrave
//	corrected windows crlf to unix lf
//

//	Revision 1.1  2010-07-19 23:40:45  lgrave

//	1st version added to cvs

//
//

#include "XmlRpcServerMethod.h"
#include "XmlRpcServer.h"

namespace XmlRpc {


  XmlRpcServerMethod::XmlRpcServerMethod(std::string const& name, XmlRpcServer* server)
  {
    _name = name;
    _server = server;
    if (_server) _server->addMethod(this);
  }

  XmlRpcServerMethod::~XmlRpcServerMethod()
  {
    if (_server) _server->removeMethod(this);
  }


} // namespace XmlRpc
