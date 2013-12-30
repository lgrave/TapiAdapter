//
//	Last Modified: $Date: 2010-07-20 09:48:12 $
//
//	$Log: Tapiphone.cpp,v $
//	Revision 1.2  2010-07-20 09:48:12  lgrave
//	corrected windows crlf to unix lf
//

//	Revision 1.1  2010-07-19 23:40:42  lgrave

//	1st version added to cvs

//
//

// TAPIPHONE.CPP
//
// This file contains the phone-level functions for the 
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

IMPLEMENT_DYNCREATE (CTapiPhone, CTapiObject)

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::CTapiPhone
//
// Constructor for the phone object
//
CTapiPhone::CTapiPhone() : m_pConn(0), m_iFlags(0), m_hPhone(0), m_dwDeviceID(0xffffffff), 
	m_dwAPIVersion(0), m_lpPhoneCaps(0), m_lpPhoneStatus(0)
{                       
}// CTapiPhone::CTapiPhone

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::~CTapiPhone
//
// Destructor for the phone object
//
CTapiPhone::~CTapiPhone()
{   
	// Close the phone if necessary
	Close();

	// Delete our two informational blocks.
	if (m_lpPhoneCaps)
		FreeMem (m_lpPhoneCaps);
	if (m_lpPhoneStatus)
		FreeMem (m_lpPhoneStatus);

}// CTapiPhone::~CTapiPhone

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::Init
//
// Initialize the phone object
//
void CTapiPhone::Init (CTapiConnection* pConn, DWORD dwDeviceID)
{                  
    m_pConn = pConn;
    
    // Negotiate the API version with TAPI.
    PHONEEXTENSIONID phoneExtID;
    if (phoneNegotiateAPIVersion (pConn->GetPhoneAppHandle(),
              dwDeviceID, TAPIVER_13, TAPIVER_21,
              &m_dwAPIVersion, &phoneExtID) == 0)
    {
		// Save off the device ID.
        m_dwDeviceID = dwDeviceID;
    }                                

}// CTapiPhone::Init

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetNegotiatedAPIVersion
//
// Return the negotiated API version for TAPI
//
DWORD CTapiPhone::GetNegotiatedAPIVersion() const
{                                     
    return m_dwAPIVersion;

}// CTapiPhone::GetNegotiatedAPIVersion

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetTapiConnection
//
// Return the owner TAPI connection object
//
CTapiConnection* CTapiPhone::GetTapiConnection() const
{                               
    return m_pConn;

}// CTapiPhone::GetTapiConnection

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetDeviceID
//
// Return the device ID for the phone object
// 
DWORD CTapiPhone::GetDeviceID() const
{                         
    return m_dwDeviceID;

}// CTapiPhone::GetDeviceID

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::IsOpen
//
// Return whether this phone is open for use.
//
BOOL CTapiPhone::IsOpen() const
{                    
    return (m_hPhone != NULL);

}// CTapiPhone::IsOpen

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetPhoneHandle
//
// Return the phone handle for this phone
//
HPHONE CTapiPhone::GetPhoneHandle() const
{                           
    return m_hPhone;

}// CTapiPhone::GetPhoneHandle

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetProviderInfo
//
// Return the name of the service provider for this device
//
CString CTapiPhone::GetProviderInfo() const
{                             
    CString strProviderName = TAPISTR_NOPROVIDERINFO;

    if (m_lpPhoneCaps)
    {    
        if (m_lpPhoneCaps->dwProviderInfoSize &&
            m_lpPhoneCaps->dwProviderInfoOffset)
//            m_lpPhoneCaps->dwStringFormat == STRINGFORMAT_ASCII)
        {
            LPCTSTR lpszProvider = ((LPCTSTR)m_lpPhoneCaps)+m_lpPhoneCaps->dwProviderInfoOffset;
            strProviderName = lpszProvider;
        }
    }

    return strProviderName;

}// CTapiPhone::GetProviderInfo

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetPhoneName
//
// Return the name for this phone device
//
CString CTapiPhone::GetPhoneName() const
{                         
    CString strName;
    LPPHONECAPS lpCaps = ((CTapiPhone*)this)->GetPhoneCaps();
    if (lpCaps)
    {
        if (lpCaps->dwPhoneNameSize && lpCaps->dwPhoneNameOffset)
//            lpCaps->dwStringFormat == STRINGFORMAT_ASCII)
        {
            LPCTSTR lpszName = ((LPCTSTR)lpCaps)+lpCaps->dwPhoneNameOffset;
            strName = lpszName;
        }
        else
            strName = TAPISTR_NOPHONENAME;
    }
    else
        strName = TAPISTR_NOPHONE;            

    return strName;
    
}// CTapiPhone::GetPhoneName

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetPhoneCaps
//
// Return the phone capabilities we loaded on entry - don't allow modification
// of the structure.
//
const LPPHONECAPS CTapiPhone::GetPhoneCaps(DWORD dwAPIVersion, DWORD dwExtVersion, BOOL fForceRealloc)
{   
	// If there is no version information, then use our negotiated version.
	if (dwAPIVersion == 0)
		dwAPIVersion = GetNegotiatedAPIVersion();

    if (m_lpPhoneCaps != NULL && !fForceRealloc)
		return m_lpPhoneCaps;

    // Allocate a buffer for the phone capabilities
    DWORD dwSize = (m_lpPhoneCaps) ? m_lpPhoneCaps->dwTotalSize : sizeof(PHONECAPS)+1024;
    while (TRUE)
    {
		if (m_lpPhoneCaps == NULL)
		{
			m_lpPhoneCaps = (LPPHONECAPS) AllocMem ( dwSize);
			if (m_lpPhoneCaps == NULL)
				return NULL;
		}
        
        // Mark the size we are sending.
        ((LPVARSTRING)m_lpPhoneCaps)->dwTotalSize = dwSize;
        if (phoneGetDevCaps (m_pConn->GetPhoneAppHandle(), GetDeviceID(), 
						    dwAPIVersion, dwExtVersion, m_lpPhoneCaps) != 0)
			return NULL;
        
        // If we didn't get it all, then reallocate the buffer and retry it.
        if (m_lpPhoneCaps->dwNeededSize <= dwSize)
			return m_lpPhoneCaps;

        dwSize = m_lpPhoneCaps->dwNeededSize;
		FreeMem (m_lpPhoneCaps);
        m_lpPhoneCaps = NULL;
    }    

}// CTapiPhone::GetPhoneCaps

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::Open
//
// Open the phone device
//
LONG CTapiPhone::Open(DWORD dwPrivilege, DWORD dwAPIVersion, DWORD dwExtVersion)
{   
    if (IsOpen())
        return FALSE;

	// If no version information given, use the highest negotiated during
	// our INIT process.
	if (dwAPIVersion == 0)
		dwAPIVersion = GetNegotiatedAPIVersion();

    LONG lResult = phoneOpen (m_pConn->GetPhoneAppHandle(), GetDeviceID(),
                             &m_hPhone, dwAPIVersion, dwExtVersion, (DWORD)this, dwPrivilege);
	if (lResult != 0)
		m_hPhone = NULL;
	return lResult;

}// CTapiPhone::Open                     

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::Close
//
// Close the phone
//
LONG CTapiPhone::Close()
{                  
    if (!IsOpen())
        return FALSE;

	LONG lResult = phoneClose (GetPhoneHandle());
	if (lResult == 0)
		OnClose();

    return lResult;

}// CTapiPhone::Close

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::PhoneCallback
//
// Callback from the CTapiConnection object about one of our settings
// changing.
//
void CTapiPhone::PhoneCallback (DWORD hDevice, DWORD dwMsg, DWORD dwParam1, 
                                DWORD dwParam2, DWORD dwParam3)
{                          
    switch (dwMsg)
    {
		case PHONE_BUTTON:
			OnButton(dwParam1, dwParam2, dwParam3);
			break;

		case PHONE_DEVSPECIFIC:
			OnDevSpecific(hDevice, dwParam1, dwParam2, dwParam3);
			break;

		case PHONE_STATE:
			OnDeviceStateChange(dwParam1, dwParam2);
			break;

        default:
            TRACE (_T("CTapiPhone: unknown TAPI callback %ld\r\n"), dwMsg);
            break;    
    }

}// CTapiPhone::PhoneCallback

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::OnClose
//
// Close the phone
//
void CTapiPhone::OnClose()
{                     
    m_hPhone = NULL;
	
}// CTapiPhone::OnClose

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::OnDevSpecific
//
// This function is called whenever we receive a device-specific message from
// a service provider.  The default implementation does nothing.
//
void CTapiPhone::OnDevSpecific (DWORD /*dwHandle*/, DWORD /*dwParam1*/, 
                               DWORD /*dwParam2*/, DWORD /*dwParam3*/)
{                           
    /* Do nothing */
    
}// CTapiPhone::OnDevSpecific

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::OnButton
//
// This function is called whenever a button change occurs on our device
//
void CTapiPhone::OnButton(DWORD /*dwButtonLampID*/, DWORD /*dwButtonMode*/, DWORD /*dwButtonState*/)
{
    /* Do nothing */

}// CTapiPhone::OnButton

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::OnDeviceStateChange
//
// This function is called whenever something changes in our status block
//
void CTapiPhone::OnDeviceStateChange(DWORD dwPhoneState, DWORD /*dwDetail*/)
{
	if (dwPhoneState & PHONESTATE_CAPSCHANGE)
	{
		if (m_lpPhoneCaps)
			GetPhoneCaps(0, 0, TRUE);
		dwPhoneState &= ~PHONESTATE_CAPSCHANGE;
	}

	if (dwPhoneState)
	{
		if (m_lpPhoneStatus)
			GetPhoneStatus(TRUE);
	}

}// CTapiPhone::OnDeviceStateChange

/////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetID
//
// Return the specified ID for the the phone device.
//
LONG CTapiPhone::GetID (LPVARSTRING lpDeviceID, LPCTSTR lpszDeviceClass)
{                      
    return phoneGetID (GetPhoneHandle(), lpDeviceID, lpszDeviceClass);

}// CTapiPhone::GetID

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::Config
//
// Manage the configuration for this phone device
//
LONG CTapiPhone::Config (CWnd* pwndOwner, LPCTSTR lpszDeviceClass)
{   
    return phoneConfigDialog (m_dwDeviceID, (pwndOwner) ? pwndOwner->GetSafeHwnd() : NULL, 
							 lpszDeviceClass);

}// CTapiPhone::Config

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetIcon
//
// Return the icon for this phone device.
//
HICON CTapiPhone::GetIcon(LPCTSTR lpszDeviceClass)
{                     
    HICON hIcon = NULL;
    if (::phoneGetIcon (m_dwDeviceID, lpszDeviceClass, &hIcon) == 0)
		return hIcon;
	return NULL;

}// CTapiPhone::GetIcon

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetPhoneStatus
//
// Return the status of this phone device
//
const LPPHONESTATUS CTapiPhone::GetPhoneStatus(BOOL fForceRealloc)
{   
	if (m_lpPhoneStatus && !fForceRealloc)
		return m_lpPhoneStatus;
    
    // Allocate a buffer for the phone information
    DWORD dwSize = (m_lpPhoneStatus) ? m_lpPhoneStatus->dwTotalSize : sizeof (PHONESTATUS) + 1024;
    while (TRUE)
    {   
		if (m_lpPhoneStatus == NULL)
		{
			m_lpPhoneStatus = (LPPHONESTATUS) AllocMem( dwSize);
			if (m_lpPhoneStatus == NULL)
				return NULL;
		}
        
        // Mark the size we are sending.
        ((LPVARSTRING)m_lpPhoneStatus)->dwTotalSize = dwSize;
        if (phoneGetStatus(GetPhoneHandle(), m_lpPhoneStatus) != 0)
            return NULL;
        
        if (m_lpPhoneStatus->dwNeededSize <= dwSize)
            return m_lpPhoneStatus;

        // If we didn't get it all, then reallocate the buffer and retry it.
        dwSize = m_lpPhoneStatus->dwNeededSize;
		FreeMem (m_lpPhoneStatus);
        m_lpPhoneStatus = NULL;
    }    

}// CTapiPhone::GetPhoneStatus

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::SetStatusMessages
//
// Set the status notifications
//
LONG CTapiPhone::SetStatusMessages (DWORD dwPhoneStates, DWORD dwButtonModes, DWORD dwButtonStates)
{                                                    
    if (!IsOpen())
        return PHONEERR_INVALPHONEHANDLE;
    return phoneSetStatusMessages (GetPhoneHandle(), dwPhoneStates, dwButtonModes, dwButtonStates);

}// CTapiPhone::SetStatusMessages

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetStatusMessages
//
// Get the status notifications
//
LONG CTapiPhone::GetStatusMessages (LPDWORD lpdwPhoneStatus, LPDWORD lpdwButtonModes, LPDWORD lpdwButtonStates)
{
    if (!IsOpen())
        return PHONEERR_INVALPHONEHANDLE;
    return phoneGetStatusMessages (GetPhoneHandle(), lpdwPhoneStatus, lpdwButtonModes, lpdwButtonStates);

}// CTapiPhone::GetStatusMessages
              
////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::DevSpecific
//
// Enables service providers to provide access to features not offered by 
// other TAPI functions. The meaning of these extensions are device specific, 
// and taking advantage of these extensions requires the application to be 
// fully aware of them.
//
LONG CTapiPhone::DevSpecific (LPVOID lpParams, DWORD dwSize)
{                         
    return ManageAsynchRequest(phoneDevSpecific(GetPhoneHandle(), lpParams, dwSize));

}// CTapiPhone::DevSpecific

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetProviderID
//
// Return the provider information for the TSP which owns this phone.
//
DWORD CTapiPhone::GetProviderID() const
{
	DWORD dwProviderID = 0;
	LPVARSTRING lpVS = (LPVARSTRING) AllocMem(sizeof(VARSTRING) + sizeof(DWORD));
	lpVS->dwTotalSize = sizeof(VARSTRING) + sizeof(DWORD);
	if (((CTapiPhone*)this)->GetID(lpVS, _T("tapi/providerid")) == 0 && lpVS->dwStringSize == sizeof(DWORD) &&
		lpVS->dwStringFormat == STRINGFORMAT_BINARY)
		dwProviderID = *((LPDWORD)((LPBYTE)lpVS + lpVS->dwStringOffset));
	FreeMem (lpVS);

	return dwProviderID;

}// CTapiPhone::GetProviderID
        
////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetTSPProvider
//
// Return the provider information for the TSP which owns this phone.
//
BOOL CTapiPhone::GetTSPProvider (LPTAPIPROVIDER pProvider) const
{   
	// First see if we can get the provider id from the phone device.
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
    
}// CTapiPhone::GetProviderInfo

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetValidIDs
//
// Return a list of valid keys for phoneGetID
//
void CTapiPhone::GetValidIDs(CStringArray& arrKeys) const
{
	LPPHONECAPS lpCaps = ((CTapiPhone*)this)->GetPhoneCaps();
	if (lpCaps && lpCaps->dwDeviceClassesSize > 0)
	{
		LPCTSTR pszKey = (LPCTSTR)((LPBYTE)lpCaps) + lpCaps->dwDeviceClassesOffset;
		while (*pszKey)
		{
			arrKeys.Add(pszKey);
			pszKey += lstrlen(pszKey)+1;
		}
	}

}// CTapiPhone::GetValidIDs

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::IsValid
//
// Return whether this phone is valid
//
BOOL CTapiPhone::IsValid() const
{
	return ((m_iFlags & Removed) == 0);

}// CTapiPhone::IsValid

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::OnDynamicCreate
//
// Called when the phone is dynamically created
//
void CTapiPhone::OnDynamicCreate()
{
}// CTapiPhone::OnDynamicCreate

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::OnDynamicRemove
//
// Called when the phone is dynamically removed
//
void CTapiPhone::OnDynamicRemove()
{
}// CTapiPhone::OnDynamicRemove

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::OnForceClose
//
// Called when the phone is forced closed by TAPI
//
void CTapiPhone::OnForceClose()
{
	// Close the phone
	OnClose();

}// CTapiPhone::OnForceClose

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetRelatedLineID
//
// Returns the related line device id for this phone.
//
DWORD CTapiPhone::GetRelatedLineID()
{
    LPVARSTRING lpVarString = (LPVARSTRING) AllocMem( sizeof(VARSTRING)+1024);
    if (lpVarString == NULL)
        return NULL;
    lpVarString->dwTotalSize = sizeof(VARSTRING)+10;
    
    DWORD dwID = 0xffffffff;
    if (GetID (lpVarString, _T("tapi/line")) == 0)
        dwID = *((LPDWORD)((LPBYTE)lpVarString+lpVarString->dwStringOffset));

	FreeMem(lpVarString);
    return dwID;
	
}// CTapiPhone::GetRelatedLineID

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetDisplay
//
// Return the current display (if supported)
//
CString CTapiPhone::GetDisplay() const
{
	CString strDisplay = _T("");
	LPPHONESTATUS lpStatus = ((CTapiPhone*)this)->GetPhoneStatus();
	if (lpStatus && lpStatus->dwDisplaySize > 0)
	{
		LPCSTR pszDisplay = ((LPCSTR)lpStatus) + lpStatus->dwDisplayOffset;
		if (strlen(pszDisplay) == 1 && lpStatus->dwDisplaySize > 1)
		{
			// UNICODE returned string.
			LPCWSTR pszUnicode = (LPCWSTR) pszDisplay;
			strDisplay = CString(pszUnicode);
		}
		else
			strDisplay = CString(pszDisplay, lpStatus->dwDisplaySize);
	}
	return strDisplay;

}// CTapiPhone::GetDisplay

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::SetVolume
//
// Set the volume of a hookswitch device
//
LONG CTapiPhone::SetVolume(DWORD dwDevice, DWORD dwVolume)
{
    return ManageAsynchRequest(phoneSetVolume(GetPhoneHandle(), dwDevice, dwVolume));

}// CTapiPhone::SetVolume

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::SetGain
//
// Set the gain of a hookswitch device
//
LONG CTapiPhone::SetGain(DWORD dwDevice, DWORD dwGain)
{
    return ManageAsynchRequest(phoneSetGain(GetPhoneHandle(), dwDevice, dwGain));

}// CTapiPhone::SetGain

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::SetHookswitch
//
// Set the state of a hookswitch device
//
LONG CTapiPhone::SetHookswitch(DWORD dwDevice, DWORD dwMode)
{
    return ManageAsynchRequest(phoneSetHookSwitch(GetPhoneHandle(), dwDevice, dwMode));

}// CTapiPhone::SetHookswitch

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::SetDisplay
//
// Set the display for the phone
//
LONG CTapiPhone::SetDisplay(LPCSTR pszDisplay)
{
    return ManageAsynchRequest(phoneSetDisplay(GetPhoneHandle(), 0, 0, pszDisplay, lstrlenA(pszDisplay)));

}// CTapiPhone::SetDisplay

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetButtonInfo
//
// Get button information from TAPI
//
LONG CTapiPhone::GetButtonInfo(int iButtonID, LPBUTTONINFO lpButton)
{
    // Allocate a buffer for the phone information
	LPPHONEBUTTONINFO lpInfo = NULL;
    DWORD dwSize = sizeof (PHONEBUTTONINFO) + 1024;
    while (TRUE)
    {   
		if (lpInfo == NULL)
		{
			lpInfo = (LPPHONEBUTTONINFO) AllocMem( dwSize);
			if (lpInfo == NULL)
				return PHONEERR_NOMEM;
		}
        
        // Mark the size we are sending.
        ((LPVARSTRING)lpInfo)->dwTotalSize = dwSize;
        LONG lResult = phoneGetButtonInfo(GetPhoneHandle(), (DWORD)iButtonID, lpInfo);
		if (lResult != 0)
            return lResult;
        
        if (lpInfo->dwNeededSize <= dwSize)
			break;

        // If we didn't get it all, then reallocate the buffer and retry it.
        dwSize = lpInfo->dwNeededSize;
		FreeMem (lpInfo);
        lpInfo = NULL;
    }    

	lpButton->dwButtonMode = lpInfo->dwButtonMode;
	lpButton->dwButtonFunction = lpInfo->dwButtonFunction;
	lpButton->dwButtonState = lpInfo->dwButtonState;
	if (lpInfo->dwButtonTextOffset > 0)
		lpButton->strButtonText = ((LPCSTR)lpInfo + lpInfo->dwButtonTextOffset);
	else
		lpButton->strButtonText = _T("");
	FreeMem(lpInfo);

	return 0;

}// CTapiPhone::GetButtonInfo

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::GetLampMode
//
// Get lamp information from TAPI
//
DWORD CTapiPhone::GetLampMode(int iLampID)
{
	PHONESTATUS* pStatus = GetPhoneStatus();
	if (pStatus != NULL && pStatus->dwLampModesSize >= (iLampID*sizeof(DWORD)))
	{
		LPDWORD lpMode = (LPDWORD)((LPBYTE)pStatus+pStatus->dwLampModesOffset);
		return *(lpMode+iLampID);
	}
	return PHONELAMPMODE_UNKNOWN;

}// CTapiPhone::GetLampMode

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone::SetRing
//
// Set the ring mode and volume
//
LONG CTapiPhone::SetRing(DWORD dwRingMode, DWORD dwRingVolume)
{
    return ManageAsynchRequest(phoneSetRing(GetPhoneHandle(), dwRingMode, dwRingVolume));

}// CTapiPhone::SetRing
