//
//	Last Modified: $Date: 2010-07-20 09:48:12 $
//
//	$Log: Tapiconn.cpp,v $
//	Revision 1.2  2010-07-20 09:48:12  lgrave
//	corrected windows crlf to unix lf
//

//	Revision 1.1  2010-07-19 23:40:41  lgrave

//	1st version added to cvs

//
//

// TAPICONN.CPP
//
// This file contains the TAPI connection functions for the 
// class library.
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

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////////////////////////////
// GetTAPIConnection
//
// Retrieve the global TAPI connection object
//
CTapiConnection* GetTAPIConnection()
{                                   
	static CTapiConnection g_connTapi;
    return &g_connTapi;
    
}// GetTAPIConnection

////////////////////////////////////////////////////////////////////////////////////
// _LineEventProc
//
// static CALLBACK function for LINE devices under TAPI
//
UINT _LineEventProc ( LPVOID pParam )
{
	CTapiConnection* pConn = (CTapiConnection*) pParam;
	pConn->LineEventProc();
	return 0;

}// _LineEventProc

////////////////////////////////////////////////////////////////////////////////////
// _PhoneEventProc
//
// static CALLBACK function for PHONE devices under TAPI
//
UINT _PhoneEventProc ( LPVOID pParam )
{
	CTapiConnection* pConn = (CTapiConnection*) pParam;
	pConn->PhoneEventProc();
	return 0;

}// _PhoneEventProc

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::CTapiConnection
//
// Constructor for the TAPI connection class.
//
CTapiConnection::CTapiConnection() : m_hLineApp(0), m_hPhoneApp(0), 
	m_pLineClass(0), m_pPhoneClass(0), m_pCallClass(0), m_pAddrClass(0),
	m_iProviderPos(0), m_dwNumLines(0), m_dwNumPhones(0)
{   
}// CTapiConnection::CTapiConnection

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::~CTapiConnection                     
//
// Destructor for the TAPI connection
//
CTapiConnection::~CTapiConnection()
{   
	Shutdown();
    
}// CTapiConnection::~CTapiConnection

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::Init
//
// Initialize the TAPI connection
//
LONG CTapiConnection::Init(LPCTSTR pszAppName, 
                    CRuntimeClass* prtLine, CRuntimeClass* prtAddr,
                    CRuntimeClass* prtCall, CRuntimeClass* prtPhone, 
					DWORD dwAPIVersion)
{
	// If we are already initialized, return an error.
	if (m_hLineApp != NULL || m_hPhoneApp != NULL)
		return LINEERR_NOMULTIPLEINSTANCE;
	
    // Save off the runtime information needed for the line/call
    m_pLineClass  = (prtLine)  ? prtLine  : RUNTIME_CLASS (CTapiLine);
    m_pAddrClass  = (prtAddr)  ? prtAddr  : RUNTIME_CLASS (CTapiAddress);        
    m_pCallClass  = (prtCall)  ? prtCall  : RUNTIME_CLASS (CTapiCall);
	m_pPhoneClass = (prtPhone) ? prtPhone : RUNTIME_CLASS (CTapiPhone);
    
    // Initialize the LINE portion of our connection
	LONG lResult = InitLines(pszAppName, dwAPIVersion);
	if (lResult != 0)
		return lResult;

	lResult = InitPhones(pszAppName, dwAPIVersion);
	if (lResult != 0)
		Shutdown();

	return lResult;
    
}// CTapiConnection::Init                    

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::InitLines
//
// Initialize the line portion of our TAPI connection
//
LONG CTapiConnection::InitLines(LPCTSTR pszAppName, DWORD dwAPIVersion)
{
	LONG lResult = -1;

	// Initialize the line portion of TAPI..
	LINEINITIALIZEEXPARAMS lip;
	ZeroMemory (&lip, sizeof(LINEINITIALIZEEXPARAMS));
    while (lResult != 0)
    {
		lip.dwTotalSize = sizeof(LINEINITIALIZEEXPARAMS);
		lip.dwOptions = LINEINITIALIZEEXOPTION_USEEVENT;
        lResult = lineInitializeEx (&m_hLineApp, AfxGetInstanceHandle(),
                         NULL, pszAppName, &m_dwNumLines, &dwAPIVersion, &lip);
        if (lResult != LINEERR_REINIT)
            break;
    }

	// If we were unsuccessful then return an error
	if (lResult != 0)
		return lResult;

	// Otherwise, open the line
	m_hTapiEvent_L = lip.Handles.hEvent;
	if (m_hTapiEvent_L == NULL)
		return LINEERR_NOMEM;

	// Create the thread which will monitor TAPI events.
	m_pMonitorThread_L = AfxBeginThread((AFX_THREADPROC) _LineEventProc, (void*)this);
	if (m_pMonitorThread_L == NULL)
	{
		TRACE(_T("Failed to create monitor thread\r\n"));
		lineShutdown(m_hLineApp);
		m_hLineApp = NULL;
		return LINEERR_OPERATIONFAILED;
	}

    // Now create the line objects
    if (lResult == 0)
    {   
		CSingleLock Lock (&m_semLines, TRUE);
        for (DWORD dwDeviceID = 0; dwDeviceID < m_dwNumLines; dwDeviceID++)
        {
            CTapiLine* pLine = (CTapiLine*) m_pLineClass->CreateObject();
            pLine->Init (this, dwDeviceID);
            m_arrLines.Add (pLine);
        }             
    }

    return lResult;

}// CTapiConnection::InitLines

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::InitPhones
//
// Initialize the phone portion of our TAPI connection
//
LONG CTapiConnection::InitPhones(LPCTSTR pszAppName, DWORD dwAPIVersion)
{
	LONG lResult = -1;

	// Initialize the phone portion of TAPI..
	PHONEINITIALIZEEXPARAMS pip;
	ZeroMemory (&pip, sizeof(PHONEINITIALIZEEXPARAMS));
    while (lResult != 0)
    {
		pip.dwTotalSize = sizeof(PHONEINITIALIZEEXPARAMS);
		pip.dwOptions = PHONEINITIALIZEEXOPTION_USEEVENT;
        lResult = phoneInitializeEx (&m_hPhoneApp, AfxGetInstanceHandle(),
                         NULL, pszAppName, &m_dwNumPhones, &dwAPIVersion, &pip);
        if (lResult != PHONEERR_REINIT)
            break;
    }

	// If we were unsuccessful then return an error
	if (lResult != 0)
		return lResult;

	// Otherwise, open the line
	m_hTapiEvent_P = pip.Handles.hEvent;
	if (m_hTapiEvent_P == NULL)
		return PHONEERR_NOMEM;

	// Create the thread which will monitor TAPI events.
	m_pMonitorThread_P = AfxBeginThread((AFX_THREADPROC) _PhoneEventProc, (void*)this);
	if (m_pMonitorThread_P == NULL)
	{
		TRACE(_T("Failed to create monitor thread\r\n"));
		phoneShutdown(m_hPhoneApp);
		m_hPhoneApp = NULL;
		return PHONEERR_OPERATIONFAILED;
	}

    // Now create the phone objects
    if (lResult == 0)
    {   
		CSingleLock Lock (&m_semPhones, TRUE);
        for (DWORD dwDeviceID = 0; dwDeviceID < m_dwNumPhones; dwDeviceID++)
        {
            CTapiPhone* pPhone = (CTapiPhone*) m_pPhoneClass->CreateObject();
            pPhone->Init (this, dwDeviceID);
            m_arrPhones.Add (pPhone);
        }             
    }

    return lResult;

}// CTapiConnection::InitPhones

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::Shutdown
//
// Shutdown the tapi connection
//
LONG CTapiConnection::Shutdown()
{
	// Kill any pending events - release all the threads
	StopWaitingForAllRequests();

    // Shutdown our TAPI line handle
	LONG lResult = 0L;
    if (m_hLineApp != NULL)
    {
        lResult = lineShutdown(m_hLineApp);
        m_hLineApp = NULL;
    }

	// Wait for our LINEEVENT thread.
	if (m_pMonitorThread_L != NULL)
	{
		if (WaitForSingleObject(m_pMonitorThread_L->m_hThread, 5000) == WAIT_TIMEOUT)
		{
			TerminateThread(m_pMonitorThread_L->m_hThread, 0);
			delete m_pMonitorThread_L;
		}
	}

	// Shutdown our TAPI phone handle
	if (m_hPhoneApp != NULL)
	{
		lResult = phoneShutdown(m_hPhoneApp);
		m_hPhoneApp = NULL;
	}

	// Wait for our PHONEEVENT thread
	if (m_pMonitorThread_P != NULL)
	{
		if (WaitForSingleObject(m_pMonitorThread_P->m_hThread, 5000) == WAIT_TIMEOUT)
		{
			TerminateThread(m_pMonitorThread_P->m_hThread, 0);
			delete m_pMonitorThread_P;
		}
	}

    // Delete all our line objects
	CSingleLock Lock(&m_semLines, TRUE);
    for (int i = 0; i < m_arrLines.GetSize(); i++)
    {
        CTapiLine* pLine = (CTapiLine*) m_arrLines.GetAt(i);
        delete pLine;
    }
    m_arrLines.RemoveAll();

	// And now our phone objects
	CSingleLock Lock2 (&m_semPhones, TRUE);
    for (int i = 0; i < m_arrPhones.GetSize(); i++)
    {
        CTapiPhone* pPhone = (CTapiPhone*) m_arrPhones.GetAt(i);
        delete pPhone;
    }
    m_arrPhones.RemoveAll();

	return lResult;

}// CTapiConnection::Shutdown

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::GetPendingRequestCount
//
// Return the total number of pending requests
//
unsigned int CTapiConnection::GetPendingRequestCount()
{
	CSingleLock keyReq (&m_semRequest);
	if (keyReq.Lock() == TRUE)
	{
		int iCount = 0;
		for (POSITION pos = m_arrWaitingRequests.GetHeadPosition(); pos != NULL; )
		{   
			CTapiRequest* pReq = (CTapiRequest*) m_arrWaitingRequests.GetNext(pos);
			if (pReq->IsPending())
				iCount++;
		}
		return iCount;
	}
	return 0;

}// CTapiConnection::GetPendingRequestCount

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::StopWaitingForAllRequests
//
// Kill all pending requests
//
void CTapiConnection::StopWaitingForAllRequests()
{
	CSingleLock keyReq (&m_semRequest);
	if (keyReq.Lock() == TRUE)
	{
		// Delete any pending requests - release any waiting threads
		for (POSITION pos = m_arrWaitingRequests.GetHeadPosition(); pos != NULL; )
		{   
			CTapiRequest* pReq = (CTapiRequest*) m_arrWaitingRequests.GetNext(pos);
			delete pReq;
		}
		m_arrWaitingRequests.RemoveAll();
	}

}// CTapiConnection::StopWaitingForAllRequests
                    
////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::GetLineDeviceCount
//
// Return the total number of line devices in the system.
//
unsigned int CTapiConnection::GetLineDeviceCount() const
{                                      
    return (unsigned int) m_dwNumLines;

}// CTapiConnection::GetLineDeviceCount

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::GetPhoneDeviceCount
//
// Return the total number of phone devices in the system.
//
unsigned int CTapiConnection::GetPhoneDeviceCount() const
{                                      
    return (unsigned int) m_dwNumPhones;

}// CTapiConnection::GetPhoneDeviceCount

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::GetLineAppHandle
//
// Return the TAPI application handle
//
HLINEAPP CTapiConnection::GetLineAppHandle() const
{                                 
    return m_hLineApp;

}// CTapiConnection::GetLineAppHandle

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::GetPhoneAppHandle
//
// Return the TAPI application handle
//
HPHONEAPP CTapiConnection::GetPhoneAppHandle() const
{                                 
    return m_hPhoneApp;

}// CTapiConnection::GetPhoneAppHandle

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::GetLineFromDeviceID
//
// Return the line object from the device id
//
CTapiLine* CTapiConnection::GetLineFromDeviceID (DWORD dwDeviceID) const
{                                       
	CSingleLock Lock (&((CTapiConnection*)this)->m_semLines, TRUE);
    if (dwDeviceID < m_dwNumLines)
        return (CTapiLine*)(m_arrLines[dwDeviceID]);
    return NULL;    

}// CTapiConnection::GetLineFromDeviceID

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::GetPhoneFromDeviceID
//
// Return the phone object from the device id
//
CTapiPhone* CTapiConnection::GetPhoneFromDeviceID (DWORD dwDeviceID) const
{                                       
	CSingleLock Lock (&((CTapiConnection*)this)->m_semPhones, TRUE);
    if (dwDeviceID < m_dwNumPhones)
        return (CTapiPhone*)(m_arrPhones[dwDeviceID]);
    return NULL;    

}// CTapiConnection::GetPhoneFromDeviceID

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::OpenLine
//
// Open a line using LINEMAPPER and a set of calling parameters.
//
LONG CTapiConnection::OpenLine (CTapiLine** pLine,
                                DWORD dwPrivileges, DWORD dwMediaModes, 
								DWORD dwAPIVersion, DWORD dwExtVersion,
                                LPLINECALLPARAMS const lpCallParams)
{                                                                  
    HLINE hLine;
    *pLine = NULL;

	// Use lowest known version if we aren't passed one.
    if (dwAPIVersion == 0)
		dwAPIVersion = TAPIVER_13;

    LONG lResult = lineOpen (GetLineAppHandle(), LINEMAPPER, &hLine, 
                              dwAPIVersion, dwExtVersion, NULL, dwPrivileges, 
							  dwMediaModes, lpCallParams);
            
    // If it was successfull, close the line and reopen it using our
    // line object so we get notifications with the correct callback instance.
    if (lResult == 0)
    {   
        LPVARSTRING lpVarString = (LPVARSTRING) new char[sizeof(VARSTRING)+20];
        lpVarString->dwTotalSize = sizeof(VARSTRING)+20;
        
        lResult = ::lineGetID (hLine, 0, NULL, LINECALLSELECT_LINE, lpVarString, _T("tapi/line"));
        lineClose (hLine);

        if (lResult == 0)
        {                      
            DWORD dwID = *((LPDWORD)((LPCSTR)lpVarString+lpVarString->dwStringOffset));
            delete [] lpVarString;
            lpVarString = NULL;
            
            CTapiLine* pMyLine = GetLineFromDeviceID(dwID);
            if (pMyLine)
            {
                *pLine = pMyLine;
                return pMyLine->Open (dwPrivileges, dwMediaModes, dwAPIVersion, 
									  dwExtVersion, lpCallParams);
            }
            else
                lResult = LINEERR_LINEMAPPERFAILED;
        }
        delete [] lpVarString;
    }              
    
    return lResult;

}// CTapiConnection::OpenLine

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::GetFirstProvider
//
// This function returns the first provider in TAPI.
//
BOOL CTapiConnection::GetFirstProvider (LPTAPIPROVIDER lpProvider)
{                                                      
	CSingleLock Lock (&m_semProviders, TRUE);

    // Remove any old provider listings.
    for (int i = 0 ; i < m_arrProviders.GetSize(); i++)
    {
        LPTAPIPROVIDER lpMyProvider = (LPTAPIPROVIDER) m_arrProviders.GetAt(i);
        delete lpMyProvider;
    }                       
    m_arrProviders.RemoveAll();
    
    // Grab a current list from TAPI
    DWORD dwSize = sizeof(LINEPROVIDERLIST) + (sizeof(LINEPROVIDERENTRY) * 10);
    LPLINEPROVIDERLIST lpProvList = NULL;
    
    while (TRUE)
    {
        lpProvList = (LPLINEPROVIDERLIST) new char [dwSize];
        if (lpProvList == NULL)
			return FALSE;
                
        lpProvList->dwTotalSize = dwSize;
        if (lineGetProviderList (TAPIVER_14, lpProvList) != 0)
        {
			delete [] lpProvList;
            return FALSE;
		}
        
        if (lpProvList->dwNeededSize <= dwSize)
			break;
        dwSize = lpProvList->dwNeededSize;
        delete [] lpProvList;
        lpProvList = NULL;
    }        

    // Now parse through the provider list and build all our structures.
    LPLINEPROVIDERENTRY lpEntry = (LPLINEPROVIDERENTRY) ((LPSTR)lpProvList + lpProvList->dwProviderListOffset);
    for (int i = 0; i < (int) lpProvList->dwNumProviders; i++)
    {
        LPTAPIPROVIDER lpMyProvider = new TAPIPROVIDER;
        lpMyProvider->dwPermanentProviderID = lpEntry->dwPermanentProviderID;
        lpMyProvider->strProviderName = (LPCSTR)((LPSTR)lpProvList+lpEntry->dwProviderFilenameOffset);
        lpEntry++;
        m_arrProviders.Add(lpMyProvider);
    }

    delete [] lpProvList;   
    
    // Return the first one.
    m_iProviderPos = -1;
    return GetNextProvider (lpProvider);

}// CTapiConnection::GetFirstProvider

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::GetNextProvider
//
// Return the next provider from our list
//
BOOL CTapiConnection::GetNextProvider (LPTAPIPROVIDER lpProvider)
{                                   
	CSingleLock Lock (&m_semProviders, TRUE);

    m_iProviderPos++;
    if (m_iProviderPos >= m_arrProviders.GetSize())
    {
        for (int i = 0 ; i < m_arrProviders.GetSize(); i++)
        {
            LPTAPIPROVIDER lpMyProvider = (LPTAPIPROVIDER) m_arrProviders.GetAt(i);
            delete lpMyProvider;
        }                       
        m_arrProviders.RemoveAll();
        return FALSE;
    }

    LPTAPIPROVIDER lpMyProvider = (LPTAPIPROVIDER) m_arrProviders.GetAt(m_iProviderPos);
    if (lpMyProvider)
    {
        lpProvider->dwPermanentProviderID = lpMyProvider->dwPermanentProviderID;
        lpProvider->strProviderName = lpMyProvider->strProviderName;
        return TRUE;
    }               
    return FALSE;

}// CTapiConnection::GetNextProvider

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::GetTranslateCaps
//
// Return the translation capabilities from TAPI
//
LONG CTapiConnection::GetTranslateCaps (LPLINETRANSLATECAPS lpTranslateCaps, DWORD dwTapiVersion)
{                                    
    return lineGetTranslateCaps (GetLineAppHandle(), dwTapiVersion, lpTranslateCaps);

}// CTapiConnection::GetTranslateCaps

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::SetCurrentLocation
//
// This operation sets the location used as the context for address translation. 
//
LONG CTapiConnection::SetCurrentLocation (DWORD dwLocation)
{                                      
    return lineSetCurrentLocation (GetLineAppHandle(), dwLocation);

}// CTapiConnection::SetCurrentLocation

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::OnLineCreate
//
// A new line has been added to the system - add it to our array.
//
void CTapiConnection::OnLineCreate (DWORD dwDeviceID)
{                                    
    ASSERT (GetLineFromDeviceID(dwDeviceID) == NULL);
    CTapiLine* pLine = (CTapiLine*) (m_pLineClass->CreateObject());
    pLine->Init (this, dwDeviceID);

	// Add it to our array
	CSingleLock keyLine (&m_semLines, TRUE);
    m_arrLines.Add (pLine);
	m_dwNumLines++;

	// Tell the line.
	pLine->OnDynamicCreate();

}// CTapiConnection::OnLineCreate

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::OnLineRemove
//
// A line has been removed to the system - mark it unavailable.
//
void CTapiConnection::OnLineRemove (DWORD dwDeviceID)
{                                    
    CTapiLine* pLine = GetLineFromDeviceID(dwDeviceID);
    ASSERT (pLine != NULL);

	// Mark it unavailable.
	pLine->m_iFlags |= CTapiLine::Removed;
	pLine->OnDynamicRemove();
	m_dwNumLines--;

}// CTapiConnection::OnLineRemove

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::OnLineClose
//
// A line has been removed to the system - mark it unavailable.
//
void CTapiConnection::OnLineClose (DWORD dwDeviceID)
{                                    
    CTapiLine* pLine = GetLineFromDeviceID(dwDeviceID);
    ASSERT (pLine != NULL);

	// Mark it unavailable.
	pLine->OnForceClose();

}// CTapiConnection::OnLineClose

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::OnPhoneCreate
//
// A new line has been added to the system - add it to our array.
//
void CTapiConnection::OnPhoneCreate (DWORD dwDeviceID)
{                                    
    ASSERT (GetPhoneFromDeviceID(dwDeviceID) == NULL);
    CTapiPhone* pPhone = (CTapiPhone*) (m_pPhoneClass->CreateObject());
    pPhone->Init (this, dwDeviceID);

	// Add it to our array
	CSingleLock keyPhone (&m_semPhones, TRUE);
    m_arrPhones.Add (pPhone);
	m_dwNumPhones++;

	// Tell the phone.
	pPhone->OnDynamicCreate();

}// CTapiConnection::OnPhoneCreate

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::OnPhoneRemove
//
// A line has been removed to the system - mark it unavailable.
//
void CTapiConnection::OnPhoneRemove (DWORD dwDeviceID)
{                                    
    CTapiPhone* pPhone = GetPhoneFromDeviceID(dwDeviceID);
    ASSERT (pPhone != NULL);

	// Mark it unavailable.
	pPhone->m_iFlags |= CTapiPhone::Removed;
	pPhone->OnDynamicRemove();
	m_dwNumPhones--;

}// CTapiConnection::OnPhoneRemove

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::OnPhoneClose
//
// A line has been removed to the system - mark it unavailable.
//
void CTapiConnection::OnPhoneClose (DWORD dwDeviceID)
{                                    
    CTapiPhone* pPhone = GetPhoneFromDeviceID(dwDeviceID);
    ASSERT (pPhone != NULL);

	// Mark it unavailable.
	pPhone->OnForceClose();

}// CTapiConnection::OnPhoneClose

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::OnRequestComplete
//
// Complete a tapi request
//
void CTapiConnection::OnRequestComplete (DWORD dwRequestID, LONG lResult)
{
	// Run through the lines and locate the request object.
	CTapiRequest* pRequest = LocateRequest(dwRequestID);

	// If we have no request, then we got a callback BEFORE the
	// original thread returned from the API which caused an
	// asynch request.  Create a request block and insert it
	// into our list to match this request.
	if (pRequest == NULL)
		pRequest = AddRequest(dwRequestID);

	// Complete the request.
	ASSERT (pRequest->m_dwRequestID == dwRequestID);
	pRequest->OnRequestComplete(lResult);

}// CTapiConnection::OnRequestComplete

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::AddRequest
//
// Add a request block to our waiting list.
//
CTapiRequest* CTapiConnection::AddRequest (DWORD dwRequestID)
{       
	CTapiRequest* pReq = NULL;

	// Add a new request for this request id.
    if (!IsTapiError(dwRequestID) && dwRequestID > 0)
    {   
		CSingleLock Lock (&m_semRequest, TRUE);

		// Always check to see if the request already exists.
		// If so, then it has already been completed so ignore
		// this request to add it again.
		pReq = LocateRequest(dwRequestID);
		if (pReq == NULL)
		{
			pReq = new CTapiRequest (dwRequestID);
			m_arrWaitingRequests.AddTail (pReq);
		}
		// Or if the request exists, verify that it isn't some old
		// request we hadn't cleared yet (i.e. handle re-use by TAPI).
		else if (pReq->m_dwCompleteTime > 0)
		{
			if ((pReq->m_dwCompleteTime + 10000) < GetTickCount())
			{
				pReq->m_evtComplete.ResetEvent();
				pReq->m_dwCompleteTime = 0;
				pReq->m_lResult = -1L;
			}
		}
	}

	return pReq;

}// CTapiConnection::AddRequest

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::LocateRequest
//
// Determine if the specified request belongs to this tapi object
//
CTapiRequest* CTapiConnection::LocateRequest (DWORD dwRequestID)
{
	CSingleLock keyReq (&m_semRequest, TRUE);
	for (POSITION pos = m_arrWaitingRequests.GetHeadPosition(); pos != NULL;)
	{
		CTapiRequest* pReq = (CTapiRequest*) m_arrWaitingRequests.GetNext(pos);
		if (pReq->m_dwRequestID == dwRequestID)
			return pReq;
	}
	return NULL;

}// CTapiConnection::LocateRequest

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::PurgeRequests
//
// Run through our request list and remove any completed requests which
// are older than the specified threshold.
//
void CTapiConnection::PurgeRequests (LONG lmSec)
{
	CSingleLock keyReq (&m_semRequest);
	if (keyReq.Lock() == TRUE)
	{
		for (POSITION pos = m_arrWaitingRequests.GetHeadPosition(); pos != NULL; )
		{   
			POSITION posCurr = pos;
			CTapiRequest* pReq = (CTapiRequest*) m_arrWaitingRequests.GetNext(pos);
			if (pReq->IsPending() == FALSE &&
				(pReq->m_dwCompleteTime + lmSec) < GetTickCount())
			{
				m_arrWaitingRequests.RemoveAt(posCurr);
				delete pReq;
			}
		}
	}

}// CTapiConnection::PurgeRequests

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::WaitForReply
//
// Wait for a TAPI request to complete
//
LONG CTapiConnection::WaitForReply(DWORD dwRequestID, LONG lTimeout/*=INFINITE*/)
{
	if (dwRequestID == 0 || IsTapiError(dwRequestID))
		return dwRequestID;

	CTapiRequest* pReq = LocateRequest(dwRequestID);
	if (pReq == NULL)
		return LINEERR_INVALPARAM;

	return pReq->GetResult(lTimeout);

}// CTapiConnection::WaitForRequest

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::LineEventProc
//
// This function is managed by a thread which spins waiting for events from
// TAPI.
//
void CTapiConnection::LineEventProc()
{
	LINEMESSAGE lm;
	LONG lResult;

	// Spin forever waiting on TAPI to shutdown
	do
	{
		// Get the TAPI message
		lResult = lineGetMessage(m_hLineApp, &lm, 5000);
		if (lResult == 0)
		{
			// If this is an asynch request completing, then mark it in our list
			// and then pass it down to the line.
			if (lm.dwMessageID == LINE_REPLY)
				OnRequestComplete (lm.dwParam1, lm.dwParam2);
              
			// If this is a LINE_CREATE message, then a new line has been dynamically
			// added to TAPI (Plug&Play).  Manage it.
			else if (lm.dwMessageID == LINE_CREATE)
				OnLineCreate (lm.dwParam1);

			// If this is a LINE_REMOVE message, then a line has been removed from the system.
			else if (lm.dwMessageID == LINE_REMOVE)
				OnLineRemove (lm.dwParam1);

			else if (lm.dwMessageID == LINE_CLOSE)
				OnLineClose (lm.dwParam1);

			else // Line or call message.
			{
				CTapiLine* pLine = (CTapiLine*) lm.dwCallbackInstance;
				if (pLine != NULL)
					pLine->LineCallback (lm.hDevice, lm.dwMessageID, lm.dwParam1, lm.dwParam2, lm.dwParam3);
			}
		}

		// Kill any completed requests still in queue.
		PurgeRequests(5000);

	} while (lResult != LINEERR_INVALAPPHANDLE);

	m_pMonitorThread_L = NULL;

}// CTapiConnection::LineEventProc

////////////////////////////////////////////////////////////////////////////////////
// CTapiConnection::PhoneEventProc
//
// This function is managed by a thread which spins waiting for events from
// TAPI.
//
void CTapiConnection::PhoneEventProc()
{
	PHONEMESSAGE pm;
	LONG lResult;

	// Fetch messages until we have no more
	do
	{
		// Get the TAPI message
		lResult = phoneGetMessage(m_hPhoneApp, &pm, INFINITE);
		if (lResult == 0)
		{
			// If this is an asynch request completing, then mark it in our list
			// and then pass it down to the line.
			if (pm.dwMessageID == PHONE_REPLY)
				OnRequestComplete (pm.dwParam1, pm.dwParam2);
              
			// If this is a PHONE_CREATE message, then a new phone has been dynamically
			// added to TAPI (Plug&Play).  Manage it.
			else if (pm.dwMessageID == PHONE_CREATE)
				OnPhoneCreate (pm.dwParam1);

			// If this is a PHONE_REMOVE message, then a phone has been removed from the system.
			else if (pm.dwMessageID == PHONE_REMOVE)
				OnPhoneRemove (pm.dwParam1);

			else if (pm.dwMessageID == PHONE_CLOSE)
				OnPhoneClose (pm.dwParam1);

			else // Other phone related message
			{
				CTapiPhone* pPhone = (CTapiPhone*) pm.dwCallbackInstance;
				if (pPhone != NULL)
					pPhone->PhoneCallback (pm.hDevice, pm.dwMessageID, pm.dwParam1, pm.dwParam2, pm.dwParam3);
			}
		}
	}
	while (lResult != PHONEERR_INVALAPPHANDLE);

	m_pMonitorThread_P = NULL;

}// CTapiConnection::PhoneEventProc
