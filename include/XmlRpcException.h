//
//	Last Modified: $Date: 2010-07-19 23:40:42 $
//
//	$Log: XmlRpcException.h,v $
//	Revision 1.1  2010-07-19 23:40:42  lgrave
//	1st version added to cvs
//
//

#ifndef _XMLRPCEXCEPTION_H_
#define _XMLRPCEXCEPTION_H_
//
// XmlRpc++ Copyright (c) 2002-2003 by Chris Morley
//
#if defined(_MSC_VER)
# pragma warning(disable:4786)    // identifier was truncated in debug info
#endif

#ifndef MAKEDEPEND
# include <string>
#endif


namespace XmlRpc {

  //! A class representing an error.
  //! If server methods throw this exception, a fault response is returned
  //! to the client.
  class XmlRpcException {
  public:
    //! Constructor
    //!   @param message  A descriptive error message
    //!   @param code     An integer error code
    XmlRpcException(const std::string& message, int code=-1) :
        _message(message), _code(code) {}

    //! Return the error message.
    const std::string& getMessage() const { return _message; }

    //! Return the error code.
    int getCode() const { return _code; }

  private:
    std::string _message;
    int _code;
  };

}

#endif	// _XMLRPCEXCEPTION_H_
