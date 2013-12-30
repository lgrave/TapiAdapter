//
//	Last Modified: $Date: 2010-07-20 09:48:12 $
//
//	$Log: Tapicall.cpp,v $
//	Revision 1.2  2010-07-20 09:48:12  lgrave
//	corrected windows crlf to unix lf
//

//	Revision 1.1  2010-07-19 23:40:41  lgrave

//	1st version added to cvs

//
//

// TAPICALL.CPP
//
// This file contains the call-level functions for the class library.
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

IMPLEMENT_DYNCREATE (CTapiCall, CTapiObject)

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::CTapiCall
//
// Constructor for the TAPI call appearance
//
CTapiCall::CTapiCall() : m_pLine(NULL), m_hCall(0), 
	m_lpCallInfo(NULL), m_lpCallStatus(NULL), m_dwState(0)
{                       
}// CTapiCall::CTapiCall

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::~CTapiCall
//
// Destructor for the call appearance.
//
CTapiCall::~CTapiCall()
{
    Drop();
	Deallocate();

	if (m_lpCallInfo)
		FreeMem (m_lpCallInfo);
	if (m_lpCallStatus)
		FreeMem (m_lpCallStatus);

}// CTapiCall::~CTapiCall
    
////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::Init
//
// Initialize the call appearance from a line device.
//
void CTapiCall::Init (CTapiLine* pLine, HCALL hCall)
{                  
    m_pLine = pLine;
    m_hCall = hCall;

}// CTapiCall::Init

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetLineOwner
//
// Return the line owner for this call
//
CTapiLine* CTapiCall::GetLineOwner() const
{                             
    return m_pLine;

}// CTapiCall::GetLineOwner

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetCallState
//
// Return the current state of this call appearance
//
DWORD CTapiCall::GetCallState()
{                         
	if (m_dwState == 0)	
	{
		const LPLINECALLSTATUS lpStat = GetCallStatus();
		if (lpStat != NULL)
			m_dwState = lpStat->dwCallState;
		else 
			m_dwState = LINECALLSTATE_UNKNOWN;
	}
	return m_dwState;

}// CTapiCall::GetCallState

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnInfoChange
//
// Virtual function called when the call appearance information changes.
//
void CTapiCall::OnInfoChange (DWORD /*dwInfoState*/)
{                          
	// Re-get our callinfo record
	if (m_lpCallInfo != NULL)
		GetCallInfo(TRUE);

}// CTapiCall::OnInfoChange

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnStateChange
//
// Virtual function called when the call appearance state changes.
//
void CTapiCall::OnStateChange (DWORD dwState, DWORD dwStateDetail, DWORD /*dwPrivilege*/)
{
	m_dwState = dwState;

    switch (dwState)
    {
        case LINECALLSTATE_IDLE:
            OnCallStateIdle();
            break;
        case LINECALLSTATE_OFFERING:
            OnCallStateOffering(dwStateDetail);
            break;
        case LINECALLSTATE_ACCEPTED:
            OnCallStateAccepted();
            break;
        case LINECALLSTATE_DIALTONE:
            OnCallStateDialtone(dwStateDetail);
            break;
        case LINECALLSTATE_DIALING:
            OnCallStateDialing();
            break;
        case LINECALLSTATE_RINGBACK:
            OnCallStateRingback();
            break;     
        case LINECALLSTATE_BUSY:
            OnCallStateBusy(dwStateDetail);
            break;
        case LINECALLSTATE_CONNECTED:
            OnCallStateConnected(dwStateDetail);
            break;            
        case LINECALLSTATE_PROCEEDING:
            OnCallStateProceeding();
            break;
        case LINECALLSTATE_ONHOLD:
            OnCallStateHold();
            break;
        case LINECALLSTATE_ONHOLDPENDCONF:
            OnCallStateHoldPendingConference();
            break;
        case LINECALLSTATE_ONHOLDPENDTRANSFER:
            OnCallStateHoldPendingTransfer();
            break;
        case LINECALLSTATE_SPECIALINFO:
            OnCallStateSpecialInfo(dwStateDetail);
            break;
        case LINECALLSTATE_DISCONNECTED:
            OnCallStateDisconnected(dwStateDetail);
            break;
        case LINECALLSTATE_CONFERENCED:
            OnCallStateConferenced();
            break;
        default:
            break;
    }

}// CTapiCall::OnStateChange

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnGatherDigitsComplete
//
// Digit gathering is complete for some reason
//
void CTapiCall::OnGatherDigitsComplete (DWORD /*dwReason*/)
{
    /* Do nothing */
    
}// CTapiCall::OnGatherDigitsComplete

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnGenerateComplete
//
// Tone generation is complete for some reason
//
void CTapiCall::OnGenerateComplete (DWORD /*dwReason*/)
{
    /* Do nothing */
    
}// CTapiCall::OnGenerateComplete

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnDigitDetected
//
// A digit has been detected
//
void CTapiCall::OnDigitDetected (DWORD /*dwDigit*/, DWORD /*dwDigitMode*/)
{                             
    /* Do nothing */

}// CTapiCall::OnDigitDetected

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnMediaModeChange
//
// The media mode for this call has changed.
//
void CTapiCall::OnMediaModeChange (DWORD /*dwMediaMode*/)
{
    /* Do nothing */
    
}// CTapiCall::OnMediaModeChange

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnToneDetected
//
// A tone has been detected
//
void CTapiCall::OnToneDetected (DWORD /*dwAppSpecific*/)
{                             
    /* Do nothing */

}// CTapiCall::OnToneDetected

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::Drop
//
// Drop the current call appearance.
//
LONG CTapiCall::Drop(LPCSTR lpszUserToUser, DWORD dwSize)
{   
    // Only drop a call appearance in NON idle state.
    if (m_dwState != LINECALLSTATE_IDLE) 
        return ManageAsynchRequest(lineDrop (m_hCall, lpszUserToUser, dwSize));
    return 0;

}// CTapiCall::Drop

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::Deallocate
//
// Deallocate this call appearance.
//
LONG CTapiCall::Deallocate()
{       
	LONG lResult = 0;

	if (GetCallHandle() != NULL)
	{
		lResult = lineDeallocateCall (GetCallHandle());
		if (lResult == 0)
		{
			m_hCall = NULL;
			m_pLine->RemoveCall(this);
		}
	}

	// Call appearance has been deleted!!
	return lResult;

}// CTapiCall::Deallocate

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::Answer
//
// Answer this pending call.  The call appearance MUST be in the OFFERING state.
//
LONG CTapiCall::Answer (LPCSTR lpszUserToUser, DWORD dwSize)
{                    
    return ManageAsynchRequest(lineAnswer (m_hCall, lpszUserToUser, dwSize));
    
}// CTapiCall::Answer

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::BlindTransfer
//
// Transfer this call appearance to a blind station
//
LONG CTapiCall::BlindTransfer (LPCTSTR lpszDestAddr, DWORD dwCountryCode)
{                           
    return ManageAsynchRequest(lineBlindTransfer (m_hCall, lpszDestAddr, dwCountryCode));

}// CTapiCall::BlindTransfer

/////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetID
//
// Return the specified ID for the the call handle
//
LONG CTapiCall::GetID (LPVARSTRING lpDeviceID, LPCTSTR lpszDeviceClass)
{                      
    return lineGetID (NULL, 0L, m_hCall, LINECALLSELECT_CALL, lpDeviceID, lpszDeviceClass);

}// CTapiCall::GetID

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::Dial
//
// Dial a number on the call appearance
//
LONG CTapiCall::Dial (LPCTSTR lpszDestAddr, DWORD dwCountryCode)
{                           
    return ManageAsynchRequest(lineDial (m_hCall, lpszDestAddr, dwCountryCode));

}// CTapiCall::Dial

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetCallInfo
//
// Return call information buffer
//
const LPLINECALLINFO CTapiCall::GetCallInfo(BOOL fForceGet)
{                         
	// If it is cached, return it.
	if (m_lpCallInfo && !fForceGet)
		return m_lpCallInfo;

    // If we have no call handle, exit.
	if (m_hCall == NULL)
		return NULL;

    // Allocate a buffer for the call information
    DWORD dwSize = (m_lpCallInfo) ? m_lpCallInfo->dwTotalSize : sizeof (LINECALLINFO) + 1024;
    while (TRUE)
    {   
		// Allocate it if necessary.  We try to not allocate everytime.
		if (m_lpCallInfo == NULL)
		{
			m_lpCallInfo = (LPLINECALLINFO) AllocMem( dwSize);
			if (m_lpCallInfo == NULL)
				return NULL;
		}
        
        // Mark the size we are sending.
        ((LPVARSTRING)m_lpCallInfo)->dwTotalSize = dwSize;
        
        if (lineGetCallInfo (m_hCall, m_lpCallInfo) != 0)
            return NULL;
        
        // If we didn't get it all, then reallocate the buffer and retry it.
        if (m_lpCallInfo->dwNeededSize <= dwSize)
			return m_lpCallInfo;

		// Otherwise, reallocate the structure.
        dwSize = m_lpCallInfo->dwNeededSize;
		FreeMem(m_lpCallInfo);
        m_lpCallInfo = NULL;
    }    
    
}// CTapiCall::GetCallInfo

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetCallHandle
//
// Return our call handle
//
HCALL CTapiCall::GetCallHandle() const
{                           
    return m_hCall;

}// CTapiCall::GetCallHandle

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetCallStatus
//
// Return call status buffer
//
const LPLINECALLSTATUS CTapiCall::GetCallStatus()
{   
    // Allocate a buffer for the call information
    DWORD dwSize = (m_lpCallStatus) ? m_lpCallStatus->dwTotalSize : sizeof (LINECALLSTATUS) + 1024;
    while (TRUE)
    {   
		if (m_lpCallStatus == NULL)
		{
			m_lpCallStatus = (LPLINECALLSTATUS) AllocMem( dwSize);
			if (m_lpCallStatus == NULL)
				return NULL;
		}
        
        // Mark the size we are sending.
        ((LPVARSTRING)m_lpCallStatus)->dwTotalSize = dwSize;
        LONG lResult = lineGetCallStatus (m_hCall, m_lpCallStatus);
		if (lResult != 0)
            return NULL;
        
        // If we didn't get it all, then reallocate the buffer and retry it.
        if (m_lpCallStatus->dwNeededSize <= dwSize)
			return m_lpCallStatus;

        dwSize = m_lpCallStatus->dwNeededSize;
		FreeMem(m_lpCallStatus);
        m_lpCallStatus = NULL;
    }    
    
}// CTapiCall::GetCallStatus

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::Hold
//
// Place this call on hold
//
LONG CTapiCall::Hold()
{                  
    return ManageAsynchRequest(lineHold (m_hCall));

}// CTapiCall::Hold

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::Unhold
//
// Release this call from hold
//
LONG CTapiCall::Unhold()
{                  
    return ManageAsynchRequest(lineUnhold (m_hCall));

}// CTapiCall::Unhold

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::SetCallParams
//
// Set the call parameters for this call appearance.
//
LONG CTapiCall::SetCallParams (DWORD dwBearerMode, DWORD dwMinRate, DWORD dwMaxRate,
                               LPLINEDIALPARAMS const lpDialParams)
{   
    return ManageAsynchRequest(lineSetCallParams (m_hCall, dwBearerMode, dwMinRate, dwMaxRate, lpDialParams));

}// CTapiCall::SetCallParams

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::SetPrivilege
//
// Set the privilege for this call appearance.
//
LONG CTapiCall::SetPrivilege (DWORD dwCallPrivilege)
{                          
    return lineSetCallPrivilege (m_hCall, dwCallPrivilege);

}// CTapiCall::SetPrivilege 

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::SetMediaMode
//
// Set the media mode for this call appearance.
//
LONG CTapiCall::SetMediaMode (DWORD dwMediaMode)
{                          
    return lineSetMediaMode (m_hCall, dwMediaMode);

}// CTapiCall::SetMediaMode

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnCallStateIdle
//
// Default virtual method
//
void CTapiCall::OnCallStateIdle()
{   
	/* Do nothing */

}// CTapiCall::OnCallStateIdle

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnCallStateOffering
//
// Default virtual method
//
void CTapiCall::OnCallStateOffering (DWORD /*dwOfferingMode*/)
{                
    /* Do nothing */

}// CTapiCall::OnCallStateOffering

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnCallStateAccepted
//
// Default virtual method
//
void CTapiCall::OnCallStateAccepted()
{                
    /* Do nothing */

}// CTapiCall::OnCallStateAccepted

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnCallStateDialtone
//
// Default virtual method
//
void CTapiCall::OnCallStateDialtone (DWORD /*dwToneMode*/)
{                
    /* Do nothing */

}// CTapiCall::OnCallStateDialtone

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnCallStateDialing
//
// Default virtual method
//
void CTapiCall::OnCallStateDialing()
{                
    /* Do nothing */

}// CTapiCall::OnCallStateDialing

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnCallStateRingback
//
// Default virtual method
//
void CTapiCall::OnCallStateRingback()
{                
    /* Do nothing */

}// CTapiCall::OnCallStateRingback

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnCallStateBusy
//
// Default virtual method
//
void CTapiCall::OnCallStateBusy (DWORD /*dwBusyMode*/)
{                
    /* Do nothing */

}// CTapiCall::OnCallStateBusy

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnCallStateConnected
//
// Default virtual method
//
void CTapiCall::OnCallStateConnected (DWORD /*dwConnectMode*/)
{                
    /* Do nothing */

}// CTapiCall::OnCallStateConnected

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnCallStateProceeding
//
// Default virtual method
//
void CTapiCall::OnCallStateProceeding()
{                
    /* Do nothing */

}// CTapiCall::OnCallStateProceeding

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnCallStateHold
//
// Default virtual method
//
void CTapiCall::OnCallStateHold()
{                
    /* Do nothing */

}// CTapiCall::OnCallStateHold

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnCallStateHoldPendingConference
//
// Default virtual method
//
void CTapiCall::OnCallStateHoldPendingConference()
{                
    /* Do nothing */

}// CTapiCall::OnCallStatePendingConference

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnCallStatePendingTransfer
//
// Default virtual method
//
void CTapiCall::OnCallStateHoldPendingTransfer()
{                
    /* Do nothing */

}// CTapiCall::OnCallStatePendingTransfer

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnCallStateSpecialInfo
//
// Default virtual method
//
void CTapiCall::OnCallStateSpecialInfo(DWORD /*dwInfo*/)
{                
    /* Do nothing */

}// CTapiCall::OnCallStateSpecialInfo

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnCallStateDisconnected
//
// Default virtual method
//
void CTapiCall::OnCallStateDisconnected(DWORD /*dwDisconnectMode*/)
{                
    /* Do nothing */

}// CTapiCall::OnCallStateDisconnected

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::OnCallStateConferenced
//
// Default virtual method
//
void CTapiCall::OnCallStateConferenced()
{                
    /* Do nothing */

}// CTapiCall::OnCallStateConferenced

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetAddressInfo
//
// Return the CTapiAddress object associated with this call appearance.
// In some cases, there may not be an address object, this can happen with
// conference calls and intermediate transfer handles.
//
CTapiAddress* CTapiCall::GetAddressInfo()
{                            
    const LPLINECALLINFO lpInfo = GetCallInfo();                         
    if (lpInfo)
        return (CTapiAddress*) m_pLine->GetAddress(lpInfo->dwAddressID);
    return NULL;        

}// CTapiCall::GetAddressInfo

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::Accept
//
// Accepts the specified offered call. It may optionally send 
// the specified user-to-user information to the calling party.
//
LONG CTapiCall::Accept (LPCSTR lpszUserToUser, DWORD dwSize)
{                    
    return ManageAsynchRequest(lineAccept (m_hCall, lpszUserToUser, dwSize));

}// CTapiCall::Accept

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::CompleteCall
// 
// Specifies how a call that could not be connected normally should be 
// completed instead. The network or switch may not be able to complete a 
// call because network resources are busy or the remote station is busy 
// or doesn't answer. The application can request that the call be 
// completed in one of a number of ways. 
//
LONG CTapiCall::CompleteCall (LPDWORD lpdwCompletionID, DWORD dwCompletionMode, 
                              DWORD dwMessageID)
{                          
    return ManageAsynchRequest(lineCompleteCall (m_hCall, lpdwCompletionID, dwCompletionMode, dwMessageID));

}// CTapiCall::CompleteCall

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GatherDigits
//
// Initiates the buffered gathering of digits on the specified call. 
// The application specifies a buffer in which to place the digits 
// and the maximum number of digits to be collected.
//                         
LONG CTapiCall::GatherDigits(DWORD dwDigitModes, LPTSTR lpsDigits, 
                             DWORD dwNumDigits, LPCTSTR lpszTerminationDigits, 
                             DWORD dwFirstDigitTimeout, DWORD dwInterDigitTimeout)
{
    return ManageAsynchRequest(lineGatherDigits (m_hCall, dwDigitModes, lpsDigits,
                                     dwNumDigits, lpszTerminationDigits,
                                     dwFirstDigitTimeout, dwInterDigitTimeout));

}// CTapiCall::GatherDigits

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::CancelGatherDigits
//
// Turn off any active digit gathering.
//
LONG CTapiCall::CancelGatherDigits()
{
    return ManageAsynchRequest(lineGatherDigits (m_hCall, LINEDIGITMODE_DTMF, NULL, 0, NULL, 0, 0));

}// CTapiCall::CancelGatherDigits

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GenerateDigits
//
// Initiates the generation of the specified digits on the specified 
// call as inband tones using the specified signaling mode. 
// 
LONG CTapiCall::GenerateDigits (DWORD dwDigitMode, LPCTSTR lpszDigits, 
                                DWORD dwDuration)
{                            
    return ManageAsynchRequest(lineGenerateDigits (m_hCall, dwDigitMode, lpszDigits, dwDuration));

}// CTapiCall::GenerateDigits

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::CancelGenerateDigits
//
// Cancel any current digit generation which has not completed.
//
LONG CTapiCall::CancelGenerateDigits()
{                                  
    return ManageAsynchRequest(lineGenerateDigits (m_hCall, LINEDIGITMODE_DTMF, NULL, 0));

}// CTapiCall::CancelGenerateDigits

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GenerateTone
//
// Generates the specified inband tone over the specified call. 
// 
LONG CTapiCall::GenerateTone (DWORD dwToneMode, DWORD dwDuration, 
                              DWORD dwNumTones, LPLINEGENERATETONE const lpTones)
{                          
    return ManageAsynchRequest(lineGenerateTone (m_hCall, dwToneMode, dwDuration, dwNumTones, lpTones));

}// CTapiCall::GenerateTone
    
////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::CancelGenerateTone
//
// Cancels any tone generation being performed on the call.
// 
LONG CTapiCall::CancelGenerateTone ()
{                          
    return ManageAsynchRequest(lineGenerateTone (m_hCall, 0, 0, 0, NULL));

}// CTapiCall::CancelGenerateTone

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::Handoff
//
// Gives ownership of the specified call to another application. 
// The application can be either specified directly by its file name or 
// indirectly as the highest priority application that handles calls of 
// the specified media mode.
//
LONG CTapiCall::Handoff (LPCTSTR lpszFilename, DWORD dwMediaMode)
{
    return ManageAsynchRequest(lineHandoff (m_hCall, lpszFilename, dwMediaMode));

}// CTapiCall::Handoff

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::MonitorDigits
//
// Enables the unbuffered detection of digits received on the call. 
// Each time a digit of the specified digit mode(s) is detected, 
// a message is sent to the application indicating which digit has been detected.
//
LONG CTapiCall::MonitorDigits (DWORD dwDigitModes)
{
    return ManageAsynchRequest(lineMonitorDigits (m_hCall, dwDigitModes));

}// CTapiCall::MonitorDigits

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::CancelMonitorDigits
//
// Disables the unbuffered detection of digits received on the call. 
//
LONG CTapiCall::CancelMonitorDigits()
{
    return ManageAsynchRequest(lineMonitorDigits (m_hCall, 0L));

}// CTapiCall::CancelMonitorDigits

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::MonitorMedia
//
// Enables the detection of media modes on the specified call. 
// When a media mode is detected, a message is sent to the application.
//
LONG CTapiCall::MonitorMedia (DWORD dwMediaModes)
{
    return ManageAsynchRequest(lineMonitorMedia (m_hCall, dwMediaModes));
    
}// CTapiCall::MonitorMedia

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::CancelMonitorMedia
//
// Disables the detection of media modes on the specified call. 
//
LONG CTapiCall::CancelMonitorMedia()
{                   
    return ManageAsynchRequest(lineMonitorMedia (m_hCall, 0L));

}// CTapiCall::CancelMonitorMedia

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::MonitorTones
//
// Enables the detection of inband tones on the call. 
// Each time a specified tone is detected, a message is sent to the application. 
//
LONG CTapiCall::MonitorTones (LPLINEMONITORTONE const lpToneList, DWORD dwNumEntries)
{
    return ManageAsynchRequest(lineMonitorTones (m_hCall, lpToneList, dwNumEntries));

}// CTapiCall::MonitorTones

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::CancelMonitorTones
//
// Disables the detection of inband tones on the call. 
//
LONG CTapiCall::CancelMonitorTones()
{
    return ManageAsynchRequest(lineMonitorTones (m_hCall, NULL, 0L));

}// CTapiCall::CancelMonitorTones

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::Park
//
// Parks the specified call according to the specified park mode.
//
LONG CTapiCall::Park (DWORD dwParkMode, LPCTSTR lpszDestAddr, LPTSTR lpszReturnBuff, 
                      DWORD dwSize)
{                  
    LPVARSTRING lpVarString = (LPVARSTRING) AllocMem( sizeof(VARSTRING)+dwSize);
    if (lpVarString == NULL)
        return LINEERR_NOMEM;

    lpVarString->dwStringFormat = STRINGFORMAT_ASCII;
    lpVarString->dwTotalSize = sizeof(VARSTRING)+dwSize;
    lpVarString->dwStringSize = 0L;
    
    memset (lpszReturnBuff, 0, (size_t) dwSize);
    LONG lResult = ManageAsynchRequest(linePark (m_hCall, dwParkMode, lpszDestAddr, lpVarString));
    if (!IsTapiError(lResult) && GetTAPIConnection()->WaitForReply(lResult) == 0)
    {              
        memcpy (lpszReturnBuff, (LPCTSTR)lpVarString+lpVarString->dwStringOffset, 
                (size_t)lpVarString->dwStringSize);
    }

    FreeMem(lpVarString);
    return lResult;
    
}// CTapiCall::Park

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::Redirect
//
// Redirects the specified offering call to the specified destination address.
//
LONG CTapiCall::Redirect (LPCTSTR lpszDialableAddr, DWORD dwCountryCode)
{
    return ManageAsynchRequest(lineRedirect (m_hCall, lpszDialableAddr, dwCountryCode));

}// CTapiCall::Redirect

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::ReleaseUserUserInfo
//
// Informs the service provider that the application has processed the 
// user-user information contained in the LINECALLINFO structure, and 
// that subsequently received user-user information can now be written into 
// that structure.
//
// The service provider will send a LINE_CALLINFO message 
// indicating LINECALLINFOSTATE_USERUSERINFO when new information is available.
//
LONG CTapiCall::ReleaseUserUserInfo()
{
    return ManageAsynchRequest(lineReleaseUserUserInfo(m_hCall));

}// CTapiCall::ReleaseUserUserInfo

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::SecureCall
//
// Secures the call from any interruptions or interference that may 
// affect the call's media stream.
//
LONG CTapiCall::SecureCall()
{                        
    return ManageAsynchRequest(lineSecureCall (m_hCall));

}// CTapiCall::SecureCall

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::SendUserUserInfo
//
// Send user-to-user information to the remote party on the specified call.
//
LONG CTapiCall::SendUserUserInfo (LPCSTR lpszUserInfo, DWORD dwSize)
{                              
    return ManageAsynchRequest(lineSendUserUserInfo (m_hCall, lpszUserInfo, dwSize));

}// CTapiCall::SendUserUserInfo

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::SetAppSpecificData
//
// This operation enables an application to set the application-specific 
// field of the specified call's call-information record
//    
LONG CTapiCall::SetAppSpecificData (DWORD dwData)
{                                
    return lineSetAppSpecific (m_hCall, dwData);

}// CTapiCall::SetAppSpecificData

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::SetMediaControl
//
// Enables and disables control actions on the media stream associated 
// with the specified call. Media control actions can be triggered 
// by the detection of specified digits, media modes, custom tones, 
// and call states.
//
LONG CTapiCall::SetMediaControl (LPLINEMEDIACONTROLDIGIT const lpDigitList, 
                                 DWORD dwDigitNumEntries, 
                                 LPLINEMEDIACONTROLMEDIA const lpMediaList, 
                                 DWORD dwMediaNumEntries, 
                                 LPLINEMEDIACONTROLTONE const lpToneList, 
                                 DWORD dwToneNumEntries, 
                                 LPLINEMEDIACONTROLCALLSTATE const lpCallStateList,
                                 DWORD dwCallStateNumEntries)
{                             
    return lineSetMediaControl (NULL, 0, m_hCall, LINECALLSELECT_CALL,
                               lpDigitList, dwDigitNumEntries,
                               lpMediaList, dwMediaNumEntries,
                               lpToneList, dwToneNumEntries,
                               lpCallStateList, dwCallStateNumEntries);
    
}// CTapiCall::SetMediaControl
    
////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::SetTerminal
//
// Enables an application to specify which terminal information related to the 
// specified call is to be routed. lineSetTerminal can be used while calls 
// are in progress on the line to allow an application to route these events 
// to different devices as required.
//
LONG CTapiCall::SetTerminal (DWORD dwTerminalMode, DWORD dwTerminalID, BOOL fEnable)
{                         
    return ManageAsynchRequest(lineSetTerminal (NULL, 0, m_hCall, LINECALLSELECT_CALL,
                                      dwTerminalMode, dwTerminalID, (DWORD)fEnable));
    
}// CTapiCall::SetTerminal

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::SetupTransfer
//
// Begin a consultation transfer call by setting up a consultation call
// appearance for this transfer event.
//
LONG CTapiCall::SetupTransfer (CTapiCall** pXferCall, LPLINECALLPARAMS const lpCallParams)
{                                       
    HCALL hCall = NULL;
    *pXferCall = NULL;
    
    LONG lResult = ManageAsynchRequest(lineSetupTransfer (m_hCall, &hCall, lpCallParams));
    if (GetTAPIConnection()->WaitForReply(lResult) == 0)
		*pXferCall = m_pLine->CreateNewCall(hCall);
    return lResult;
    
}// CTapiCall::SetupTransfer

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::CompleteTransfer
//
// Completes the transfer of the specified call to the party connected 
// in the consultation call.
//
LONG CTapiCall::CompleteTransfer(CTapiCall* pDestCall, CTapiCall** pConfCall, 
                                 DWORD dwTransferMode)
{                              
    HCALL hCall = NULL;
    *pConfCall = NULL;
    
    LONG lResult = ManageAsynchRequest(
		lineCompleteTransfer (m_hCall, pDestCall->GetCallHandle(),
                                         &hCall, dwTransferMode));
    if (!GetTAPIConnection()->WaitForReply(lResult) &&
         dwTransferMode == LINETRANSFERMODE_CONFERENCE)
        *pConfCall = m_pLine->CreateNewCall(hCall);
    return lResult;

}// CTapiCall::CompleteTransfer
    
////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::DevSpecific
//
// Enables service providers to provide access to features not offered 
// by other TAPI functions. The meaning of the extensions are device specific, 
// and taking advantage of these extensions requires the application to be 
// fully aware of them.
//
LONG CTapiCall::DevSpecific (LPVOID lpParams, DWORD dwSize)
{
    CTapiAddress* pAddr = GetAddressInfo();
    DWORD dwAddress = (pAddr) ? pAddr->GetAddressID() : 0L;
    return  ManageAsynchRequest(
		lineDevSpecific (m_pLine->GetLineHandle(), dwAddress, m_hCall, lpParams, dwSize));

}// CTapiCall::DevSpecific

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::AddToConference
//
// Add the specified call appearance to this conference call.  The current
// call appearance MUST be a conference call handle returned by SetupConference.
//
LONG CTapiCall::AddToConference (CTapiCall* pCall)
{                             
    return ManageAsynchRequest(lineAddToConference (m_hCall, pCall->GetCallHandle()));

}// CTapiCall::AddToConference

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetConferenceRelatedCalls
//
// This function returns the list of calls related to this conference call.
// Then current call appearance object MUST be a conference call handle.
//
LONG CTapiCall::GetConferenceRelatedCalls (CObList& lstCalls)
{                                       
    DWORD dwSize = sizeof (LINECALLLIST) + 1024;
    LPLINECALLLIST lpCallList = NULL;
    
    while (TRUE)
    {
		lpCallList = (LPLINECALLLIST) AllocMem( dwSize);
        if (lpCallList == NULL)
			return LINEERR_NOMEM;
    
		lpCallList->dwTotalSize = dwSize;
        LONG lResult = lineGetConfRelatedCalls (m_hCall, lpCallList);
        if (lResult != 0)
        {
			FreeMem(lpCallList);
            return lResult;
        }                        
            
        // If we didn't get them all, then reallocate and try again.
        if (lpCallList->dwNeededSize <= dwSize)
			break;
        dwSize = lpCallList->dwNeededSize;
		FreeMem(lpCallList);
        lpCallList = NULL;
    }
    
    // Now go through the call list and create call handles for each.
    LPHCALL lphCall = (LPHCALL) ((LPTSTR)lpCallList + lpCallList->dwCallsOffset);
    for (DWORD i = 0; i < lpCallList->dwCallsNumEntries; i++)
    {
        CTapiCall* pCall = m_pLine->CreateNewCall(*lphCall);
        if (pCall)
            lstCalls.AddTail(pCall);
        lphCall++;
    }        
    
	FreeMem(lpCallList);
    return 0L;

}// CTapiCall::GetConferenceRelatedCalls
    
////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::PrepareAddToConference
//
// Prepares an existing conference call for the addition of another party.
// The current object MUST be a conference call appearance.
//    
LONG CTapiCall::PrepareAddToConference (CTapiCall** pCall, 
                                        LPLINECALLPARAMS const lpCallParams)
{                                                         
    HCALL hCall = NULL;
    *pCall = NULL;
    
    LONG lResult = ManageAsynchRequest(
		linePrepareAddToConference (m_hCall, &hCall, lpCallParams));
    if (!GetTAPIConnection()->WaitForReply(lResult))
        *pCall = m_pLine->CreateNewCall(hCall);
    return lResult;

}// CTapiCall::PrepareAddToConference

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::RemoveFromConference
//
// Remove the current call appearance from any conference call it is currently
// part of.
//
LONG CTapiCall::RemoveFromConference()
{                                  
    return ManageAsynchRequest(lineRemoveFromConference (m_hCall));

}// CTapiCall::RemoveFromConference

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::SetupConference
//
// Setup a call appearance to manage a conference
//
LONG CTapiCall::SetupConference (CTapiCall** pConfCall, CTapiCall** pConstCall,
                                 DWORD dwNumParties, 
                                 LPLINECALLPARAMS const lpCallParams)
{                             
    HCALL hCall1 = NULL;
    HCALL hCall2 = NULL;
    *pConfCall = NULL;
    *pConstCall = NULL;
    
    LONG lResult = ManageAsynchRequest(
		lineSetupConference (m_hCall, NULL, &hCall1, &hCall2, dwNumParties, lpCallParams));
    if (!GetTAPIConnection()->WaitForReply(lResult))
    {
        *pConfCall = m_pLine->CreateNewCall (hCall1);
        *pConstCall = m_pLine->CreateNewCall (hCall2);
    }
    return lResult;

}// CTapiCall::SetupConference

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetCommHandle
//
// Return a handle to the COMM port (datamodem) for this call appearance.
//
HANDLE CTapiCall::GetCommHandle ()
{                         
    LPVARSTRING lpVarString = (LPVARSTRING) AllocMem( sizeof(VARSTRING)+1024);
    if (lpVarString == NULL)
        return NULL;
    lpVarString->dwTotalSize = sizeof(VARSTRING)+1024;
    
    HANDLE hCommFile = NULL;
    if (GetID (lpVarString, _T("comm/datamodem")) == 0)
        hCommFile = *((LPHANDLE)((LPBYTE)lpVarString+lpVarString->dwStringOffset));

	FreeMem(lpVarString);
    return hCommFile;
    
}// CTapiCall::GetCommFile

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetCommDevice
//
// Return which COMM port this call is associated with
//
CString CTapiCall::GetCommDevice()
{
    LPVARSTRING lpVarString = (LPVARSTRING) AllocMem( sizeof(VARSTRING)+1024);
    if (lpVarString == NULL)
        return "";
    lpVarString->dwTotalSize = sizeof(VARSTRING)+1024;
    
	CString strBuff;
    if (GetID (lpVarString, _T("comm")) == 0)
        strBuff = *((LPCTSTR)((LPBYTE)lpVarString+lpVarString->dwStringOffset));

	FreeMem(lpVarString);
    return strBuff;

}// CTapiCall::GetCommDevice

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetWaveInDeviceID
//
// Return the device id for the wave input device.  This identifier may be
// passed to "waveInOpen" to get a HWAVE handle.
//
DWORD CTapiCall::GetWaveInDeviceID()
{                         
    LPVARSTRING lpVarString = (LPVARSTRING) AllocMem( sizeof(VARSTRING)+1024);
    if (lpVarString == NULL)
        return NULL;
    lpVarString->dwTotalSize = sizeof(VARSTRING)+10;
    
    DWORD dwID = 0L;
    if (GetID (lpVarString, _T("wave/in")) == 0)
        dwID = *((LPDWORD)((LPBYTE)lpVarString+lpVarString->dwStringOffset));

	FreeMem(lpVarString);
    return dwID;
    
}// CTapiCall::GetWaveInDeviceID

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetWaveOutDeviceID
//
// Return the device id for the wave output device.  This identifier may be
// passed to "waveInOpen" to get a HWAVE handle.
//
DWORD CTapiCall::GetWaveOutDeviceID()
{                         
    LPVARSTRING lpVarString = (LPVARSTRING) AllocMem( sizeof(VARSTRING)+1024);
    if (lpVarString == NULL)
        return NULL;
    lpVarString->dwTotalSize = sizeof(VARSTRING)+10;
    
    DWORD dwID = 0L;
    if (GetID (lpVarString, _T("wave/out")) == 0)
        dwID = *((LPDWORD)((LPBYTE)lpVarString+lpVarString->dwStringOffset));

	FreeMem(lpVarString);
    return dwID;
    
}// CTapiCall::GetWaveOutDeviceID

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetMidiInDeviceID
//
// Return the device id for the midi input device.  This identifier may be
// passed to "midiInOpen" to get a HMIDI handle.
//
DWORD CTapiCall::GetMidiInDeviceID()
{                         
    LPVARSTRING lpVarString = (LPVARSTRING) AllocMem( sizeof(VARSTRING)+1024);
    if (lpVarString == NULL)
        return NULL;
    lpVarString->dwTotalSize = sizeof(VARSTRING)+10;
    
    DWORD dwID = 0L;
    if (GetID (lpVarString, _T("midi/in")) == 0)
        dwID = *((LPDWORD)((LPBYTE)lpVarString+lpVarString->dwStringOffset));

	FreeMem(lpVarString);
    return dwID;
    
}// CTapiCall::GetMidiInDeviceID

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetMidiOutDeviceID
//
// Return the device id for the midi output device.  This identifier may be
// passed to "midiInOpen" to get a HMIDI handle.
//
DWORD CTapiCall::GetMidiOutDeviceID()
{                         
    LPVARSTRING lpVarString = (LPVARSTRING) AllocMem( sizeof(VARSTRING)+1024);
    if (lpVarString == NULL)
        return NULL;
    lpVarString->dwTotalSize = sizeof(VARSTRING)+10;
    
    DWORD dwID = 0L;
    if (GetID (lpVarString, _T("midi/out")) == 0)
        dwID = *((LPDWORD)((LPBYTE)lpVarString+lpVarString->dwStringOffset));

	FreeMem(lpVarString);
    return dwID;
    
}// CTapiCall::GetMidiOutDeviceID

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetCallStateString
//
// Return a string representing the call state of the call.
//
CString CTapiCall::GetCallStateString()
{
	CString strState = TAPISTR_LCS_UNKNOWN;
	switch (GetCallState())
	{
		case LINECALLSTATE_IDLE: strState = TAPISTR_LCS_IDLE; break;
		case LINECALLSTATE_OFFERING: strState = TAPISTR_LCS_OFFERING; break;
		case LINECALLSTATE_ACCEPTED: strState = TAPISTR_LCS_ACCEPTED; break;
		case LINECALLSTATE_DIALTONE: strState = TAPISTR_LCS_DIALTONE; break;
		case LINECALLSTATE_DIALING:	strState = TAPISTR_LCS_DIALING; break;
		case LINECALLSTATE_RINGBACK: strState = TAPISTR_LCS_RINGBACK; break;
		case LINECALLSTATE_BUSY: strState = TAPISTR_LCS_BUSY; break;
		case LINECALLSTATE_SPECIALINFO: strState = TAPISTR_LCS_SPECIALINFO; break;
		case LINECALLSTATE_CONNECTED: strState = TAPISTR_LCS_CONNECTED; break;
		case LINECALLSTATE_PROCEEDING: strState = TAPISTR_LCS_PROCEEDING; break;
		case LINECALLSTATE_ONHOLD: strState = TAPISTR_LCS_ONHOLD; break;
		case LINECALLSTATE_CONFERENCED:	strState = TAPISTR_LCS_CONFERENCED; break;
		case LINECALLSTATE_ONHOLDPENDCONF: strState = TAPISTR_LCS_ONHOLDPENDCONF; break;
		case LINECALLSTATE_ONHOLDPENDTRANSFER: strState = TAPISTR_LCS_ONHOLDPENDXFER; break;
		case LINECALLSTATE_DISCONNECTED: strState = TAPISTR_LCS_DISCONNECTED; break;
	}
	
	strState += _T(" ") + GetCallStateModeString();

	return strState;

}// CTapiCall::GetCallStateString

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetCallStateModeString
//
// Return the callstate mode
//
CString CTapiCall::GetCallStateModeString()
{
	DWORD dwState = GetCallState();
	LPLINECALLSTATUS lpCall = GetCallStatus();
	if (lpCall == NULL)
		return _T("");

	if (dwState == LINECALLSTATE_DISCONNECTED)
	{
		switch (lpCall->dwCallStateMode)
		{
			case LINEDISCONNECTMODE_NORMAL: return _T("(Normal)");
			case LINEDISCONNECTMODE_REJECT: return _T("(Reject)");
			case LINEDISCONNECTMODE_PICKUP: return _T("(Pickup)");
			case LINEDISCONNECTMODE_FORWARDED: return _T("(Fwd)");
			case LINEDISCONNECTMODE_BUSY: return _T("(Busy)");
			case LINEDISCONNECTMODE_NOANSWER: return _T("(NoAnswer)");
			case LINEDISCONNECTMODE_BADADDRESS: return _T("(BadAddress)");
			case LINEDISCONNECTMODE_UNREACHABLE: return _T("(Unreachable)");
			case LINEDISCONNECTMODE_CONGESTION: return _T("(Congestion)");
			case LINEDISCONNECTMODE_INCOMPATIBLE: return _T("(Incompatible)");
			case LINEDISCONNECTMODE_NODIALTONE: return _T("(No Dialtone)");
			case LINEDISCONNECTMODE_NUMBERCHANGED: return _T("(Number Changed)");
			case LINEDISCONNECTMODE_OUTOFORDER: return _T("(Out of Order)");
			case LINEDISCONNECTMODE_TEMPFAILURE: return _T("(Temp Failure)");
			case LINEDISCONNECTMODE_QOSUNAVAIL: return _T("(QOS Unavalable)");
			case LINEDISCONNECTMODE_BLOCKED: return _T("(Blocked)");
			case LINEDISCONNECTMODE_DONOTDISTURB: return _T("(Do Not Disturb)");
			case LINEDISCONNECTMODE_CANCELLED: return _T("(Canceled)");
		}
	}

	else if (dwState == LINECALLSTATE_CONNECTED)
	{
		switch (lpCall->dwCallStateMode)
		{
			case LINECONNECTEDMODE_ACTIVE: return _T("(Active)");
			case LINECONNECTEDMODE_INACTIVE: return _T("(Inactive)");
			case LINECONNECTEDMODE_ACTIVEHELD: return _T("(ActiveHeld)");
			case LINECONNECTEDMODE_INACTIVEHELD: return _T("(InactiveHeld)");
			case LINECONNECTEDMODE_CONFIRMED: return _T("(Confirmed)");
		}
	}

	else if (dwState == LINECALLSTATE_BUSY)
	{
		switch (lpCall->dwCallStateMode)
		{
			case LINEBUSYMODE_STATION: return _T("(Station)");
			case LINEBUSYMODE_TRUNK: return _T("(Trunk)");
		}		
	}

	else if (dwState == LINECALLSTATE_OFFERING)
	{
		switch (lpCall->dwCallStateMode)
		{
			case LINEOFFERINGMODE_ACTIVE: return _T("(Active)");
			case LINEOFFERINGMODE_INACTIVE: return _T("(Inactive)");
		}
	}

	else if (dwState == LINECALLSTATE_DIALTONE)
	{
		switch (lpCall->dwCallStateMode)
		{
			case LINEDIALTONEMODE_NORMAL: return _T("(Normal)");
			case LINEDIALTONEMODE_SPECIAL: return _T("(Special)");
			case LINEDIALTONEMODE_INTERNAL: return _T("(Internal)");
			case LINEDIALTONEMODE_EXTERNAL: return _T("(External)");
		}
	}

	return _T("");

}// CTapiCall::GetCallStateModeString

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetCallerIDNumber
//
// Return a string representing the caller number (if available)
//
CString CTapiCall::GetCallerIDNumber()
{
	const LPLINECALLINFO pCallInfo = GetCallInfo();
	if (pCallInfo->dwCallerIDFlags & LINECALLPARTYID_ADDRESS &&
		pCallInfo->dwCallerIDSize > 0)
	{
		CString strNumber = (LPCTSTR)((LPBYTE)pCallInfo + pCallInfo->dwCallerIDOffset);
		return strNumber;
	}
	return "";

}// CTapiCall::GetCallerIDNumber

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetCallerIDName
//
// Return a string representing the caller number (if available)
//
CString CTapiCall::GetCallerIDName()
{
	const LPLINECALLINFO pCallInfo = GetCallInfo();
	if (pCallInfo->dwCallerIDFlags & LINECALLPARTYID_NAME &&
		pCallInfo->dwCallerIDNameSize > 0)
	{
		CString strName = (LPCTSTR)((LPBYTE)pCallInfo + pCallInfo->dwCallerIDNameOffset);
		return strName;
	}
	return "";

}// CTapiCall::GetCallerIDName

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetConnectedIDNumber
//
// Return a string representing the caller number (if available)
//
CString CTapiCall::GetConnectedIDNumber()
{
	const LPLINECALLINFO pCallInfo = GetCallInfo();
	if (pCallInfo->dwConnectedIDFlags & LINECALLPARTYID_ADDRESS &&
		pCallInfo->dwConnectedIDSize > 0)
	{
		CString strNumber = (LPCTSTR)((LPBYTE)pCallInfo + pCallInfo->dwConnectedIDOffset);
		return strNumber;
	}
	return "";

}// CTapiCall::GetConnectedIDNumber

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetConnectedIDName
//
// Return a string representing the caller number (if available)
//
CString CTapiCall::GetConnectedIDName()
{
	const LPLINECALLINFO pCallInfo = GetCallInfo();
	if (pCallInfo->dwConnectedIDFlags & LINECALLPARTYID_NAME &&
		pCallInfo->dwConnectedIDNameSize > 0)
	{
		CString strName = (LPCTSTR)((LPBYTE)pCallInfo + pCallInfo->dwConnectedIDNameOffset);
		return strName;
	}
	return "";

}// CTapiCall::GetConnectedIDName

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetCalledIDNumber
//
// Return a string representing the called number (if available)
//
CString CTapiCall::GetCalledIDNumber()
{
	const LPLINECALLINFO pCallInfo = GetCallInfo();
	if (pCallInfo->dwCalledIDFlags & LINECALLPARTYID_ADDRESS &&
		pCallInfo->dwCalledIDSize > 0)
	{
		CString strNumber = (LPCTSTR)((LPBYTE)pCallInfo + pCallInfo->dwCalledIDOffset);
		return strNumber;
	}
	return "";

}// CTapiCall::GetCalledIDNumber

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetCalledIDName
//
// Return a string representing the caller number (if available)
//
CString CTapiCall::GetCalledIDName()
{
	const LPLINECALLINFO pCallInfo = GetCallInfo();
	if (pCallInfo->dwCalledIDFlags & LINECALLPARTYID_NAME &&
		pCallInfo->dwCalledIDNameSize > 0)
	{
		CString strName = (LPCTSTR)((LPBYTE)pCallInfo + pCallInfo->dwCalledIDNameOffset);
		return strName;
	}
	return "";

}// CTapiCall::GetCalledIDName

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetRedirectingIDNumber
//
// Return a string representing the caller number (if available)
//
CString CTapiCall::GetRedirectingIDNumber()
{
	const LPLINECALLINFO pCallInfo = GetCallInfo();
	if (pCallInfo->dwRedirectingIDFlags & LINECALLPARTYID_ADDRESS &&
		pCallInfo->dwRedirectingIDSize > 0)
	{
		CString strNumber = (LPCTSTR)((LPBYTE)pCallInfo + pCallInfo->dwRedirectingIDOffset);
		return strNumber;
	}
	return "";

}// CTapiCall::GetRedirectingIDNumber

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetRedirectingIDName
//
// Return a string representing the caller number (if available)
//
CString CTapiCall::GetRedirectingIDName()
{
	const LPLINECALLINFO pCallInfo = GetCallInfo();
	if (pCallInfo->dwRedirectingIDFlags & LINECALLPARTYID_NAME &&
		pCallInfo->dwRedirectingIDNameSize > 0)
	{
		CString strName = (LPCTSTR)((LPBYTE)pCallInfo + pCallInfo->dwRedirectingIDNameOffset);
		return strName;
	}
	return "";

}// CTapiCall::GetRedirectingIDName

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetRedirectedFromIDNumber
//
// Return a string representing the caller number (if available)
//
CString CTapiCall::GetRedirectedFromIDNumber()
{
	const LPLINECALLINFO pCallInfo = GetCallInfo();
	if (pCallInfo->dwRedirectionIDFlags & LINECALLPARTYID_ADDRESS &&
		pCallInfo->dwRedirectionIDSize > 0)
	{
		CString strNumber = (LPCTSTR)((LPBYTE)pCallInfo + pCallInfo->dwRedirectionIDOffset);
		return strNumber;
	}
	return "";

}// CTapiCall::GetRedirectedFromIDNumber

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::GetRedirectedFromIDName
//
// Return a string representing the caller number (if available)
//
CString CTapiCall::GetRedirectedFromIDName()
{
	const LPLINECALLINFO pCallInfo = GetCallInfo();
	if (pCallInfo->dwRedirectionIDFlags & LINECALLPARTYID_NAME &&
		pCallInfo->dwRedirectionIDNameSize > 0)
	{
		CString strName = (LPCTSTR)((LPBYTE)pCallInfo + pCallInfo->dwRedirectionIDNameOffset);
		return strName;
	}
	return "";

}// CTapiCall::GetRedirectedFromIDName

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::SetCallData
//
// Sets the CALLDATA field of the call object
//    
LONG CTapiCall::SetCallData(LPVOID lpBuff, DWORD dwSize)
{                                                         
    return ManageAsynchRequest(lineSetCallData(m_hCall, lpBuff, dwSize));

}// CTapiCall::SetCallData

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::SetQualityOfService
//
// Sets the QOS fields of the call object
//    
LONG CTapiCall::SetQualityOfService(LPVOID lpvSendingFlowSpec, DWORD dwSFSize, LPVOID lpvRcvFlowSpec, DWORD dwRFSize)
{                                                         
    return ManageAsynchRequest(lineSetCallQualityOfService(m_hCall, lpvSendingFlowSpec, dwSFSize, lpvRcvFlowSpec, dwRFSize));

}// CTapiCall::SetCallData

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall::WaitForFeature
//
// Wait for a call feature to show up.
//    
bool CTapiCall::WaitForFeature(DWORD dwFeature, DWORD dwTimeout)
{
	while (TRUE)
	{
		LPLINECALLSTATUS plcs = GetCallStatus();
		if ((plcs->dwCallFeatures & dwFeature) == dwFeature)
			return true;
		
		MSG msg;
		DWORD dwTickCount = GetTickCount();
		while (dwTickCount + dwTimeout > GetTickCount())
		{
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	return false;

}// CTapiCall::WaitForFeature
