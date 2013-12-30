#
#	Last Modified: $Date: 2010-07-22 10:16:38 $
#
#	$Log: Makefile,v $
#	Revision 1.1.2.1  2010-07-22 10:16:38  lgrave
#	RT8: TapiAdapter: threads for reception/execution of paralel commands
#
#	Revision 1.1  2010-07-19 23:40:45  lgrave
#	1st version added to cvs
#
#

CPPFLAGS = /nologo /EHsc /MT $(CPPFLAGS) /I ..\include /D_WINDOWS /DWIN32=1 /DWINVER=0x4000 /DUSE_WINDOWS_DEBUG

LIB_DIR = ..\lib
BIN_DIR = ..\bin
OBJS = XmlRpcClient.obj XmlRpcDispatch.obj XmlRpcServer.obj XmlRpcServerConnection.obj XmlRpcServerMethod.obj XmlRpcSocket.obj XmlRpcSource.obj XmlRpcUtil.obj XmlRpcValue.obj MultithreadXmlRpcServer.obj Thread.obj

$(LIB_DIR)\XmlRpc.lib: $(OBJS)
	LIB /NOLOGO $(OBJS) /OUT:$@

clean:
	DEL $(OBJS) $(LIB_DIR)\XmlRpc.lib
