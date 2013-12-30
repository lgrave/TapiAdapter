//
//	Last Modified: $Date: 2010-07-20 09:48:12 $
//
//	$Log: Tapireq.cpp,v $
//	Revision 1.2  2010-07-20 09:48:12  lgrave
//	corrected windows crlf to unix lf
//

//	Revision 1.1  2010-07-19 23:40:42  lgrave

//	1st version added to cvs

//
//

// TAPIREQ.CPP
//
// This file contains the asynch. request completion functions
// for the class library.
// 
// This is a part of the TAPI Applications Classes C++ library.
// Original Copyright © 1995-2004 JulMar Entertainment Technology, Inc. All rights reserved.
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

#include "stdafx.h"
#include "atapi.h"

//////////////////////////////////////////////////////////////////////////////
// CTapiRequest::CTapiRequest
//
// Request object constructor
//
CTapiRequest::CTapiRequest(DWORD dwRequestID) : m_evtComplete (FALSE, TRUE)
{
	m_dwRequestID = dwRequestID;
    m_dwCompleteTime = 0L;
	m_lResult = -1L;

}// CTapiRequest::CTapiRequest

//////////////////////////////////////////////////////////////////////////////
// CTapiRequest::~CTapiRequest
//
// Destructor for the request object
//
CTapiRequest::~CTapiRequest()
{
	m_evtComplete.SetEvent();

}// CTapiRequest::~CTapiRequest

////////////////////////////////////////////////////////////////////////////////////
// CTapiRequest::IsPending
//
// Return "TRUE" result if the request has not been completed by TAPI
// yet.
//
BOOL CTapiRequest::IsPending()
{
	return (WaitForSingleObject(m_evtComplete,0) == WAIT_TIMEOUT);

}// CTapiRequest::IsPending

////////////////////////////////////////////////////////////////////////////////////
// CTapiRequest::OnRequestComplete
//
// Complete this request
//
void CTapiRequest::OnRequestComplete (LONG lResult)
{
	m_lResult = lResult;
	m_dwCompleteTime = GetTickCount();
	m_evtComplete.SetEvent();

}// CTapiRequest::OnRequestComplete

////////////////////////////////////////////////////////////////////////////////////
// CTapiRequest::GetResult
//
// Return the result for this request.  Possibly wait for a response from 
// TAPI.
//
LONG CTapiRequest::GetResult (LONG lTimeout /*=0*/)
{
    MSG msg;

	// If this thread is the EVENT thread, ASSERT!
	ASSERT (GetCurrentThreadId() != GetTAPIConnection()->m_pMonitorThread_L->m_nThreadID);
	ASSERT (GetCurrentThreadId() != GetTAPIConnection()->m_pMonitorThread_P->m_nThreadID);

	// Place a reasonable limit on the timeout.
	DWORD dwTicks = (lTimeout != 0 && lTimeout != INFINITE) ? GetTickCount() + lTimeout : 0;
	while (IsPending() ) //&& (dwTicks > GetTickCount()))
	{
		if (dwTicks > 0 && dwTicks < GetTickCount())
			return -1L;

        while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
			if (msg.message == WM_QUIT)
			{
				AfxGetMainWnd()->PostMessage(WM_QUIT);
				return -1L;
			}

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

		Sleep(5);
    }
    return m_lResult;
	
}// CTapiRequest::GetResult

