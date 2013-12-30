//
//	Last Modified: $Date: 2010-07-20 09:48:12 $
//
//	$Log: Tapiaddr.cpp,v $
//	Revision 1.2  2010-07-20 09:48:12  lgrave
//	corrected windows crlf to unix lf
//

//	Revision 1.1  2010-07-19 23:40:41  lgrave

//	1st version added to cvs

//
//

// TAPIADDR.CPP
//
// This file contains the code for managing a single dialable address inside the class library.
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

IMPLEMENT_DYNCREATE (CTapiAddress, CTapiObject)

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::CTapiAddress
//
// Constructor for the TAPI address structure
//
CTapiAddress::CTapiAddress()
{                             
    m_pLine = NULL;
    m_dwAddressID = 0L;
    m_strAddress = "";
    m_lpAddrCaps = NULL;
    m_lpAddrStatus = NULL;
	m_lpAgentCaps = NULL;
	m_lpAgentStatus = NULL;
	m_fAgentCapsReload = TRUE;
	m_fAgentStatsReload = TRUE;

}// CTapiAddress::CTapiAddress

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::~CTapiAddress
//
// Destructor for the address structure
//
CTapiAddress::~CTapiAddress()
{   
	if (m_lpAddrCaps)
		FreeMem ( m_lpAddrCaps);
	if (m_lpAddrStatus)
		FreeMem ( m_lpAddrStatus);
	if (m_lpAgentCaps)
		FreeMem( m_lpAgentCaps);
	if (m_lpAgentStatus)
		FreeMem( m_lpAgentStatus);

}// CTapiAddress::~CTapiAddress

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::Init
//
// Initialize the address object from a line object
//
void CTapiAddress::Init (CTapiLine* pLine, DWORD dwAddressID)
{                     
    m_pLine = pLine;
    m_dwAddressID = dwAddressID;
    
}// CTapiAddress::Init

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::GetLineOwner
//
// Return the line owner for this address
//
CTapiLine* CTapiAddress::GetLineOwner() const
{                             
    return m_pLine;

}// CTapiAddress::GetLineOwner

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::GetAddressID
//
// Return the address id for this object
//
DWORD CTapiAddress::GetAddressID() const
{                             
    return m_dwAddressID;

}// CTapiAddress::GetAddressID

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::GetAddressCaps
//
// Return the address capabilities for this address object
//
const LPLINEADDRESSCAPS CTapiAddress::GetAddressCaps(DWORD dwAPIVersion, DWORD dwExtVersion, BOOL fForceRealloc)
{
	if (m_lpAddrCaps && !fForceRealloc)
		return m_lpAddrCaps;

	// If there was no passed version, use the negotiated version for
	// the line owner.
	if (dwAPIVersion == 0)
		dwAPIVersion = m_pLine->GetNegotiatedAPIVersion();

    // Allocate a buffer for the address information
    DWORD dwSize = (m_lpAddrCaps) ? m_lpAddrCaps->dwTotalSize : sizeof (LINEADDRESSCAPS) + 1024;
    while (TRUE)
    {   
		if (m_lpAddrCaps == NULL)
		{
			m_lpAddrCaps = (LPLINEADDRESSCAPS) AllocMem( dwSize);
			if (m_lpAddrCaps == NULL)
				return NULL;
		}
        
        // Mark the size we are sending.
        ((LPVARSTRING)m_lpAddrCaps)->dwTotalSize = dwSize;
        if (lineGetAddressCaps (m_pLine->GetTapiConnection()->GetLineAppHandle(),
                                m_pLine->GetDeviceID(), GetAddressID(),
                                dwAPIVersion, dwExtVersion, m_lpAddrCaps)  != 0)
            return NULL;

        // Return the structure if we got it all.
        if (m_lpAddrCaps->dwNeededSize <= dwSize)
			return m_lpAddrCaps;

        // If we didn't get it all, then reallocate the buffer and retry it.
        dwSize = m_lpAddrCaps->dwNeededSize;
		FreeMem ( m_lpAddrCaps);
        m_lpAddrCaps = NULL;
    }    

}// CTapiAddress::GetAddressCaps

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::GetAddressStatus
//
// Return the address status for this object
//
const LPLINEADDRESSSTATUS CTapiAddress::GetAddressStatus(BOOL fForceRealloc)
{   
    // If the line hasn't been opened yet, then don't allow this call to
    // continue - it requires an hLine.
    if (!m_pLine->IsOpen())
        return NULL;
    
    // re-retrieve our status record.
	if (m_lpAddrStatus && !fForceRealloc)
		return m_lpAddrStatus;
    
    // Allocate a buffer for the call information
    DWORD dwSize = (m_lpAddrStatus) ? m_lpAddrStatus->dwTotalSize : sizeof (LINEADDRESSSTATUS) + 1024;
    while (TRUE)
    {   
		if (m_lpAddrStatus == NULL)
		{
			m_lpAddrStatus = (LPLINEADDRESSSTATUS) AllocMem( dwSize);
			if (m_lpAddrStatus == NULL)
				return NULL;
		}
        
        // Mark the size we are sending.
        ((LPVARSTRING)m_lpAddrStatus)->dwTotalSize = dwSize;
        if (lineGetAddressStatus (m_pLine->GetLineHandle(), 
                                  GetAddressID(), m_lpAddrStatus) != 0)
            return NULL;

        // Return the structure if we got it all.
        if (m_lpAddrStatus->dwNeededSize <= dwSize)
			return m_lpAddrStatus;

        // If we didn't get it all, then reallocate the buffer and retry it.
        dwSize = m_lpAddrStatus->dwNeededSize;
		FreeMem ( m_lpAddrStatus);
        m_lpAddrStatus = NULL;
    }    

}// CTapiAddress::GetAddressStatus

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::GetID
//
// Return the device handle for the specified identifier on this address.
//
DWORD CTapiAddress::GetID(LPVARSTRING lpDeviceID, LPCTSTR lpszDeviceClass)
{
    if (!m_pLine->IsOpen())
        return LINEERR_NODEVICE;
    return lineGetID (m_pLine->GetLineHandle(), GetAddressID(), NULL, 
					  LINECALLSELECT_ADDRESS, lpDeviceID, lpszDeviceClass);

}// CTapiAddress::GetID

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::OnStateChange
//
// This is invoked when the status of the address has changed.
//
void CTapiAddress::OnStateChange (DWORD dwState)
{
	if (dwState & LINEADDRESSSTATE_CAPSCHANGE)
	{
		if (m_lpAddrCaps)
			GetAddressCaps(0, 0, TRUE);
		dwState &= ~LINEADDRESSSTATE_CAPSCHANGE;
	}
	
	if (dwState)
	{
		if (m_lpAddrStatus)
			GetAddressStatus(TRUE);
	}

}// CTapiAddress::OnStateChange

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::GetDialableAddress
//
// Return the address in a dialable form.
//
CString CTapiAddress::GetDialableAddress()
{
    LPLINEADDRESSCAPS lpAddrCaps = GetAddressCaps();
    CString strAddress = TAPISTR_NOADDRNAME;
    if (lpAddrCaps)
    {
        if (lpAddrCaps->dwAddressSize && lpAddrCaps->dwAddressOffset)
        {
            LPCSTR lpszAddress = ((LPCSTR)lpAddrCaps)+lpAddrCaps->dwAddressOffset;
            strAddress = lpszAddress;
        }
    }
    
    return strAddress;

}// CTapiAddress::GetDialableAddress

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::DevSpecific
//
// Enables service providers to provide access to features not offered 
// by other TAPI functions. The meaning of the extensions are device specific, 
// and taking advantage of these extensions requires the application to be 
// fully aware of them.
//
LONG CTapiAddress::DevSpecific (LPVOID lpParams, DWORD dwSize)
{                            
    return ManageAsynchRequest(lineDevSpecific (m_pLine->GetLineHandle(), GetAddressID(), 
                                    NULL, lpParams, dwSize));

}// CTapiAddress::DevSpecific

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::GetNewCalls
//
// This function returns all the active call handles for this address in an 
// object list.
//
LONG CTapiAddress::GetNewCalls (CObList& lstCalls)
{                         
    DWORD dwSize = sizeof (LINECALLLIST) + 1024;
    LPLINECALLLIST lpCallList = NULL;
    
    while (TRUE)
    {
		lpCallList = (LPLINECALLLIST) AllocMem ( dwSize);
        if (lpCallList == NULL)
			return LINEERR_NOMEM;
            
		LONG lResult = lineGetNewCalls (m_pLine->GetLineHandle(), GetAddressID(), 
                             LINECALLSELECT_LINE, lpCallList);
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
    LPHCALL lphCall = (LPHCALL) ((LPSTR)lpCallList + lpCallList->dwCallsOffset);
    for (DWORD i = 0; i < lpCallList->dwCallsNumEntries; i++)
    {
        CTapiCall* pCall = m_pLine->CreateNewCall(*lphCall);
        if (pCall)
            lstCalls.AddTail(pCall);
        lphCall++;
    }        
    
	FreeMem(lpCallList);
    return 0;
    
}// CTapiAddress::GetNewCalls

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress:SetNumRings
//
// Sets the number of rings that must occur before an incoming call is answered. 
// This function can be used to implement a toll-saver-style function. 
// It allows multiple independent applications to each register the number 
// of rings. The function GetNumRings returns the minimum number of all 
// number of rings requested.  It can be used by the application that answers 
// inbound calls to determine the number of rings it should wait before 
// answering the call.
//
LONG CTapiAddress::SetNumRings (DWORD dwRings)
{                           
    return lineSetNumRings (m_pLine->GetLineHandle(), GetAddressID(), dwRings);

}// CTapiAddress:SetNumRings

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::GetNumRings
//
// Determines the number of rings an inbound call on the given address 
// should ring prior to answering the call. 
//
LONG CTapiAddress::GetNumRings (LPDWORD lpdwNumRings)
{                            
    return lineGetNumRings (m_pLine->GetLineHandle(), GetAddressID(), lpdwNumRings);

}// CTapiAddress::GetNumRings
    
////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::Pickup
//
// Picks up a call alerting at the specified destination address and returns a 
// call handle for the picked-up call.  If invoked with NULL for the 
// lpszDestAddress parameter, a group pickup is performed. If required 
// by the device, lpszGroupID specifies the group ID to which the 
// alerting station belongs.
//    
LONG CTapiAddress::Pickup (CTapiCall** pCall, LPCTSTR lpszDestAddr, LPCTSTR lpszGroupID)
{                       
    HCALL hCall = NULL;
    *pCall = NULL;
    LONG lResult = ManageAsynchRequest(
		linePickup (m_pLine->GetLineHandle(), GetAddressID(),
                               &hCall, lpszDestAddr, lpszGroupID));
    if (!GetTAPIConnection()->WaitForReply(lResult))
		*pCall = m_pLine->CreateNewCall(hCall);
    return lResult;
    
}// CTapiAddress::Pickup

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::SetMediaControl
//
// Enables and disables control actions on the media stream associated 
// with the specified address. Media control actions can be triggered 
// by the detection of specified digits, media modes, custom tones, 
// and call states.
//
LONG CTapiAddress::SetMediaControl (LPLINEMEDIACONTROLDIGIT const lpDigitList, 
                                 DWORD dwDigitNumEntries, 
                                 LPLINEMEDIACONTROLMEDIA const lpMediaList, 
                                 DWORD dwMediaNumEntries, 
                                 LPLINEMEDIACONTROLTONE const lpToneList, 
                                 DWORD dwToneNumEntries, 
                                 LPLINEMEDIACONTROLCALLSTATE const lpCallStateList,
                                 DWORD dwCallStateNumEntries)
{                             
    return lineSetMediaControl (m_pLine->GetLineHandle(), GetAddressID(),
                               NULL, LINECALLSELECT_ADDRESS,
                               lpDigitList, dwDigitNumEntries,
                               lpMediaList, dwMediaNumEntries,
                               lpToneList, dwToneNumEntries,
                               lpCallStateList, dwCallStateNumEntries);
    
}// CTapiAddress::SetMediaControl

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::SetTerminal
//
// Enables an application to specify which terminal information related to the 
// specified address is to be routed. lineSetTerminal can be used while calls 
// are in progress on the line to allow an application to route these events 
// to different devices as required.
//
LONG CTapiAddress::SetTerminal (DWORD dwTerminalMode, DWORD dwTerminalID, BOOL fEnable)
{                         
    return ManageAsynchRequest(
		lineSetTerminal (m_pLine->GetLineHandle(), GetAddressID(), 
                                    NULL, LINECALLSELECT_ADDRESS,
                                    dwTerminalMode, dwTerminalID, (DWORD)fEnable));
}// CTapiAddress::SetTerminal

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::Unpark
//
// Retrieves the call parked at the specified address and returns a call 
// handle for it.
//
LONG CTapiAddress::Unpark (CTapiCall** pCall, LPCTSTR lpszDestAddr)
{                       
    HCALL hCall = NULL;
    *pCall = NULL;
    
    LONG lResult = ManageAsynchRequest(
		lineUnpark (m_pLine->GetLineHandle(), GetAddressID(), 
                               &hCall, lpszDestAddr));
    if (!GetTAPIConnection()->WaitForReply(lResult))
        *pCall = m_pLine->CreateNewCall(hCall);
    return lResult;                                

}// CTapiAddress::Unpark

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::GetAgentCaps
//
// Return the address agent capabilities for this address object
//
const LPLINEAGENTCAPS CTapiAddress::GetAgentCaps(DWORD dwAPIVersion)
{
	if (m_lpAgentCaps && !m_fAgentCapsReload)
		return m_lpAgentCaps;

	// If there was no passed version, use the negotiated version for
	// the line owner.
	if (dwAPIVersion == 0)
		dwAPIVersion = m_pLine->GetNegotiatedAPIVersion();

    // Allocate a buffer for the address information
    DWORD dwSize = (m_lpAgentCaps) ? m_lpAgentCaps->dwTotalSize : sizeof (LINEAGENTCAPS) + 1024;
    while (TRUE)
    {   
		if (m_lpAgentCaps == NULL)
		{
			m_lpAgentCaps = (LPLINEAGENTCAPS) AllocMem( dwSize);
			if (m_lpAgentCaps == NULL)
				return NULL;
		}
        
        // Mark the size we are sending.
        ((LPVARSTRING)m_lpAgentCaps)->dwTotalSize = dwSize;
		LONG lResult = ManageAsynchRequest(
			lineGetAgentCaps (m_pLine->GetTapiConnection()->GetLineAppHandle(),
                                m_pLine->GetDeviceID(), GetAddressID(),
                                dwAPIVersion, m_lpAgentCaps));
		if (GetTAPIConnection()->WaitForReply(lResult) != 0)
            return NULL;

        // Return the structure if we got it all.
        if (m_lpAgentCaps->dwNeededSize <= dwSize)
		{
			m_fAgentCapsReload = FALSE;
			return m_lpAgentCaps;
		}

        // If we didn't get it all, then reallocate the buffer and retry it.
        dwSize = m_lpAgentCaps->dwNeededSize;
		FreeMem ( m_lpAgentCaps);
        m_lpAgentCaps = NULL;
    }    

}// CTapiAddress::GetAgentCaps

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::GetAgentStatus
//
// Return the agent status for this object
//
const LPLINEAGENTSTATUS CTapiAddress::GetAgentStatus()
{   
    // If the line hasn't been opened yet, then don't allow this call to
    // continue - it requires an hLine.
    if (!m_pLine->IsOpen())
        return NULL;
    
    // re-retrieve our status record.
// Disabled due to TAPI bug..
//	if (m_lpAgentStatus != NULL && !m_fAgentStatsReload)
//		return m_lpAgentStatus;

    // Allocate a buffer for the call information
    DWORD dwSize = (m_lpAgentStatus) ? m_lpAgentStatus->dwTotalSize : sizeof (LINEAGENTSTATUS) + 1024;
    while (TRUE)
    {   
		if (m_lpAgentStatus == NULL)
		{
			m_lpAgentStatus = (LPLINEAGENTSTATUS) AllocMem( dwSize);
			if (m_lpAgentStatus == NULL)
				return NULL;
		}
        
        // Mark the size we are sending.
        ((LPVARSTRING)m_lpAgentStatus)->dwTotalSize = dwSize;
		LONG lResult = ManageAsynchRequest(
			lineGetAgentStatus (m_pLine->GetLineHandle(), 
                                  GetAddressID(), m_lpAgentStatus));
		if (GetTAPIConnection()->WaitForReply(lResult) != 0)
            return NULL;

        // Return the structure if we got it all.
        if (m_lpAgentStatus->dwNeededSize <= dwSize)
		{
			m_fAgentStatsReload = FALSE;
			return m_lpAgentStatus;
		}

        // If we didn't get it all, then reallocate the buffer and retry it.
        dwSize = m_lpAgentStatus->dwNeededSize;
		FreeMem ( m_lpAgentStatus);
        m_lpAgentStatus = NULL;
    }    

}// CTapiAddress::GetAgentStatus

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::OnAgentStateChange
//
// This is invoked when the agent status of the address has changed.
//
void CTapiAddress::OnAgentStateChange (DWORD dwFields, DWORD /*dwState*/)
{
	if (dwFields & LINEAGENTSTATUS_CAPSCHANGE)
	{
		m_fAgentCapsReload = TRUE;
		dwFields &= ~LINEAGENTSTATUS_CAPSCHANGE;
	}

	// Delete our agent status structure.
	if (dwFields != 0)
	{
		m_fAgentStatsReload = TRUE;
	}

}// CTapiAddress::OnAgentStateChange

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::GetAgentGroupList
//
// Return the list of groups available for this address
//
LONG CTapiAddress::GetAgentGroupList(CPtrArray& arrGroups)
{
	LPLINEAGENTGROUPLIST lpGroupList = NULL;

	// Retrieve the agent group entries from TAPI.
    DWORD dwSize = (sizeof(LINEAGENTGROUPENTRY) * 100) + 1024;
    while (TRUE)
    {   
        lpGroupList = (LPLINEAGENTGROUPLIST) AllocMem( dwSize);
        if (lpGroupList == NULL)
            return LINEERR_NOMEM;
        
        // Mark the size we are sending.
        ((LPVARSTRING)lpGroupList)->dwTotalSize = dwSize;
        
        LONG lResult = ManageAsynchRequest(
			lineGetAgentGroupList (m_pLine->GetLineHandle(), GetAddressID(), lpGroupList));
		if (GetTAPIConnection()->WaitForReply(lResult) != 0)
		{
			FreeMem ( lpGroupList);
            lpGroupList = NULL;
            return lResult;
        }

        // Return the structure if we got it all.
        if (lpGroupList->dwNeededSize <= dwSize)
			break;

        // If we didn't get it all, then reallocate the buffer and retry it.
        dwSize = lpGroupList->dwNeededSize;
		FreeMem ( lpGroupList);
        lpGroupList = NULL;
    }    

	// Now break all the data out of the structure.
	LPLINEAGENTGROUPENTRY lpge = (LPLINEAGENTGROUPENTRY) ((LPBYTE)lpGroupList + lpGroupList->dwListOffset);
	for (DWORD dwCount = 0; dwCount < lpGroupList->dwNumEntries; dwCount++)
	{
		LPAGENTGROUP pAG = new AGENTGROUP;
		pAG->GroupID.dwGroupID1 = lpge->GroupID.dwGroupID1;
		pAG->GroupID.dwGroupID2 = lpge->GroupID.dwGroupID2;
		pAG->GroupID.dwGroupID3 = lpge->GroupID.dwGroupID3;
		pAG->GroupID.dwGroupID4 = lpge->GroupID.dwGroupID3;
		pAG->strName = (LPCTSTR)((LPBYTE)lpGroupList + lpge->dwNameOffset);
		arrGroups.Add(pAG);
		lpge++;
	}

	FreeMem( lpGroupList);

	return 0;

}// CTapiAddress::GetAgentGroupList

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::GetAgentActivityList
//
// Return the list of activities associated for an address
//
LONG CTapiAddress::GetAgentActivityList(CPtrArray& arrActivities)
{
	LPLINEAGENTACTIVITYLIST lpActivityList = NULL;

	// Retrieve the agent group entries from TAPI.
    DWORD dwSize = (sizeof(LINEAGENTACTIVITYENTRY) * 100) + 1024;
    while (TRUE)
    {   
        lpActivityList = (LPLINEAGENTACTIVITYLIST) AllocMem( dwSize);
        if (lpActivityList == NULL)
            return LINEERR_NOMEM;
        
        // Mark the size we are sending.
        ((LPVARSTRING)lpActivityList)->dwTotalSize = dwSize;
        
        LONG lResult = ManageAsynchRequest(
			lineGetAgentActivityList (m_pLine->GetLineHandle(), GetAddressID(), lpActivityList));
		if (GetTAPIConnection()->WaitForReply(lResult) != 0)
		{
			FreeMem ( lpActivityList);
            lpActivityList = NULL;
            return lResult;
        }

        // Return the structure if we got it all.
        if (lpActivityList->dwNeededSize <= dwSize)
			break;

        // If we didn't get it all, then reallocate the buffer and retry it.
        dwSize = lpActivityList->dwNeededSize;
		FreeMem ( lpActivityList);
        lpActivityList = NULL;
    }    

	// Now break all the data out of the structure.
	LPLINEAGENTACTIVITYENTRY lpae = (LPLINEAGENTACTIVITYENTRY) ((LPBYTE)lpActivityList + lpActivityList->dwListOffset);
	for (DWORD dwCount = 0; dwCount < lpActivityList->dwNumEntries; dwCount++)
	{
		LPAGENTACTIVITY pAE = new AGENTACTIVITY;
		pAE->dwActivityID = lpae->dwID;
		pAE->strName = (LPCTSTR)((LPBYTE)lpActivityList + lpae->dwNameOffset);
		arrActivities.Add(pAE);
		lpae++;
	}

	FreeMem( lpActivityList);

	return 0;

}// CTapiAddress::GetAgentActivityList

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::SetAgentActivity
// 
// Change the agent activity 
//
LONG CTapiAddress::SetAgentActivity(DWORD dwActivityID)
{
    return ManageAsynchRequest(
		lineSetAgentActivity (m_pLine->GetLineHandle(), GetAddressID(), dwActivityID));

}// CTapiAddress::SetAgentActivity

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::GetAgentState
//
// Return the current agent state
//
DWORD CTapiAddress::GetAgentState()
{
	LPLINEAGENTSTATUS lpAgentStatus = GetAgentStatus();
	if (lpAgentStatus)
		return lpAgentStatus->dwState;
	return LINEAGENTSTATE_UNKNOWN;

}// CTapiAddress::GetAgentState

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::SetAgentState
//
// Change the agent state
//
LONG CTapiAddress::SetAgentState(DWORD dwState, DWORD dwNextState)
{
    return ManageAsynchRequest(
		lineSetAgentState(m_pLine->GetLineHandle(), GetAddressID(), dwState, dwNextState));

}// CTapiAddress::SetAgentState

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::SetAgentGroup
//
// Change the agent groups
//
LONG CTapiAddress::SetAgentGroup(CPtrArray& arrGroups)
{
	LPLINEAGENTGROUPLIST lpgl = NULL;

	// Create our group list.
	if (arrGroups.GetSize() > 0)
	{
		DWORD dwSize = sizeof (LINEAGENTGROUPLIST) + 
				(arrGroups.GetSize() * (sizeof(LINEAGENTGROUPENTRY) + 512));
		lpgl = (LPLINEAGENTGROUPLIST) AllocMem ( dwSize);
		if (lpgl == NULL)
			return LINEERR_NOMEM;

		lpgl->dwTotalSize = 
		lpgl->dwNeededSize =
		lpgl->dwUsedSize = dwSize;
		lpgl->dwNumEntries = arrGroups.GetSize();
		lpgl->dwListSize = sizeof(LINEAGENTGROUPENTRY) * lpgl->dwNumEntries;
		lpgl->dwListOffset = sizeof(LINEAGENTGROUPLIST);

		// Fill in the group entry structures.
		LPLINEAGENTGROUPENTRY lpge = (LPLINEAGENTGROUPENTRY) ((LPBYTE)lpgl + lpgl->dwListOffset);
		LPTSTR pszName = (LPTSTR)((LPBYTE)lpge + lpgl->dwListSize);
		for (int i = 0; i < arrGroups.GetSize(); i++)
		{
			LPAGENTGROUP lpGroup = (LPAGENTGROUP) arrGroups[i];
			lpge->GroupID.dwGroupID1 = lpGroup->GroupID.dwGroupID1;
			lpge->GroupID.dwGroupID2 = lpGroup->GroupID.dwGroupID2;
			lpge->GroupID.dwGroupID3 = lpGroup->GroupID.dwGroupID3;
			lpge->GroupID.dwGroupID4 = lpGroup->GroupID.dwGroupID4;
			lpge->dwNameSize = (lpGroup->strName.GetLength()+1) * sizeof(TCHAR);
			lpge->dwNameOffset = ((DWORD)pszName - (DWORD)lpgl); 
			lstrcpy(pszName, lpGroup->strName);
			pszName += (lpGroup->strName.GetLength()+1);
			lpge++;
		}
	}

	// Set the agent group to the group list.
	LONG lResult = GetTAPIConnection()->WaitForReply(ManageAsynchRequest(
		lineSetAgentGroup (m_pLine->GetLineHandle(), GetAddressID(), lpgl)));
	
	if (lpgl != NULL)
		FreeMem(lpgl);

	return lResult;

}// CTapiAddress::SetAgentGroup

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::SupportsAgents
//
// Returns TRUE/FALSE whether agents are supported on this address
//
BOOL CTapiAddress::SupportsAgents() const
{
	static LINEAGENTSTATUS las;
	las.dwTotalSize = sizeof(LINEAGENTSTATUS);

	// First, open the line using our own handle.
	if (!GetLineOwner()->IsOpen())
	{
		HLINE hLine;
		if (lineOpen(GetTAPIConnection()->GetLineAppHandle(), 
				GetLineOwner()->GetDeviceID(),
                             &hLine, TAPIVER_20, 0,
                             (DWORD)NULL, LINECALLPRIVILEGE_NONE,
							 LINEMEDIAMODE_UNKNOWN, NULL) == 0)
		{
			LONG lResult = GetTAPIConnection()->WaitForReply(
				lineGetAgentStatus(hLine, GetAddressID(), &las));
			lineClose(hLine);
			if (lResult != LINEERR_OPERATIONUNAVAIL)
				return TRUE;
		}
	}
	else
	{
		LONG lResult = lineGetAgentStatus(m_pLine->GetLineHandle(), GetAddressID(), &las);
		if (lResult != LINEERR_OPERATIONUNAVAIL)
			return TRUE;
	}
	return FALSE;

}// CTapiAddress::SupportsAgents

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::GetCurrentAgentGroupList
//
// Returns the current agent groups
//
LONG CTapiAddress::GetCurrentAgentGroupList(CPtrArray& arrGroups)
{
	LPLINEAGENTSTATUS lpStatus = GetAgentStatus();
	if (lpStatus == NULL || lpStatus->dwNumEntries == 0)
		return LINEERR_INVALAGENTGROUP;

	// Retrieve the agent group entries from TAPI.
	if (lpStatus != NULL &&
		lpStatus->dwGroupListSize > 0 &&
		lpStatus->dwGroupListOffset > 0)
	{
		LPLINEAGENTGROUPENTRY lpge = (LPLINEAGENTGROUPENTRY) ((LPBYTE)lpStatus + lpStatus->dwGroupListOffset);
		for (DWORD dwCount = 0; dwCount < lpStatus->dwNumEntries; dwCount++)
		{
			LPAGENTGROUP pAG = new AGENTGROUP;
			pAG->GroupID.dwGroupID1 = lpge->GroupID.dwGroupID1;
			pAG->GroupID.dwGroupID2 = lpge->GroupID.dwGroupID2;
			pAG->GroupID.dwGroupID3 = lpge->GroupID.dwGroupID3;
			pAG->GroupID.dwGroupID4 = lpge->GroupID.dwGroupID3;
			pAG->strName = (LPCTSTR)((LPBYTE)lpStatus + lpge->dwNameOffset);
			arrGroups.Add(pAG);
			lpge++;
		}
	}

	return 0;

}// CTapiAddress::GetCurrentAgentGroupList

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress::GetValidIDs
//
// Return a list of valid keys for lineGetID
//
void CTapiAddress::GetValidIDs(CStringArray& arrKeys) const
{
	LPLINEADDRESSCAPS lpAddrCaps = ((CTapiAddress*)this)->GetAddressCaps();
	if (lpAddrCaps && lpAddrCaps->dwDeviceClassesSize > 0)
	{
		LPCTSTR pszKey = (LPCTSTR)((LPBYTE)lpAddrCaps) + lpAddrCaps->dwDeviceClassesOffset;
		while (*pszKey)
		{
			arrKeys.Add(pszKey);
			pszKey += lstrlen(pszKey)+1;
		}
	}

}// CTapiAddress::GetValidIDs
