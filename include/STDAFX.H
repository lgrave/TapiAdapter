//
//	Last Modified: $Date: 2010-07-22 10:16:36 $
//
//	$Log: STDAFX.H,v $
//	Revision 1.2.2.1  2010-07-22 10:16:36  lgrave
//	RT8: TapiAdapter: threads for reception/execution of paralel commands
//
//	Revision 1.2  2010-07-20 09:48:12  lgrave
//	corrected windows crlf to unix lf
//
//	Revision 1.1  2010-07-19 23:40:42  lgrave
//	1st version added to cvs
//
//

// STDAFX.H
//
// TAPI application class library v1.00
// This file contains the globals and precompiled headers.
// 
// This is a part of the TAPI Applications Classes C++ library.
// Original Copyright � 1995-2004 JulMar Entertainment Technology, Inc. All rights reserved.
//
// "This program is free software; you can redistribute it and/or modify it under the terms of 
// the GNU General Public License as published by the Free Software Foundation; version 2 of the License.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without 
// even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General 
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program; if not, write 
// to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
// Or, contact: JulMar Technology, Inc. at: info@julmar.com." 
//

#define TAPI_CURRENT_VERSION 0x00030001
//#define WINVER 0x4000

//#define WIN32 1

#include <afxwin.h>				// Standard MFC definitions
//#include <afxext.h>				// MFC extensions
#include <afxmt.h>				// Multi-threaded classes
#include <tapi.h>				// TAPI support
