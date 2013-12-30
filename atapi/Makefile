#
#	Last Modified: $Date: 2010-07-22 10:16:36 $
#
#	$Log: Makefile,v $
#	Revision 1.2.2.1  2010-07-22 10:16:36  lgrave
#	RT8: TapiAdapter: threads for reception/execution of paralel commands
#
#	Revision 1.2  2010-07-20 09:48:11  lgrave
#	corrected windows crlf to unix lf
#
#	Revision 1.1  2010-07-19 23:40:41  lgrave
#	1st version added to cvs
#
#

CPPFLAGS = /nologo /EHsc /MT $(CPPFLAGS) /I ..\include /D_WINDOWS /DWIN32=1 /DWINVER=0x4000

LIB_DIR = ..\lib
OBJS = Tapiaddr.obj Tapicall.obj Tapiconn.obj Tapiline.obj Tapiobj.obj Tapiphone.obj Tapireq.obj

$(LIB_DIR)\Atapi.lib: $(OBJS)
	ECHO $(CPPFLAGS)
	LIB /NOLOGO $(OBJS) /OUT:$@
	
clean:
	DEL $(OBJS) $(LIB_DIR)\Atapi.lib
