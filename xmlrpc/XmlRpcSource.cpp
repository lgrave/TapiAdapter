//
//	Last Modified: $Date: 2010-07-20 09:48:14 $
//
//	$Log: XmlRpcSource.cpp,v $
//	Revision 1.2  2010-07-20 09:48:14  lgrave
//	corrected windows crlf to unix lf
//

//	Revision 1.1  2010-07-19 23:40:45  lgrave

//	1st version added to cvs

//
//

#include "XmlRpcSource.h"
#include "XmlRpcSocket.h"
#include "XmlRpcUtil.h"

namespace XmlRpc {


  XmlRpcSource::XmlRpcSource(int fd /*= -1*/, bool deleteOnClose /*= false*/) 
    : _fd(fd), _deleteOnClose(deleteOnClose), _keepOpen(false)
  {
  }

  XmlRpcSource::~XmlRpcSource()
  {
  }


  void
  XmlRpcSource::close()
  {
    if (_fd != -1) {
      XmlRpcUtil::log(2,"XmlRpcSource::close: closing socket %d.", _fd);
      XmlRpcSocket::close(_fd);
      XmlRpcUtil::log(2,"XmlRpcSource::close: done closing socket %d.", _fd);
      _fd = -1;
    }
    if (_deleteOnClose) {
      XmlRpcUtil::log(2,"XmlRpcSource::close: deleting this");
      _deleteOnClose = false;
      delete this;
    }
  }

} // namespace XmlRpc
