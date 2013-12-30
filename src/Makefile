#
#	Last Modified: $Date: 2010-07-22 10:16:37 $
#
#	$Log: Makefile,v $
#	Revision 1.2.2.1  2010-07-22 10:16:37  lgrave
#	RT8: TapiAdapter: threads for reception/execution of paralel commands
#
#	Revision 1.2  2010-07-21 12:12:56  lgrave
#	TestClient for sending commands from the command line
#
#	Revision 1.1  2010-07-19 23:40:44  lgrave
#	1st version added to cvs
#
#

CPPFLAGS = /nologo /EHsc /MT $(CPPFLAGS) /I ..\include /D_WINDOWS /DWIN32=1 /DWINVER=0x4000

OBJS = TapiAdapter.obj Settings.obj XmlRpcMethods.obj TapiConn.obj Log.obj 
LIBS = nafxcw.lib version.lib Advapi32.lib uuid.lib ..\lib\Atapi.lib ..\lib\XmlRpc.lib ws2_32.lib ole32.lib oleaut32.lib tapi32.lib

BIN_DIR = ..\bin

TapiAdapter.obj: Log.h Settings.h


$(BIN_DIR)\TapiAdapter.exe: $(OBJS) 
	LINK /NOLOGO /NOD:nafxcw.lib $(LIBS) $(OBJS) /OUT:$@
	COPY $(BIN_DIR)\TapiAdapter.exe "C:\Documents and Settings\Bete\Meus Documentos\My Dropbox\transfer"
	COPY ..\TapiAdapter.ini "C:\Documents and Settings\Bete\Meus Documentos\My Dropbox\transfer"

$(BIN_DIR)\TestClient.exe: TestClient.obj Log.obj
	LINK /NOLOGO /NOD:nafxcw.lib nafxcw.lib ws2_32.lib ..\lib\XmlRpc.lib Log.obj TestClient.obj /OUT:$@
	COPY $(BIN_DIR)\TestClient.exe "C:\Documents and Settings\Bete\Meus Documentos\My Dropbox\transfer"

clean:
	DEL $(OBJS) TestClient.obj $(BIN_DIR)\TapiAdapter.exe $(BIN_DIR)\TestClient.exe
