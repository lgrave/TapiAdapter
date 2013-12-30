#
#	Last Modified: $Date: 2010-07-21 12:12:56 $
#
#	$Log: Makefile,v $
#	Revision 1.3  2010-07-21 12:12:56  lgrave
#	TestClient for sending commands from the command line
#
#	Revision 1.2  2010-07-20 09:48:11  lgrave
#	corrected windows crlf to unix lf
#
#	Revision 1.1  2010-07-19 23:40:40  lgrave
#	1st version added to cvs
#
#

# Main Makefile file


all: TapiAdapter.exe TestClient.exe

TapiAdapter.exe: Atapi.lib XmlRpc.lib
	CD src
	$(MAKE) /NOLOGO ..\bin\TapiAdapter.exe
	CD ..
	
TestClient.exe: XmlRpc.lib
	CD src
	$(MAKE) /NOLOGO ..\bin\TestClient.exe
	CD ..

Atapi.lib:
	CD atapi
	$(MAKE) /NOLOGO ..\lib\Atapi.lib
	CD ..

XmlRpc.lib:
	CD xmlrpc
	$(MAKE) /NOLOGO ..\lib\XmlRpc.lib
	CD ..

clean:
	CD atapi
	$(MAKE) /NOLOGO $@
	CD ..
	CD xmlrpc
	$(MAKE) /NOLOGO $@
	CD ..
	CD src
	$(MAKE) /NOLOGO $@
	CD ..

