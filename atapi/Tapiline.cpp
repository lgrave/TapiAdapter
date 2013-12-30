//
//	Last Modified: $Date: 2010-07-20 09:48:12 $
//
//	$Log: Tapiline.cpp,v $
//	Revision 1.2  2010-07-20 09:48:12  lgrave
//	corrected windows crlf to unix lf
//

//	Revision 1.1  2010-07-19 23:40:41  lgrave

//	1st version added to cvs

//
//

// TAPILINE.CPP
//
// This file contains the line-level functions for the 
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
#include "tapistr.h"

IMPLEMENT_DYNCREATE (CTapiLine, CTapiObject)

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::CTapiLine
//
// Constructor for the line object
//
CTapiLine::CTapiLine()
{                       
    m_pConn = NULL;
	m_iFlags = 0;
    m_hLine = NULL;
    m_dwDeviceID = 0xffffffff;
    m_dwAPIVersion = 0L;
    m_lpLineCaps = NULL;
    m_lpLineStatus = NULL;

}// CTapiLine::CTapiLine

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::~CTapiLine
//
// Destructor for the line object
//
CTapiLine::~CTapiLine()
{   
	// Close the line if necessary
	Close();

    // Delete all our address appearances.
	CSingleLock Lock(&m_semAddress, TRUE);
    for (int i = 0; i < (int) GetAddressCount(); i++)
    {
        CTapiAddress* pAddr = (CTapiAddress*) m_arrAddress.GetAt(i);
        delete pAddr;
    }
    m_arrAddress.RemoveAll();

	// Force any existing call appearances to disappear as well.
	CSingleLock Lock2 (&m_semCalls, TRUE);
    while (m_arrCalls.GetSize() > 0)
    {
		CTapiCall* pCall = (CTapiCall*) m_arrCalls[0];
		m_arrCalls.RemoveAt(0);
		delete pCall;
	}

	// Delete our two informational blocks.
	if (m_lpLineCaps)
		FreeMem (m_lpLineCaps);
	if (m_lpLineStatus)
		FreeMem (m_lpLineStatus);

}// CTapiLine::~CTapiLine

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::Init
//
// Initialize the line object
//
void CTapiLine::Init (CTapiConnection* pConn, DWORD dwDeviceID)
{                  
    m_pConn = pConn;
    
    // Negotiate the API version with TAPI.
    LINEEXTENSIONID lineExtID;
    if (lineNegotiateAPIVersion (pConn->GetLineAppHandle(),
              dwDeviceID, TAPIVER_13, TAPIVER_31,
              &m_dwAPIVersion, &lineExtID) == 0)
    {
		// Save off the device ID.
        m_dwDeviceID = dwDeviceID;
        
        // Gather all the address information for each address on the line
        GatherAddressInformation();
    }                                

}// CTapiLine::Init

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GatherAddressInformation
//
// Build all the address information objects for this line.
//
void CTapiLine::GatherAddressInformation()
{
	if (GetLineCaps() != NULL)
    {
		CSingleLock Lock (&m_semAddress, TRUE);
        for (int i = 0; i < (int) GetAddressCount(); i++)
        {
            CTapiAddress* pAddr = (CTapiAddress*) m_pConn->m_pAddrClass->CreateObject();
            pAddr->Init (this, (DWORD)i);
            m_arrAddress.Add(pAddr);
        }
    }    

}// CTapiLine::GatherAddressInformation

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetAddressCount
//
// Return the count of addresses on this line
//
DWORD CTapiLine::GetAddressCount() const
{                             
	LPLINEDEVCAPS lpCaps = ((CTapiLine*)this)->GetLineCaps();
	if (lpCaps)
        return lpCaps->dwNumAddresses;
    return 0;

}// CTapiLine::GetAddressCount

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetAddress
//
// Return the address structure for an address id.
//
CTapiAddress* CTapiLine::GetAddress(DWORD dwAddr)
{   
	CSingleLock Lock (&m_semAddress, TRUE);
    if (dwAddr < (DWORD) m_arrAddress.GetSize())
        return (CTapiAddress*) m_arrAddress.GetAt((int)dwAddr);
    return NULL;
    
}// CTapiLine::GetAddress

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetAddress
//
// Return the address structure for an address id.
//
CTapiAddress* CTapiLine::GetAddress(LPCTSTR lpszAddr, DWORD dwSize, DWORD dwMode)
{
    if (IsOpen())
    {
        DWORD dwAddrId = 0xffffffff;
        if (lineGetAddressID (GetLineHandle(), &dwAddrId, dwMode, lpszAddr, dwSize) == 0)
            return GetAddress(dwAddrId);
    }
    return NULL;

}// CTapiLine::GetAddress

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetNegotiatedAPIVersion
//
// Return the negotiated API version for TAPI
//
DWORD CTapiLine::GetNegotiatedAPIVersion() const
{                                     
    return m_dwAPIVersion;

}// CTapiLine::GetNegotiatedAPIVersion

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetTapiConnection
//
// Return the owner TAPI connection object
//
CTapiConnection* CTapiLine::GetTapiConnection() const
{                               
    return m_pConn;

}// CTapiLine::GetTapiConnection

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetDeviceID
//
// Return the device ID for the line object
// 
DWORD CTapiLine::GetDeviceID() const
{                         
    return m_dwDeviceID;

}// CTapiLine::GetDeviceID

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::IsOpen
//
// Return whether this line is open for use.
//
BOOL CTapiLine::IsOpen() const
{                    
    return (m_hLine != NULL);

}// CTapiLine::IsOpen

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetLineHandle
//
// Return the line handle for this line
//
HLINE CTapiLine::GetLineHandle() const
{                           
    return m_hLine;

}// CTapiLine::GetLineHandle

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetProviderInfo
//
// Return the name of the service provider for this device
//
CString CTapiLine::GetProviderInfo() const
{                             
    CString strProviderName = TAPISTR_NOPROVIDERINFO;

    if (m_lpLineCaps)
    {    
        if (m_lpLineCaps->dwProviderInfoSize &&
            m_lpLineCaps->dwProviderInfoOffset)
//            m_lpLineCaps->dwStringFormat == STRINGFORMAT_ASCII)
        {
            LPCTSTR lpszProvider = ((LPCTSTR)m_lpLineCaps)+m_lpLineCaps->dwProviderInfoOffset;
            strProviderName = lpszProvider;
        }
    }

    return strProviderName;

}// CTapiLine::GetProviderInfo

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetSwitchInfo
//                            
// Return the information about the switch
//
CString CTapiLine::GetSwitchInfo() const
{
    CString strSwitchInfo = TAPISTR_SWITCHUNKNOWN;
    
    if (m_lpLineCaps)
    {
        if (m_lpLineCaps->dwSwitchInfoSize &&
            m_lpLineCaps->dwSwitchInfoOffset)
//            m_lpLineCaps->dwStringFormat == STRINGFORMAT_ASCII)
        {
            LPCTSTR lpszSwitch = ((LPCTSTR)m_lpLineCaps)+m_lpLineCaps->dwSwitchInfoOffset;
            strSwitchInfo = lpszSwitch;
        }
    }
    
    return strSwitchInfo;

}// CTapiLine::GetSwitchInfo

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetLineName
//
// Return the line name for this line device
//
CString CTapiLine::GetLineName() const
{                         
    CString strLineName;
    LPLINEDEVCAPS lpLineCaps = ((CTapiLine*)this)->GetLineCaps();
    if (lpLineCaps)
    {
        if (lpLineCaps->dwLineNameSize &&
            lpLineCaps->dwLineNameOffset)
//            (lpLineCaps->dwStringFormat == STRINGFORMAT_ASCII ||
//			 lpLineCaps->dwStringFormat == STRINGFORMAT_UNICODE))
        {
            LPCTSTR lpszLineName = ((LPCTSTR)lpLineCaps)+lpLineCaps->dwLineNameOffset;
            strLineName = lpszLineName;
        }
        else
            strLineName = TAPISTR_NOLINENAME;
    }
    else
        strLineName = TAPISTR_NOLINE;            

    return strLineName;
    
}// CTapiLine::GetLineName

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetLineCaps
//
// Return the line capabilities we loaded on entry - don't allow modification
// of the structure.
//
const LPLINEDEVCAPS CTapiLine::GetLineCaps(DWORD dwAPIVersion, DWORD dwExtVersion, BOOL fForceRealloc)
{   
	// If there is no version information, then use our negotiated version.
	if (dwAPIVersion == 0)
		dwAPIVersion = GetNegotiatedAPIVersion();

    if (m_lpLineCaps != NULL && !fForceRealloc)
		return m_lpLineCaps;

    // Allocate a buffer for the line capabilities
    DWORD dwSize = (m_lpLineCaps) ? m_lpLineCaps->dwTotalSize : sizeof(LINEDEVCAPS)+1024;
    while (TRUE)
    {
		if (m_lpLineCaps == NULL)
		{
			m_lpLineCaps = (LPLINEDEVCAPS) AllocMem ( dwSize);
			if (m_lpLineCaps == NULL)
				return NULL;
		}
        
        // Mark the size we are sending.
        ((LPVARSTRING)m_lpLineCaps)->dwTotalSize = dwSize;
        if (lineGetDevCaps (m_pConn->GetLineAppHandle(), GetDeviceID(), 
						    dwAPIVersion, dwExtVersion, m_lpLineCaps) != 0)
			return NULL;
        
        // If we didn't get it all, then reallocate the buffer and retry it.
        if (m_lpLineCaps->dwNeededSize <= dwSize)
		{
			memset((LPBYTE)m_lpLineCaps + m_lpLineCaps->dwUsedSize, 0, dwSize - m_lpLineCaps->dwUsedSize);
			return m_lpLineCaps;
		}

        dwSize = m_lpLineCaps->dwNeededSize;
		FreeMem (m_lpLineCaps);
        m_lpLineCaps = NULL;
    }    

}// CTapiLine::GetLineCaps

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::Open
//
// Open the line device
//
LONG CTapiLine::Open(DWORD dwPrivilege, DWORD dwMediaMode, 
					 DWORD dwAPIVersion, DWORD dwExtVersion,
					 LPLINECALLPARAMS const lpCallParams)
{   
    if (IsOpen())
        return FALSE;

	// If no version information given, use the highest negotiated during
	// our INIT process.
	if (dwAPIVersion == 0)
		dwAPIVersion = GetNegotiatedAPIVersion();

    LONG lResult = lineOpen (m_pConn->GetLineAppHandle(), GetDeviceID(),
                             &m_hLine, dwAPIVersion, dwExtVersion,
                             (DWORD)this, dwPrivilege, dwMediaMode, lpCallParams);
	if (lResult != 0)
		m_hLine = NULL;
	return lResult;

}// CTapiLine::Open                     

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::Close
//
// Close the line (and all calls)
//
LONG CTapiLine::Close()
{                  
    if (!IsOpen())
        return FALSE;

	LONG lResult = lineClose (GetLineHandle());
	if (lResult == 0)
		OnClose();

    return lResult;

}// CTapiLine::Close

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetCallFromHandle
//
// Given the HCALL handle to a call appearance on this line, return the
// C++ object representing that call if available.
//
CTapiCall* CTapiLine::GetCallFromHandle (HCALL hCall)
{                               
    // Go through our call array and determine if the call exists.
	CSingleLock Lock (&m_semCalls, TRUE);
    for (int i = 0; i < m_arrCalls.GetSize(); i++)
    {
		CTapiCall* pTest = (CTapiCall*) m_arrCalls[i];
		if (pTest->GetCallHandle() == hCall)
			return pTest;
	}
	return NULL;

}// CTapiLine::GetCallFromHandle

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::FindCall
//
// Locate a call by state/features
//
CTapiCall* CTapiLine::FindCall(DWORD dwStates, DWORD dwFeatures)
{
    // Go through our call array and determine if the call exists.
	CSingleLock Lock (&m_semCalls, TRUE);
    for (int i = 0; i < m_arrCalls.GetSize(); i++)
    {
		CTapiCall* pCall = (CTapiCall*) m_arrCalls[i];
		if (pCall->GetCallState() & dwStates)
		{
			LPLINECALLSTATUS lpStatus = pCall->GetCallStatus();
			if (dwFeatures == 0 || (lpStatus && (lpStatus->dwCallFeatures & dwFeatures)))
				return pCall;
		}
	}
	return NULL;

}// CTapiLine::FindCall

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::LineCallback
//
// Callback from the CTapiConnection object about one of our settings or calls
// changing.
//
void CTapiLine::LineCallback (DWORD hDevice, DWORD dwMsg, DWORD dwParam1, 
                                DWORD dwParam2, DWORD dwParam3)
{                          
    switch (dwMsg)
    {
		case LINE_APPNEWCALL:
			CreateNewCall((HCALL)dwParam2);
			break;

        case LINE_ADDRESSSTATE:   
            if ((HLINE)hDevice == GetLineHandle())
				OnAddressStateChange (dwParam1, dwParam2);
            break;
        
		case LINE_AGENTSTATUS:
			OnAgentStateChange (dwParam1, dwParam2, dwParam3);
			break;

        case LINE_CALLINFO:
            OnCallInfoChange ((HCALL)hDevice, dwParam1);
            break;
        
        case LINE_CALLSTATE:
            OnCallStateChange ((HCALL)hDevice, dwParam1, dwParam2, dwParam3);
            break;

        case LINE_DEVSPECIFIC:
            OnDevSpecific (hDevice, dwParam1, dwParam2, dwParam3);
            break;
        
        case LINE_DEVSPECIFICFEATURE:
            OnDevSpecificFeature (hDevice, dwParam1, dwParam2, dwParam3);
            break;

        case LINE_GATHERDIGITS:
            OnGatherDigitsComplete ((HCALL)hDevice, dwParam1);
            break;

        case LINE_GENERATE:
            OnGenerateComplete ((HCALL)hDevice, dwParam1);
            break;

        case LINE_LINEDEVSTATE:  
            ASSERT ((HLINE)hDevice == GetLineHandle());
            OnDeviceStateChange (dwParam1, dwParam2, dwParam3);
            break;

        case LINE_MONITORDIGITS:
            OnDigitDetected ((HCALL)hDevice, dwParam1, dwParam2);
            break;

        case LINE_MONITORMEDIA:
            OnCallMediaModeChange ((HCALL)hDevice, dwParam1);
            break;

        case LINE_MONITORTONE:
            OnToneDetected ((HCALL)hDevice, dwParam1);
            break;

        default:
            TRACE (_T("CTapiLine: unknown TAPI callback %ld\r\n"), dwMsg);
            break;    
    }

}// CTapiLine::LineCallback

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnAddressStateChange
//
// This virtual function is called whenever the status of one of our addresses
// has changed.
//
void CTapiLine::OnAddressStateChange (DWORD dwAddressID, DWORD dwState)
{                                                                      
    CTapiAddress* pAddr = GetAddress((int)dwAddressID);
    if (pAddr != NULL)
        pAddr->OnStateChange (dwState);

}// CTapiLine::OnAddressStateChange

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnAgentStateChange
//
// This virtual function is called whenever the status of one of our agent addresses
// has changed.
//
void CTapiLine::OnAgentStateChange (DWORD dwAddressID, DWORD dwFields, DWORD dwState)
{                                                                      
    CTapiAddress* pAddr = GetAddress((int)dwAddressID);
    if (pAddr != NULL)
        pAddr->OnAgentStateChange (dwFields, dwState);

}// CTapiLine::OnAgentStateChange

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnCallInfoChange
//
// This virtual function is called whenever the information with one of our
// call appearances changes.  Its default implementation is to pass it down to
// the call appearance object.
//
void CTapiLine::OnCallInfoChange (HCALL hCall, DWORD dwCallInfoState)
{                              
    CTapiCall* pCall = GetCallFromHandle (hCall);
    if (pCall)
        pCall->OnInfoChange (dwCallInfoState);

}// CTapiLine::OnCallInfoChange

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnCallStateChange
//
// This virtual function is called whenever the state of one of our call
// appearances has changed.  Its default behavior is to pass it down to the
// call appearance object.
//
void CTapiLine::OnCallStateChange (HCALL hCall, DWORD dwState, 
                            DWORD dwStateDetail, DWORD dwPrivilage)
{                                            
	// Find the call - create a new call if one doesn't exist.
    CTapiCall* pCall = CreateNewCall (hCall);
	if (pCall != NULL)
		pCall->OnStateChange (dwState, dwStateDetail, dwPrivilage);

}// CTapiLine::OnCallStateChange

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::CreateNewCall
//
// This method is called to create a new call on this line.
//
CTapiCall* CTapiLine::CreateNewCall (HCALL hCall)
{
	if (hCall == NULL)
		return NULL;

	// Lock the array up front so no other thread can
	// come through here while we are searching/creating
	// the call.  Otherwise, timing conditions could cause
	// TWO objects to be created for the same call.
	CSingleLock Lock (&m_semCalls, TRUE);
	CTapiCall* pCall = GetCallFromHandle(hCall);
	if (pCall)
		return pCall;
	
	// Create a new call object for this call.
	pCall = (CTapiCall*) GetTapiConnection()->m_pCallClass->CreateObject();
	pCall->Init (this, hCall);

	m_arrCalls.Add(pCall);
	Lock.Unlock();
	
	OnNewCall(pCall);

	return pCall;

}// CTapiLine::CreateNewCall

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::RemoveCall
//
// Remove the call appearance from the call list.
//
void CTapiLine::RemoveCall(CTapiCall* pCall)
{
	// Don't allow non-IDLE calls to be removed.
	if (pCall->GetCallState() != LINECALLSTATE_IDLE)
		return;

	CSingleLock Lock (&m_semCalls, TRUE);
	for (int i = 0; i < m_arrCalls.GetSize(); i++)
	{
		CTapiCall* pTest = (CTapiCall*) m_arrCalls[i];
		if (pCall == pTest)
		{
			m_arrCalls.RemoveAt(i);
			delete pCall;
		}
	}

}// CTapiLine::RemoveCall

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnNewCall
//
// This method gets called whenever a new call object is created on this
// line.  It is invoked from the address object
//
void CTapiLine::OnNewCall (CTapiCall* /*pCall*/)
{
	/* Do nothing */

}// CTapiLine::OnNewCall

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnClose
//
// Close the line
//
void CTapiLine::OnClose()
{                     
    m_hLine = NULL;
	
	// Go through the addresses on this line and remove all the call
	// appearance objects.
	CSingleLock Lock (&m_semCalls, TRUE);
    while (m_arrCalls.GetSize() > 0)
    {
		CTapiCall* pCall = (CTapiCall*) m_arrCalls[0];
		m_arrCalls.RemoveAt(0);
		pCall->Deallocate();
		delete pCall;
	}
	
}// CTapiLine::OnClose

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnDevSpecific
//
// This function is called whenever we receive a device-specific message from
// a service provider.  The default implementation does nothing.
//
void CTapiLine::OnDevSpecific (DWORD /*dwHandle*/, DWORD /*dwParam1*/, 
                               DWORD /*dwParam2*/, DWORD /*dwParam3*/)
{                           
    /* Do nothing */
    
}// CTapiLine::OnDevSpecific

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnDevSpecificFeature
//
// This function is called whenever we receive a device-specific message from
// a service provider.  The default implementation does nothing.
//
void CTapiLine::OnDevSpecificFeature (DWORD /*dwHandle*/, DWORD /*dwParam1*/, 
                               DWORD /*dwParam2*/, DWORD /*dwParam3*/)
{                           
    /* Do nothing */
    
}// CTapiLine::OnDevSpecificFeature

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnGatherDigitsComplete
//
// The call in question has completed gathering the digits.  Pass the
// result onto the appropriate call appearance.
//
void CTapiLine::OnGatherDigitsComplete (HCALL hCall, DWORD dwReason)
{
    CTapiCall* pCall = GetCallFromHandle (hCall);
    if (pCall)
        pCall->OnGatherDigitsComplete (dwReason);
        
}// CTapiLine::OnGatherDigitsComplete

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnGenerateComplete
//
// Tone generation on the specified call appearance has completed.  Pass
// the result onto the appropropriate call appearance.
//
void CTapiLine::OnGenerateComplete (HCALL hCall, DWORD dwReason)
{                                
    CTapiCall* pCall = GetCallFromHandle (hCall);
    if (pCall)
        pCall->OnGenerateComplete (dwReason);

}// CTapiLine::OnGenerateComplete

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnDeviceStateChange
//
// Our line device has changed status for some reason.  Default implementation
// is to do nothing.
//
void CTapiLine::OnDeviceStateChange (DWORD dwDeviceState, DWORD /*dwStateDetail1*/, DWORD /*dwStateDetail2*/)
{
	if (dwDeviceState & LINEDEVSTATE_CAPSCHANGE)
	{
		if (m_lpLineCaps)
			GetLineCaps(0, 0, TRUE);
		dwDeviceState &= ~LINEDEVSTATE_CAPSCHANGE;
	}

}// CTapiLine::OnDeviceStateChange   

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnDigitDetected
//
// Override called whenever a digit is detected on a particular call
// appearance.
//
void CTapiLine::OnDigitDetected (HCALL hCall, DWORD dwDigit, DWORD dwDigitMode)
{                             
    CTapiCall* pCall = GetCallFromHandle (hCall);
    if (pCall)
        pCall->OnDigitDetected (dwDigit, dwDigitMode);    

}// CTapiLine::OnDigitDetected

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnCallMediaModeChange
//
// The media mode of the specified call appearance has changed.  Pass it
// down to the appropriate call appearance.
//
void CTapiLine::OnCallMediaModeChange (HCALL hCall, DWORD dwMediaMode)
{                                   
    CTapiCall* pCall = GetCallFromHandle (hCall);
    if (pCall)
        pCall->OnMediaModeChange (dwMediaMode);

}// CTapiLine::OnCallMediaModeChange

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnToneDetected
//
// A DTMF tone was detected.  Pass it to the call appearance object.
//
void CTapiLine::OnToneDetected (HCALL hCall, DWORD dwAppSpecific)
{                            
    CTapiCall* pCall = GetCallFromHandle (hCall);
    if (pCall)
        pCall->OnToneDetected (dwAppSpecific);

}// CTapiLine::OnToneDetected

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::MakeCall
//
// Place a call onto a call appearance on this line.
//
LONG CTapiLine::MakeCall (CTapiCall** pCall, 
                          LPCTSTR lpszDestAddr, DWORD dwCountry, 
                          LPLINECALLPARAMS const lpCallParams)
{                      
    HCALL hCall;
    LONG lResult = GetTAPIConnection()->WaitForReply(
			ManageAsynchRequest(lineMakeCall (GetLineHandle(), &hCall, lpszDestAddr, 
                                  dwCountry, lpCallParams)));
    if (!lResult)
		*pCall = CreateNewCall(hCall);
    return lResult;

}// CTapiLine::MakeCall                                    

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetID
//
// Return the specified ID for the address on the line device.
//
LONG CTapiLine::GetID (DWORD dwAddressID, LPVARSTRING lpDeviceID, LPCTSTR lpszDeviceClass)
{
    return lineGetID (GetLineHandle(), dwAddressID, NULL, LINECALLSELECT_ADDRESS, lpDeviceID, lpszDeviceClass);

}// CTapiLine::GetID

/////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetID
//
// Return the specified ID for the the line device.
//
LONG CTapiLine::GetID (LPVARSTRING lpDeviceID, LPCTSTR lpszDeviceClass)
{                      
    return lineGetID (GetLineHandle(), 0L, NULL, LINECALLSELECT_LINE, lpDeviceID, lpszDeviceClass);

}// CTapiLine::GetID

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::Config
//
// Manage the configuration for this line device
//
LONG CTapiLine::Config (CWnd* pwndOwner, LPCTSTR lpszDeviceClass)
{   
    return lineConfigDialog (m_dwDeviceID, (pwndOwner) ? pwndOwner->GetSafeHwnd() : NULL, 
							 lpszDeviceClass);

}// CTapiLine::Config

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::ConfigEdit
//
// Manage the configuration for this line device
//
LONG CTapiLine::ConfigEdit (CWnd* pwndOwner, LPCTSTR lpszDeviceClass, 
                            LPVOID const lpDeviceConfigIn, DWORD dwSize, 
                            LPVARSTRING lpDeviceConfigOut)
{
    return ::lineConfigDialogEdit (m_dwDeviceID, (pwndOwner) ? pwndOwner->GetSafeHwnd() : NULL, 
						lpszDeviceClass, lpDeviceConfigIn, dwSize, lpDeviceConfigOut);

}// CTapiLine::ConfigEdit
                     
////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetIcon
//
// Return the icon for this line device.
//
HICON CTapiLine::GetIcon(LPCTSTR lpszDeviceClass)
{                     
    HICON hIcon = NULL;
    if (::lineGetIcon (m_dwDeviceID, lpszDeviceClass, &hIcon) == 0)
		return hIcon;
	return NULL;

}// CTapiLine::GetIcon

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetLineStatus
//
// Return the status of this line device
//
const LPLINEDEVSTATUS CTapiLine::GetLineStatus()
{   
    // Allocate a buffer for the line information
    DWORD dwSize = (m_lpLineStatus) ? m_lpLineStatus->dwTotalSize : sizeof (LINEDEVSTATUS) + 1024;
    while (TRUE)
    {   
		if (m_lpLineStatus == NULL)
		{
			m_lpLineStatus = (LPLINEDEVSTATUS) AllocMem( dwSize);
			if (m_lpLineStatus == NULL)
				return NULL;
		}
        
        // Mark the size we are sending.
        ((LPVARSTRING)m_lpLineStatus)->dwTotalSize = dwSize;
        if (lineGetLineDevStatus (GetLineHandle(), m_lpLineStatus) != 0)
            return NULL;
        
        if (m_lpLineStatus->dwNeededSize <= dwSize)
            return m_lpLineStatus;

        // If we didn't get it all, then reallocate the buffer and retry it.
        dwSize = m_lpLineStatus->dwNeededSize;
		FreeMem (m_lpLineStatus);
        m_lpLineStatus = NULL;
    }    

}// CTapiLine::GetLineStatus

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::SwapHold
//
// Swap two call appearances on hold
//
LONG CTapiLine::SwapHold (CTapiCall* pCall, CTapiCall* pCall2)
{                     
	CTapiCall* pActive, *pHeld;

	ASSERT (pCall && pCall2);
	if ((pCall->GetCallState() & (LINECALLSTATE_ONHOLD | 
		LINECALLSTATE_ONHOLDPENDTRANSFER |
		LINECALLSTATE_ONHOLDPENDCONF)) != 0)
	{
		pHeld = pCall;
		pActive = pCall2;
	}
	else
	{
		pHeld = pCall2;
		pActive = pCall;
	}

    return ManageAsynchRequest(lineSwapHold (pActive->GetCallHandle(), pHeld->GetCallHandle()));

}// CTapiLine::SwapHold

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::SetStatusMessages
//
// Set the status notifications
//
LONG CTapiLine::SetStatusMessages (DWORD dwLineStates, DWORD dwAddressStates)
{                                                    
    if (!IsOpen())
        return LINEERR_INVALLINEHANDLE;
    return lineSetStatusMessages (GetLineHandle(), dwLineStates, dwAddressStates);

}// CTapiLine::SetStatusMessages

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetStatusMessages
//
// Get the status notifications
//
LONG CTapiLine::GetStatusMessages (LPDWORD lpdwLineStatus, LPDWORD lpdwAddressStates)
{
    if (!IsOpen())
        return LINEERR_INVALLINEHANDLE;
    return lineGetStatusMessages (GetLineHandle(), lpdwLineStatus, lpdwAddressStates);

}// CTapiLine::GetStatusMessages
              
////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::DevSpecific
//
// Enables service providers to provide access to features not offered by 
// other TAPI functions. The meaning of these extensions are device specific, 
// and taking advantage of these extensions requires the application to be 
// fully aware of them.
//
LONG CTapiLine::DevSpecific (DWORD dwFeature, LPVOID lpParams, DWORD dwSize)
{                         
    return ManageAsynchRequest(lineDevSpecificFeature (GetLineHandle(), dwFeature, lpParams, dwSize));

}// CTapiLine::DevSpecific

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::Forward
//
// Forwards calls destined for the specified address on the specified line, 
// according to the specified forwarding instructions. When an originating 
// address (dwAddressID) is forwarded, the specified incoming calls for 
// that address are deflected to the other number by the switch.
// This function provides a combination of forward and do-not-disturb features. 
//
// dwAddress = -1L for all addresses forwarded, otherwise 0-numAddress-1.
//  
LONG CTapiLine::Forward (DWORD dwAddress, LPLINEFORWARDLIST const lpForwardList, 
                         DWORD dwNumRingsNoAnswer, CTapiCall** pConsCall, 
                         LPLINECALLPARAMS const lpCallParams)
{               
    HCALL hCall = NULL;
    *pConsCall = NULL;
          
    LONG lResult = ManageAsynchRequest(
		lineForward (GetLineHandle(), (dwAddress == (DWORD)-1L) ? TRUE : FALSE,
                              (dwAddress == (DWORD)-1L) ? 0 : dwAddress,
                              lpForwardList, dwNumRingsNoAnswer, &hCall, 
                              lpCallParams));
    if (!GetTAPIConnection()->WaitForReply(lResult))
        *pConsCall = GetCallFromHandle(hCall);
    return lResult;

}// CTapiLine::Forward
    
////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::CancelForward
//
// Cancels any forwarding being done on the specified address (-1L = all).
//
LONG CTapiLine::CancelForward (DWORD dwAddress)
{                           
    return ManageAsynchRequest(
		lineForward (GetLineHandle(), (dwAddress == (DWORD)-1L) ? TRUE : FALSE,
                              (dwAddress == (DWORD)-1L) ? 0 : dwAddress,
                              NULL, 0, NULL, NULL));
}// CTapiLine::CancelForward

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::SetDevConfig
//
// Allows the application to restore the configuration of a media stream 
// device on a line device to a setup previously obtained using lineGetDevConfig.
// For example, the contents of this structure could specify data rate, 
// character format, modulation schemes, and error control protocol 
// settings for a "datamodem" media device associated with the line. 
//
LONG CTapiLine::SetDevConfig (LPVOID const lpBuff, DWORD dwSize, LPCTSTR lpszDeviceClass)
{                                                          
    return ManageAsynchRequest(lineSetDevConfig (GetDeviceID(), lpBuff, dwSize, lpszDeviceClass));

}// CTapiLine::SetDevConfig

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::SetDeviceStatus
//
// Allows the application to change the status of the line device.
//
LONG CTapiLine::SetDeviceStatus (DWORD dwDevState, BOOL fSet)
{                                                          
    return ManageAsynchRequest(lineSetLineDevStatus (GetLineHandle(), dwDevState, (fSet) ? -1L : 0));

}// CTapiLine::SetDeviceStatus

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetDevConfig
//
// Returns an "opaque" data structure object the contents of which are 
// specific to the line (service provider) and device class. 
// The data structure object stores the current configuration of a 
// media-stream device associated with the line device.
//
LONG CTapiLine::GetDevConfig (LPVOID lpBuff, DWORD dwSize, LPCTSTR lpszDeviceClass)
{                          
    LPVARSTRING lpVarString = (LPVARSTRING) AllocMem( sizeof(VARSTRING)+dwSize);
    if (lpVarString == NULL)
        return LINEERR_NOMEM;
        
    lpVarString->dwStringFormat = STRINGFORMAT_BINARY;
    lpVarString->dwTotalSize = sizeof(VARSTRING)+dwSize;
    lpVarString->dwStringSize = 0L;
    memset (lpBuff, 0, (size_t)dwSize);
    
    LONG lResult = lineGetDevConfig (GetDeviceID(), lpVarString, lpszDeviceClass);
    if (lResult == 0)
    {
        memcpy (lpBuff, (LPSTR)lpVarString+lpVarString->dwStringOffset, 
                (size_t)lpVarString->dwStringSize);
    }

    FreeMem(lpVarString);
    return lResult;    

}// CTapiLine::GetDevConfig

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetNewCalls
//
// This function returns all the active call handles for this line in an 
// object list.
//
LONG CTapiLine::GetNewCalls (CObList& lstCalls)
{                         
    DWORD dwSize = sizeof (LINECALLLIST) + 1024;
    LPLINECALLLIST lpCallList = NULL;
    
    while (TRUE)
    {
		lpCallList = (LPLINECALLLIST) AllocMem ( dwSize);
        if (lpCallList == NULL)
			return LINEERR_NOMEM;

		lpCallList->dwTotalSize = dwSize;
        LONG lResult = lineGetNewCalls (GetLineHandle(), 0, LINECALLSELECT_LINE, lpCallList);
		if (lResult != 0)
        {
			FreeMem (lpCallList);
            return lResult;
        }
        
        // If we didn't get them all, then reallocate and try again.
        if (lpCallList->dwNeededSize <= dwSize)
			break;

        dwSize = lpCallList->dwNeededSize;
		FreeMem (lpCallList);
        lpCallList = NULL;
    }
    
    // Now go through the call list and create call handles for each.
    LPHCALL lphCall = (LPHCALL) ((LPSTR)lpCallList + lpCallList->dwCallsOffset);
    for (DWORD i = 0; i < lpCallList->dwCallsNumEntries; i++)
    {
        CTapiCall* pCall = CreateNewCall (*lphCall);
        if (pCall)
            lstCalls.AddTail(pCall);
        lphCall++;
    }        

	FreeMem (lpCallList);
    return 0;
    
}// CTapiLine::GetNewCalls

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::SetMediaControl
//
// Enables and disables control actions on the media stream associated 
// with the specified line. Media control actions can be triggered 
// by the detection of specified digits, media modes, custom tones, 
// and call states.
//
LONG CTapiLine::SetMediaControl (LPLINEMEDIACONTROLDIGIT const lpDigitList, 
                                 DWORD dwDigitNumEntries, 
                                 LPLINEMEDIACONTROLMEDIA const lpMediaList, 
                                 DWORD dwMediaNumEntries, 
                                 LPLINEMEDIACONTROLTONE const lpToneList, 
                                 DWORD dwToneNumEntries, 
                                 LPLINEMEDIACONTROLCALLSTATE const lpCallStateList,
                                 DWORD dwCallStateNumEntries)
{                             
    return lineSetMediaControl (GetLineHandle(), 0, NULL, LINECALLSELECT_LINE,
                               lpDigitList, dwDigitNumEntries,
                               lpMediaList, dwMediaNumEntries,
                               lpToneList, dwToneNumEntries,
                               lpCallStateList, dwCallStateNumEntries);
    
}// CTapiLine::SetMediaControl

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::SetTerminal
//
// Enables an application to specify which terminal information related to the 
// specified line is to be routed. lineSetTerminal can be used while calls 
// are in progress on the line to allow an application to route these events 
// to different devices as required.
//
LONG CTapiLine::SetTerminal (DWORD dwTerminalMode, DWORD dwTerminalID, BOOL fEnable)
{                         
    return ManageAsynchRequest(lineSetTerminal (GetLineHandle(), 0, NULL, LINECALLSELECT_LINE,
                            dwTerminalMode, dwTerminalID, (DWORD)fEnable));

}// CTapiLine::SetTerminal

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::SetTollList
//
// Manipulates the toll list.
//
LONG CTapiLine::SetTollList (LPCTSTR lpszAddressIn, DWORD dwTollListOption)
{                            
    return lineSetTollList (m_pConn->GetLineAppHandle(), GetDeviceID(), 
                            lpszAddressIn, dwTollListOption);

}// CTapiLine::SetTollList

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::TranslateAddress
//
// This operation translates the specified address into another format. 
//
LONG CTapiLine::TranslateAddress (LPCTSTR lpszAddressIn, DWORD dwCard, 
                                  DWORD dwTranslateOptions, 
                                  LPLINETRANSLATEOUTPUT lpTranslateOutput)
{                              
    return lineTranslateAddress (m_pConn->GetLineAppHandle(), GetDeviceID(),
                                  GetNegotiatedAPIVersion(), lpszAddressIn,
                                  dwCard, dwTranslateOptions, lpTranslateOutput);

}// CTapiLine::TranslateAddress

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::TranslateDialog
//
// Displays an application-modal dialog which allows the user to change 
// the current location, adjust location and calling card parameters, and 
// see the effect on a phone number about to be dialed.
//
LONG CTapiLine::TranslateDialog (CWnd* pwndOwner, LPCTSTR lpszAddressIn)
{                                       
    return lineTranslateDialog (m_pConn->GetLineAppHandle(), GetDeviceID(),
                                GetNegotiatedAPIVersion(), (pwndOwner) ? pwndOwner->GetSafeHwnd() : NULL, 
								lpszAddressIn);

}// CTapiLine::TranslateDialog

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::UncompleteCall
//
// Cancels the specified call competion request on the specified line. 
//
LONG CTapiLine::UncompleteCall (DWORD dwCompletionID)
{                  
    if (!IsOpen())
        return LINEERR_INVALLINEHANDLE;
    return ManageAsynchRequest(lineUncompleteCall (GetLineHandle(), dwCompletionID));

}// CTapiLine::UncompleteCall
        
////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::SetupConference
//
// Setup a call appearance to manage a conference
//
LONG CTapiLine::SetupConference (CTapiCall** pConfCall, CTapiCall** pConstCall,
                                 DWORD dwNumParties, 
                                 LPLINECALLPARAMS const lpCallParams)
{                             
    HCALL hCall1 = NULL;
    HCALL hCall2 = NULL;
    *pConfCall = NULL;  
    *pConstCall = NULL;
    
    LONG lResult = ManageAsynchRequest(
		lineSetupConference (NULL, m_hLine, &hCall1, &hCall2, dwNumParties, lpCallParams));
    if (!GetTAPIConnection()->WaitForReply(lResult))
    {
        *pConfCall = GetCallFromHandle (hCall1);
        *pConstCall = GetCallFromHandle (hCall2);
    }
    return lResult;

}// CTapiLine::SetupConference

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetProviderID
//
// Return the provider information for the TSP which owns this line.
//
DWORD CTapiLine::GetProviderID() const
{
	DWORD dwProviderID = 0;
	LPVARSTRING lpVS = (LPVARSTRING) AllocMem(sizeof(VARSTRING) + sizeof(DWORD));
	lpVS->dwTotalSize = sizeof(VARSTRING) + sizeof(DWORD);
	if (((CTapiLine*)this)->GetID(lpVS, _T("tapi/providerid")) == 0 && 
		lpVS->dwStringSize == sizeof(DWORD) &&
		lpVS->dwStringFormat == STRINGFORMAT_BINARY)
		dwProviderID = *((LPDWORD)((LPBYTE)lpVS + lpVS->dwStringOffset));
	FreeMem (lpVS);

	return dwProviderID;

}// CTapiLine::GetProviderID
        
////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetTSPProvider
//
// Return the provider information for the TSP which owns this line.
//
BOOL CTapiLine::GetTSPProvider (LPTAPIPROVIDER pProvider) const
{   
	// First see if we can get the provider id from the line device.
	// We use the new "tapi/providerid" key.
	DWORD dwProviderID = GetProviderID();
	if (dwProviderID > 0)
	{
		if (m_pConn->GetFirstProvider (pProvider))
		{            
			do
			{
				if (pProvider->dwPermanentProviderID == dwProviderID)
            		return TRUE;
			}
			while (m_pConn->GetNextProvider(pProvider));
		}
	}
	
	pProvider->dwPermanentProviderID = 0;
	pProvider->strProviderName.Empty();
    return FALSE;
    
}// CTapiLine::GetProviderInfo

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetValidIDs
//
// Return a list of valid keys for lineGetID
//
void CTapiLine::GetValidIDs(CStringArray& arrKeys) const
{
	LPLINEDEVCAPS lpLineCaps = ((CTapiLine*)this)->GetLineCaps();
	if (lpLineCaps && lpLineCaps->dwDeviceClassesSize > 0 &&
		lpLineCaps->dwDeviceClassesSize < 4096)
	{
		LPCTSTR pszKey = (LPCTSTR)((LPBYTE)lpLineCaps) + lpLineCaps->dwDeviceClassesOffset;
		while (*pszKey)
		{
			arrKeys.Add(pszKey);
			pszKey += lstrlen(pszKey)+1;
		}
	}

}// CTapiLine::GetValidIDs

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetCallCount
//
// Return the count of known calls on this line.
//
int CTapiLine::GetCallCount()
{
	// Make sure we know about all the calls.
	CObList lstCalls;
	GetNewCalls (lstCalls);

	CSingleLock Lock (&m_semCalls, TRUE);
	return m_arrCalls.GetSize();

}// CTapiLine::GetCallCount

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetCall
//
// Return a call from our list.
//
CTapiCall* CTapiLine::GetCall(int iIndex)
{
	CSingleLock Lock (&m_semCalls, TRUE);
	if (iIndex >= 0 && iIndex < m_arrCalls.GetSize())
		return (CTapiCall*) m_arrCalls[iIndex];
	return NULL;

}// CTapiLine::GetCall

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::IsValid
//
// Return whether this line is valid
//
BOOL CTapiLine::IsValid() const
{
	return ((m_iFlags & Removed) == 0);

}// CTapiLine::IsValid

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnDynamicCreate
//
// Called when the line is dynamically created
//
void CTapiLine::OnDynamicCreate()
{
}// CTapiLine::OnDynamicCreate

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnDynamicRemove
//
// Called when the line is dynamically removed
//
void CTapiLine::OnDynamicRemove()
{
}// CTapiLine::OnDynamicRemove

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::OnForceClose
//
// Called when the line is forced closed by TAPI
//
void CTapiLine::OnForceClose()
{
	// Close the line
	OnClose();

}// CTapiLine::OnForceClose

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine::GetRelatedPhoneID
//
// Returns the related phone device id for this line.
//
DWORD CTapiLine::GetRelatedPhoneID()
{
    LPVARSTRING lpVarString = (LPVARSTRING) AllocMem( sizeof(VARSTRING)+1024);
    if (lpVarString == NULL)
        return NULL;
    lpVarString->dwTotalSize = sizeof(VARSTRING)+10;
    
    DWORD dwID = 0xffffffff;
    if (GetID (lpVarString, _T("tapi/phone")) == 0)
        dwID = *((LPDWORD)((LPBYTE)lpVarString+lpVarString->dwStringOffset));

	FreeMem(lpVarString);
    return dwID;
	
}// CTapiLine::GetRelatedPhoneID
