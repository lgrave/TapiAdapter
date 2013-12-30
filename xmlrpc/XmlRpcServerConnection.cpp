//
//	Last Modified: $Date: 2010-07-19 23:40:45 $
//
//	$Log: XmlRpcServerConnection.cpp,v $
//	Revision 1.1  2010-07-19 23:40:45  lgrave
//	1st version added to cvs
//
//

#include "XmlRpcServerConnection.h"

#include "..\src\Log.h"

#include "XmlRpcSocket.h"
#include "XmlRpc.h"
#ifndef MAKEDEPEND
# include <stdio.h>
# include <stdlib.h>
#endif

using namespace XmlRpc;

// Static data
const char XmlRpcServerConnection::METHODNAME_TAG[] = "<methodName>";
const char XmlRpcServerConnection::PARAMS_TAG[] = "<params>";
const char XmlRpcServerConnection::PARAMS_ETAG[] = "</params>";
const char XmlRpcServerConnection::PARAM_TAG[] = "<param>";
const char XmlRpcServerConnection::PARAM_ETAG[] = "</param>";

const std::string XmlRpcServerConnection::SYSTEM_MULTICALL = "system.multicall";
const std::string XmlRpcServerConnection::METHODNAME = "methodName";
const std::string XmlRpcServerConnection::PARAMS = "params";

const std::string XmlRpcServerConnection::FAULTCODE = "faultCode";
const std::string XmlRpcServerConnection::FAULTSTRING = "faultString";



// The server delegates handling client requests to a serverConnection object.
XmlRpcServerConnection::XmlRpcServerConnection(int fd, XmlRpcServer* server, bool deleteOnClose /*= false*/) :
  XmlRpcSource(fd, deleteOnClose)
{
  XmlRpcUtil::log(2,"XmlRpcServerConnection: new socket %d.", fd);
  _server = server;
  _connectionState = READ_HEADER;
  _keepAlive = true;
}


XmlRpcServerConnection::~XmlRpcServerConnection()
{
  XmlRpcUtil::log(4,"XmlRpcServerConnection dtor.");
  _server->removeConnection(this);
}


// Handle input on the server socket by accepting the connection
// and reading the rpc request. Return true to continue to monitor
// the socket for events, false to remove it from the dispatcher.
unsigned
XmlRpcServerConnection::handleEvent(unsigned /*eventType*/)
{
  if (_connectionState == READ_HEADER)
    if ( ! readHeader()) return 0;

  if (_connectionState == READ_REQUEST)
    if ( ! readRequest()) return 0;

  if (_connectionState == WRITE_RESPONSE)
    if ( ! writeResponse()) return 0;

  return (_connectionState == WRITE_RESPONSE) 
        ? XmlRpcDispatch::WritableEvent : XmlRpcDispatch::ReadableEvent;
}


bool
XmlRpcServerConnection::readHeader()
{
  // Read available data
_x
  bool eof;
_x
  if ( ! XmlRpcSocket::nbRead(this->getfd(), _header, &eof)) {
    // Its only an error if we already have read some data
_x
    if (_header.length() > 0)
      XmlRpcUtil::error("XmlRpcServerConnection::readHeader: error while reading header (%s).",XmlRpcSocket::getErrorMsg().c_str());
_x
    return false;
  }

_x
  XmlRpcUtil::log(4, "XmlRpcServerConnection::readHeader: read %d bytes.", _header.length());
_x
  char *hp = (char*)_header.c_str();  // Start of header
_x
  char *ep = hp + _header.length();   // End of string
_x
  char *bp = 0;                       // Start of body
_x
  char *lp = 0;                       // Start of content-length value
_x
  char *kp = 0;                       // Start of connection value
_x

  for (char *cp = hp; (bp == 0) && (cp < ep); ++cp) {
_x
	if ((ep - cp > 16) && (strncasecmp(cp, "Content-length: ", 16) == 0))
	  lp = cp + 16;
	else if ((ep - cp > 12) && (strncasecmp(cp, "Connection: ", 12) == 0))
	  kp = cp + 12;
	else if ((ep - cp > 4) && (strncmp(cp, "\r\n\r\n", 4) == 0))
	  bp = cp + 4;
	else if ((ep - cp > 2) && (strncmp(cp, "\n\n", 2) == 0))
	  bp = cp + 2;
_x
  }
_x

  // If we haven't gotten the entire header yet, return (keep reading)
  if (bp == 0) {
_x
    // EOF in the middle of a request is an error, otherwise its ok
    if (eof) {
_x
      XmlRpcUtil::log(4, "XmlRpcServerConnection::readHeader: EOF");
_x
      if (_header.length() > 0)
        XmlRpcUtil::error("XmlRpcServerConnection::readHeader: EOF while reading header");
_x
      return false;   // Either way we close the connection
    }
    
_x
    return true;  // Keep reading
  }

  // Decode content length
_x
  if (lp == 0) {
_x
    XmlRpcUtil::error("XmlRpcServerConnection::readHeader: No Content-length specified");
_x
    return false;   // We could try to figure it out by parsing as we read, but for now...
  }

_x
  _contentLength = atoi(lp);
_x
  if (_contentLength <= 0) {
_x
    XmlRpcUtil::error("XmlRpcServerConnection::readHeader: Invalid Content-length specified (%d).", _contentLength);
_x
    return false;
  }
  	
_x
  XmlRpcUtil::log(3, "XmlRpcServerConnection::readHeader: specified content length is %d.", _contentLength);

_x
  // Otherwise copy non-header data to request buffer and set state to read request.
  _request = bp;

_x
  // Parse out any interesting bits from the header (HTTP version, connection)
  _keepAlive = true;
_x
  if (_header.find("HTTP/1.0") != std::string::npos) {
_x
    if (kp == 0 || strncasecmp(kp, "keep-alive", 10) != 0)
      _keepAlive = false;           // Default for HTTP 1.0 is to close the connection
  } else {
_x
    if (kp != 0 && strncasecmp(kp, "close", 5) == 0)
      _keepAlive = false;
_x
  }
_x
  XmlRpcUtil::log(3, "KeepAlive: %d", _keepAlive);
_x


  _header = ""; 
_x
  _connectionState = READ_REQUEST;
_x
  return true;    // Continue monitoring this source
}

bool
XmlRpcServerConnection::readRequest()
{
_x
  // If we dont have the entire request yet, read available data
  if (int(_request.length()) < _contentLength) {
_x
    bool eof;
_x
    if ( ! XmlRpcSocket::nbRead(this->getfd(), _request, &eof)) {
_x
      XmlRpcUtil::error("XmlRpcServerConnection::readRequest: read error (%s).",XmlRpcSocket::getErrorMsg().c_str());
_x
      return false;
    }
_x

    // If we haven't gotten the entire request yet, return (keep reading)
    if (int(_request.length()) < _contentLength) {
_x
      if (eof) {
_x
        XmlRpcUtil::error("XmlRpcServerConnection::readRequest: EOF while reading request");
_x
        return false;   // Either way we close the connection
      }
_x
      return true;
    }
_x
  }
_x

  // Otherwise, parse and dispatch the request
  XmlRpcUtil::log(3, "XmlRpcServerConnection::readRequest read %d bytes.", _request.length());
  //XmlRpcUtil::log(5, "XmlRpcServerConnection::readRequest:\n%s\n", _request.c_str());

_x
  _connectionState = WRITE_RESPONSE;
_x

  return true;    // Continue monitoring this source
}


bool
XmlRpcServerConnection::writeResponse()
{
_x
  if (_response.length() == 0) {
_x
    executeRequest();
_x
    _bytesWritten = 0;
_x
    if (_response.length() == 0) {
_x
      XmlRpcUtil::error("XmlRpcServerConnection::writeResponse: empty response.");
_x
      return false;
    }
_x
  }
_x

  // Try to write the response
  if ( ! XmlRpcSocket::nbWrite(this->getfd(), _response, &_bytesWritten)) {
_x
    XmlRpcUtil::error("XmlRpcServerConnection::writeResponse: write error (%s).",XmlRpcSocket::getErrorMsg().c_str());
_x
    return false;
  }
_x
  XmlRpcUtil::log(3, "XmlRpcServerConnection::writeResponse: wrote %d of %d bytes.", _bytesWritten, _response.length());
_x

  // Prepare to read the next request
  if (_bytesWritten == int(_response.length())) {
_x
    _header = "";
_x
    _request = "";
_x
    _response = "";
_x
    _connectionState = READ_HEADER;
_x
  }
_x

  return _keepAlive;    // Continue monitoring this source if true
}

// Run the method, generate _response string
void
XmlRpcServerConnection::executeRequest()
{
_x
  XmlRpcValue params, resultValue;
_x
  std::string methodName = parseRequest(params);
_x
  XmlRpcUtil::log(2, "XmlRpcServerConnection::executeRequest: server calling method '%s'", 
                    methodName.c_str());
_x

  try {
_x

    if ( ! executeMethod(methodName, params, resultValue) &&
         ! executeMulticall(methodName, params, resultValue))
      generateFaultResponse(methodName + ": unknown method name");
    else
      generateResponse(resultValue.toXml());
_x

  } catch (const XmlRpcException& fault) {
_x
    XmlRpcUtil::log(2, "XmlRpcServerConnection::executeRequest: fault %s.",
                    fault.getMessage().c_str()); 
_x
    generateFaultResponse(fault.getMessage(), fault.getCode());
_x
  }
_x
}

// Parse the method name and the argument values from the request.
std::string
XmlRpcServerConnection::parseRequest(XmlRpcValue& params)
{
_x
  int offset = 0;   // Number of chars parsed from the request
_x

  std::string methodName = XmlRpcUtil::parseTag(METHODNAME_TAG, _request, &offset);
_x

  if (methodName.size() > 0 && XmlRpcUtil::findTag(PARAMS_TAG, _request, &offset))
  {
_x
    int nArgs = 0;
_x
    while (XmlRpcUtil::nextTagIs(PARAM_TAG, _request, &offset)) {
_x
      params[nArgs++] = XmlRpcValue(_request, &offset);
_x
      (void) XmlRpcUtil::nextTagIs(PARAM_ETAG, _request, &offset);
_x
    }
_x

    (void) XmlRpcUtil::nextTagIs(PARAMS_ETAG, _request, &offset);
  }
_x

  return methodName;
}

// Execute a named method with the specified params.
bool
XmlRpcServerConnection::executeMethod(const std::string& methodName, 
                                      XmlRpcValue& params, XmlRpcValue& result)
{
_x
  XmlRpcServerMethod* method = _server->findMethod(methodName);

_x
  if ( ! method) return false;

_x
  method->execute(params, result);

_x
  // Ensure a valid result value
  if ( ! result.valid())
      result = std::string();
_x

  return true;
}

// Execute multiple calls and return the results in an array.
bool
XmlRpcServerConnection::executeMulticall(const std::string& methodName, 
                                         XmlRpcValue& params, XmlRpcValue& result)
{
_x
  if (methodName != SYSTEM_MULTICALL) return false;

_x
  // There ought to be 1 parameter, an array of structs
  if (params.size() != 1 || params[0].getType() != XmlRpcValue::TypeArray)
    throw XmlRpcException(SYSTEM_MULTICALL + ": Invalid argument (expected an array)");

_x
  int nc = params[0].size();
_x
  result.setSize(nc);
_x

  for (int i=0; i<nc; ++i) {
_x

    if ( ! params[0][i].hasMember(METHODNAME) ||
         ! params[0][i].hasMember(PARAMS)) {
_x
      result[i][FAULTCODE] = -1;
_x
      result[i][FAULTSTRING] = SYSTEM_MULTICALL +
              ": Invalid argument (expected a struct with members methodName and params)";
_x
      continue;
    }

_x
    const std::string& methodName = params[0][i][METHODNAME];
_x
    XmlRpcValue& methodParams = params[0][i][PARAMS];
_x

    XmlRpcValue resultValue;
_x
    resultValue.setSize(1);
_x
    try {
_x
      if ( ! executeMethod(methodName, methodParams, resultValue[0]) &&
           ! executeMulticall(methodName, params, resultValue[0]))
      {
_x
        result[i][FAULTCODE] = -1;
_x
        result[i][FAULTSTRING] = methodName + ": unknown method name";
_x
      }
      else
        result[i] = resultValue;
_x

    } catch (const XmlRpcException& fault) {
_x
        result[i][FAULTCODE] = fault.getCode();
_x
        result[i][FAULTSTRING] = fault.getMessage();
_x
    }
_x
  }
_x

  return true;
}


// Create a response from results xml
void
XmlRpcServerConnection::generateResponse(std::string const& resultXml)
{
_x
  const char RESPONSE_1[] = 
    "<?xml version=\"1.0\"?>\r\n"
    "<methodResponse><params><param>\r\n\t";
_x
  const char RESPONSE_2[] =
    "\r\n</param></params></methodResponse>\r\n";

_x
  std::string body = RESPONSE_1 + resultXml + RESPONSE_2;
_x
  std::string header = generateHeader(body);
_x

  _response = header + body;
_x
  XmlRpcUtil::log(5, "XmlRpcServerConnection::generateResponse:\n%s\n", _response.c_str()); 
_x
}

// Prepend http headers
std::string
XmlRpcServerConnection::generateHeader(std::string const& body)
{
_x
  std::string header = 
    "HTTP/1.1 200 OK\r\n"
    "Server: ";
_x
  header += XMLRPC_VERSION;
_x
  header += "\r\n"
    "Content-Type: text/xml\r\n"
    "Content-length: ";

_x
  char buffLen[40];
_x
  sprintf(buffLen,"%d\r\n\r\n", body.size());
_x

  return header + buffLen;
}


void
XmlRpcServerConnection::generateFaultResponse(std::string const& errorMsg, int errorCode)
{
_x
  const char RESPONSE_1[] = 
    "<?xml version=\"1.0\"?>\r\n"
    "<methodResponse><fault>\r\n\t";
_x
  const char RESPONSE_2[] =
    "\r\n</fault></methodResponse>\r\n";

_x
  XmlRpcValue faultStruct;
_x
  faultStruct[FAULTCODE] = errorCode;
_x
  faultStruct[FAULTSTRING] = errorMsg;
_x
  std::string body = RESPONSE_1 + faultStruct.toXml() + RESPONSE_2;
_x
  std::string header = generateHeader(body);
_x

  _response = header + body;
_x
}

