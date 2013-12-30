//
//	Last Modified: $Date: 2010-07-22 10:16:36 $
//
//	$Log: Atapi.h,v $
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

// ATAPI.H
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

#ifndef __ATAPI_INC__
#define __ATAPI_INC__

#include "STDAFX.H"

#ifndef TAPI_H
#include <tapi.h>
#endif

// Pre-definitions
class CTapiObject;
class CTapiLine;
class CTapiPhone;
class CTapiCall;
class CTapiAddress;
class CTapiConnection;
class CTapiRequest;

////////////////////////////////////////////////////////////////////////////////////
// Globals and constants

#define TAPIVER_13 (0x00010003)     // First released version of TAPI (Win3.1)
#define TAPIVER_14 (0x00010004)     // Latest 16-bit version (Chicago)
#define TAPIVER_20 (0x00020000)     // Initial 32-bit version (NT)
#define TAPIVER_21 (0x00020001)     // NT/Win98
#define TAPIVER_22 (0x00020002)     // SP2
#define TAPIVER_30 (0x00030000)     // Win2K
#define TAPIVER_31 (0x00030001)     // WinXP

/////////////////////////////////////////////////////////////////////////////////
// TAPIPROVIDER
//
// This structure describes a TAPI service provider.  This is returned
// in the provider enumeration APIs.
//

typedef struct _tagTAPIPROVIDER
{                                    
	DWORD   dwPermanentProviderID;      // Unique provider ID from TAPI
	CString strProviderName;            // Name of service provider

} TAPIPROVIDER, FAR * LPTAPIPROVIDER;

//////////////////////////////////////////////////////////////////////////////////
// ButtonInfo
//
// Button structure
//

typedef struct _tagButtonInfo
{
	DWORD dwButtonMode;
    DWORD dwButtonFunction;
    DWORD dwButtonState;
	CString strButtonText;

} BUTTONINFO, FAR * LPBUTTONINFO;

//////////////////////////////////////////////////////////////////////////////////
// AgentGroup
//
// This structure describes a single agent group
//

typedef struct _tagAgentGroup
{
    struct
    {
        DWORD dwGroupID1;
        DWORD dwGroupID2;
        DWORD dwGroupID3;
        DWORD dwGroupID4;
    } GroupID;

	CString strName;

} AGENTGROUP, FAR * LPAGENTGROUP;

//////////////////////////////////////////////////////////////////////////////////
// AgentActivity
//
// This structure describes a single agent activity
//

typedef struct _tagAgentActivity
{
	DWORD dwActivityID;
	CString strName;

} AGENTACTIVITY, FAR * LPAGENTACTIVITY;

//////////////////////////////////////////////////////////////////////////////////
// Global functions

LONG ManageRequest (LONG lRequestID);
CTapiConnection* GetTAPIConnection();

////////////////////////////////////////////////////////////////////////////////////
// CTapiObject
//
// Basic Telephony Object
//
class CTapiObject : public CObject
{
    DECLARE_DYNCREATE (CTapiObject)

// Class data
protected:
    DWORD m_dwUserData;     // User data value

// Constructor
public:
    CTapiObject();
    virtual ~CTapiObject();

// Access methods
public:
    DWORD GetUserData() const;
    void* GetUserDataPtr() const;
    void SetUserData(DWORD dwData);
    void SetUserDataPtr (void* ptrValue);
};

////////////////////////////////////////////////////////////////////////////////////
// CTapiRequest
//
// This represents an asynchronous request for TAPI
//

class CTapiRequest : public CObject
{
// Class data
public:
    DWORD m_dwRequestID;            // Request ID assigned by TAPI
    DWORD m_dwCompleteTime;         // Completion time
    volatile LONG m_lResult;        // Final result code
    CEvent m_evtComplete;           // Signalled when request complete.
  
// Constructor
public:
    CTapiRequest(DWORD dwRequestID);
    virtual ~CTapiRequest();

// Methods
public:
    BOOL IsPending();
    void OnRequestComplete (LONG lResult);
    LONG GetResult (LONG lTimeout=0);
};

//////////////////////////////////////////////////////////////////////////////////
// CTapiConnection
//
// This class defines the basic TAPI connection.  All applications desiring
// use of telephony must establish a connection to the TAPI system.  This is
// done through a callback function and opaque handle contained within this class.
//

class CTapiConnection
{ 
// Class data
protected:
    HLINEAPP        m_hLineApp;             // Line connection
	HPHONEAPP		m_hPhoneApp;			// Phone connection
    DWORD           m_dwNumLines;           // Number of line devices in system.
	DWORD           m_dwNumPhones;			// Number of phone devices in system.
    CCriticalSection m_semLines;            // Synch access to lines
    CObArray        m_arrLines;             // Array of CTapiLine objects found
	CCriticalSection m_semPhones;			// Synch access to phones
	CObArray		m_arrPhones;			// Array of CTapiPhone objects found
    // These allow dynamic creation of new object types
    CRuntimeClass*  m_pLineClass;           // Runtime class for CTapiLine
    CRuntimeClass*  m_pCallClass;           // Runtime class for CTapiCall
    CRuntimeClass*  m_pAddrClass;           // Runtime class for CTapiAddress
	CRuntimeClass*  m_pPhoneClass;			// Runtime class for CTapiPhone
    CCriticalSection m_semProviders;        // Synch access to provider list
    int             m_iProviderPos;         // Position in below array
    CPtrArray       m_arrProviders;         // Temp array used for provider enum.
    CCriticalSection m_semRequest;			// Synch access to request list
    CObList m_arrWaitingRequests;			// Pending requests
	CWinThread* m_pMonitorThread_L;			// TAPI line event processor thread
	CWinThread* m_pMonitorThread_P;			// TAPI phone event processor thread
	HANDLE m_hTapiEvent_L;					// TAPI line event handle
	HANDLE m_hTapiEvent_P;					// TAPI phone event handle
    
// Constructor
public:
    CTapiConnection();
    virtual ~CTapiConnection();

// Access functions
public:
    LONG Init(LPCTSTR pszAppName, CRuntimeClass* prtLine=NULL, CRuntimeClass* prtAddr=NULL, CRuntimeClass* prtCall=NULL, CRuntimeClass* prtPhone=NULL, DWORD dwAPIVersion=TAPI_CURRENT_VERSION);
    LONG Shutdown();

    HLINEAPP GetLineAppHandle() const;
	HPHONEAPP GetPhoneAppHandle() const;

	// Line access
    unsigned int GetLineDeviceCount() const;
    CTapiLine* GetLineFromDeviceID(DWORD dwDeviceID) const;
    LONG OpenLine (CTapiLine** pLine, DWORD dwPrivileges, DWORD dwMediaModes, DWORD dwAPIVersion, DWORD dwExtVersion, LPLINECALLPARAMS const lpCallParams);

	// Phone access
	unsigned int GetPhoneDeviceCount() const;
	CTapiPhone* GetPhoneFromDeviceID(DWORD dwDeviceID) const;

    // Provider enumeration functions
    BOOL GetFirstProvider (LPTAPIPROVIDER lpProvider);
    BOOL GetNextProvider (LPTAPIPROVIDER lpProvider);

    // Misc. functions.
    LONG GetTranslateCaps (LPLINETRANSLATECAPS lpTranslateCaps, DWORD dwTapiVersion=TAPIVER_14);
    LONG SetCurrentLocation (DWORD dwLocation);

	unsigned int GetPendingRequestCount();
	void StopWaitingForAllRequests();
    CTapiRequest* LocateRequest (DWORD dwRequestID);
    CTapiRequest* AddRequest (DWORD dwRequestID);
	LONG WaitForReply(DWORD dwRequestID, LONG lTimeout=60000);
	void PurgeRequests (LONG lmSec);

	// Internal event procs
	void LineEventProc();
	void PhoneEventProc();

// Internal functions
public:
    void OnLineCreate (DWORD dwDeviceID);
    void OnLineRemove (DWORD dwDeviceID);
	void OnLineClose (DWORD dwDeviceID);
    void OnPhoneCreate (DWORD dwDeviceID);
    void OnPhoneRemove (DWORD dwDeviceID);
	void OnPhoneClose (DWORD dwDeviceID);
    void OnRequestComplete (DWORD dwRequestID, LONG lResult);

    friend class CTapiLine;
	friend class CTapiPhone;
    friend class CTapiRequest;

// Internal functions
protected:
	LONG InitLines(LPCTSTR pszAppName, DWORD dwAPIVersion);
	LONG InitPhones(LPCTSTR pszAppName, DWORD dwAPIVersion);
};

////////////////////////////////////////////////////////////////////////////////////
// CTapiLine
//
// This object encapsulates a line device in our TAPI world.  Each line device
// is owned by a CTapiConnection.
//

class CTapiLine : public CTapiObject
{ 
    DECLARE_DYNCREATE (CTapiLine)
// Class data
protected:
	enum {
		Removed = 0x01					// Line is not valid and has been removed.
	};

	int					m_iFlags;		// Flags for this line.
    CTapiConnection*    m_pConn;        // TAPI connection owner
    CCriticalSection    m_semAddress;   // Synch. access to array
    CObArray            m_arrAddress;   // Addresses on this line
    CCriticalSection    m_semCalls;     // Synch access to array
    CObArray            m_arrCalls;     // List of known CTapiCall objects
    HLINE               m_hLine;        // Opaque Line handle
    DWORD               m_dwDeviceID;   // Device ID (0-num Devices)
    DWORD               m_dwAPIVersion; // API version negotiated with TAPI
    LPLINEDEVCAPS       m_lpLineCaps;   // Cached line capabilities
    LPLINEDEVSTATUS     m_lpLineStatus; // Last read line status
    
// Constructor
public:
    CTapiLine();
    virtual ~CTapiLine();
    
// Access methods
public:
    const LPLINEDEVCAPS GetLineCaps(DWORD dwAPIVersion=0, DWORD dwExtVersion=0, BOOL fForceRealloc=FALSE);
    const LPLINEDEVSTATUS GetLineStatus();

    DWORD GetDeviceID() const;
    DWORD GetNegotiatedAPIVersion() const;
    CString GetLineName() const;
    CString GetProviderInfo() const;
    CString GetSwitchInfo() const;
    CTapiConnection* GetTapiConnection() const;

    // Methods to query the currently known calls
    int GetCallCount();
    CTapiCall* GetCall (int iIndex);
	CTapiCall* FindCall(DWORD dwState, DWORD dwFeature=0xffffffff);

    // Open the line device
    LONG Open(DWORD dwPriv, DWORD dwMediaMode, DWORD dwAPIVersion=0, DWORD dwExtVersion=0, LPLINECALLPARAMS const lpCallParams=NULL);
    BOOL IsOpen() const;
	BOOL IsValid() const;
    HLINE GetLineHandle() const;
    LONG Close();

    // Place a call
    LONG MakeCall (CTapiCall** pCall, LPCTSTR lpszDestAddr, DWORD dwCountry=0, LPLINECALLPARAMS const lpCallParams=NULL);
    void DeallocateCall (CTapiCall* pCall);
    CTapiCall* CreateNewCall (HCALL hCall);
    
    // Status message notifications
    LONG GetStatusMessages (LPDWORD dwLineStatus, LPDWORD lpdwAddressStates);
    LONG SetStatusMessages (DWORD dwLineStates, DWORD dwAddressStates);
    
    // Retrieve line handle information
    LONG GetID (DWORD dwAddressID, LPVARSTRING lpDeviceID, LPCTSTR lpszDeviceClass);
    LONG GetID (LPVARSTRING lpDeviceID, LPCTSTR lpszDeviceClass);

    // Manage configuration
    LONG Config (CWnd* pwndOwner, LPCTSTR lpszDeviceClass);
    LONG ConfigEdit (CWnd* pwndOwner, LPCTSTR lpszDeviceClass, LPVOID const lpDeviceConfigIn, DWORD dwSize, LPVARSTRING lpDeviceConfigOut);
    LONG DevSpecific (DWORD dwFeature, LPVOID lpParams=NULL, DWORD dwSize=0L);
    LONG GetDevConfig (LPVOID lpBuff, DWORD dwSize, LPCTSTR lpszDeviceClass);
    LONG SetDevConfig (LPVOID const lpBuff, DWORD dwSize, LPCTSTR lpszDeviceClass);
	LONG SetDeviceStatus (DWORD dwDevStatus, BOOL fSet=TRUE);
    LONG GetNewCalls (CObList& lstCalls);
    
    // Misc. functions
    BOOL GetTSPProvider (LPTAPIPROVIDER pProvider) const;
	DWORD GetProviderID() const;
	void GetValidIDs(CStringArray& arrKeys) const;
    HICON GetIcon(LPCTSTR lpszDeviceClass = NULL);
    LONG SwapHold (CTapiCall* pCall, CTapiCall* pCall2);
    LONG Forward (DWORD dwAddress, LPLINEFORWARDLIST const lpForwardList, DWORD dwNumRingsNoAnswer, CTapiCall** pConsCall, LPLINECALLPARAMS const lpCallParams=NULL);
    LONG CancelForward (DWORD dwAddress);
    LONG SetMediaControl (LPLINEMEDIACONTROLDIGIT const lpDigitList=NULL, DWORD dwDigitNumEntries=0, LPLINEMEDIACONTROLMEDIA const lpMediaList=NULL, DWORD dwMediaNumEntries=0, 
                          LPLINEMEDIACONTROLTONE const lpToneList=NULL, DWORD dwToneNumEntries=0, LPLINEMEDIACONTROLCALLSTATE const lpCallStateList=NULL, DWORD dwCallStateNumEntries=0);
    LONG SetTerminal (DWORD dwTerminalMode, DWORD dwTerminalID, BOOL fEnable);
    LONG SetTollList (LPCTSTR lpszAddressIn, DWORD dwTollListOption);
    LONG UncompleteCall (DWORD dwCompletionID);
    LONG SetupConference (CTapiCall** pConfCall, CTapiCall** pConstCall, DWORD dwNumParties, LPLINECALLPARAMS const lpCallParams = NULL);
	DWORD GetRelatedPhoneID();    

    // Address manipulation
    CTapiAddress* GetAddress(DWORD dwAddr);
    CTapiAddress* GetAddress(LPCTSTR lpszAddr, DWORD dwSize=0, DWORD dwMode=LINEADDRESSMODE_DIALABLEADDR);
    DWORD GetAddressCount() const;
    LONG TranslateAddress (LPCTSTR lpszAddressIn, DWORD dwCard, DWORD dwTranslateOptions, LPLINETRANSLATEOUTPUT lpTranslateOutput);
    LONG TranslateDialog (CWnd* pwndOwner, LPCTSTR lpszAddressIn=NULL);

public:
    void LineCallback (DWORD hDevice, DWORD dwMsg, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
    CTapiCall* GetCallFromHandle(HCALL hCall);

protected:    
    void Init (CTapiConnection* pConn, DWORD dwDeviceID);
    void GatherLineCapabilities();
    void DeleteCallAppearances();
    void GatherAddressInformation();
    void RemoveCall(CTapiCall* pCall);
    
// Overriable methods for notifications
protected:
    virtual void OnAddressStateChange (DWORD dwAddressID, DWORD dwState);
	virtual void OnAgentStateChange (DWORD dwAddressID, DWORD dwFields, DWORD dwState);
    virtual void OnCallInfoChange (HCALL hCall, DWORD dwCallInfoState);
    virtual void OnCallStateChange (HCALL hCall, DWORD dwState, DWORD dwStateDetail, DWORD dwPrivilage);
    virtual void OnClose();
    virtual void OnDevSpecific (DWORD dwHandle, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
    virtual void OnDevSpecificFeature (DWORD dwHandle, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
    virtual void OnGatherDigitsComplete (HCALL hCall, DWORD dwReason);
    virtual void OnGenerateComplete (HCALL hCall, DWORD dwReason);
    virtual void OnDeviceStateChange (DWORD dwDeviceState, DWORD dwStateDetail1, DWORD dwStateDetail2);
    virtual void OnDigitDetected (HCALL hCall, DWORD dwDigit, DWORD dwDigitMode);
    virtual void OnCallMediaModeChange (HCALL hCall, DWORD dwMediaMode);
    virtual void OnToneDetected (HCALL hCall, DWORD dwAppSpecific);
    virtual void OnNewCall (CTapiCall* pCall);
	virtual void OnDynamicCreate();
	virtual void OnDynamicRemove();
	virtual void OnForceClose();

// Friend definitions
    friend class CTapiConnection;
    friend class CTapiCall;    
    friend class CTapiAddress;
};

////////////////////////////////////////////////////////////////////////////////////
// CTapiAddress
//
// This is the address object class.  In TAPI, call appearances are dynamic,
// but addresses are fixed.  There may be multiple addresses per line, for
// example - distinctive ring on an analog system.
//

class CTapiAddress : public CTapiObject
{   
    DECLARE_DYNCREATE (CTapiAddress)

// Class data
private:                                         
    CTapiLine* m_pLine;         // Line owner
    DWORD   m_dwAddressID;      // Address id assigned by TAPI
    CString m_strAddress;       // Printable address (phone #).
    LPLINEADDRESSCAPS m_lpAddrCaps;		// Last read address caps
    LPLINEADDRESSSTATUS m_lpAddrStatus; // Last read address status
	LPLINEAGENTCAPS m_lpAgentCaps;		// Last read agent caps
	LPLINEAGENTSTATUS m_lpAgentStatus;	// Last read agent status
	BOOL m_fAgentStatsReload;	// Reload AgentStatus on next interval
	BOOL m_fAgentCapsReload;	// Reload AgentCaps on next interval
    
// Constructor
public:
    CTapiAddress();
    virtual ~CTapiAddress();
    
// Access methods
public:
    const LPLINEADDRESSCAPS GetAddressCaps(DWORD dwAPIVersion=0, DWORD dwExtVersion=0, BOOL fForceRealloc=FALSE);
    const LPLINEADDRESSSTATUS GetAddressStatus(BOOL fForceRealloc=FALSE);
	const LPLINEAGENTCAPS GetAgentCaps(DWORD dwAPIVersion=0);
	const LPLINEAGENTSTATUS GetAgentStatus();

    CTapiLine* GetLineOwner() const;
    DWORD GetAddressID() const;
    DWORD GetID(LPVARSTRING lpDeviceID, LPCTSTR lpszDeviceClass);
	void GetValidIDs(CStringArray& arrKeys) const;
    CString GetDialableAddress();
    LONG DevSpecific (LPVOID lpParams, DWORD dwSize);
    LONG GetNewCalls (CObList& lstCalls);
    LONG GetNumRings (LPDWORD lpdwNumRings);
    LONG SetNumRings (DWORD dwNumRings);
    LONG Pickup (CTapiCall** pCall, LPCTSTR lpszDestAddr=NULL, LPCTSTR lpszGroupID=NULL);
    LONG SetMediaControl (LPLINEMEDIACONTROLDIGIT const lpDigitList=NULL, DWORD dwDigitNumEntries=0, LPLINEMEDIACONTROLMEDIA const lpMediaList=NULL, DWORD dwMediaNumEntries=0, 
                          LPLINEMEDIACONTROLTONE const lpToneList=NULL, DWORD dwToneNumEntries=0, LPLINEMEDIACONTROLCALLSTATE const lpCallStateList=NULL, DWORD dwCallStateNumEntries=0);
    LONG SetTerminal (DWORD dwTerminalMode, DWORD dwTerminalID, BOOL fEnable);
    LONG Unpark (CTapiCall** pCall, LPCTSTR lpszDestAddr);

	// Agent functions
	BOOL SupportsAgents() const;
	LONG GetAgentGroupList(CPtrArray& arrGroups);
	LONG GetAgentActivityList(CPtrArray& arrActivities);
	LONG SetAgentActivity(DWORD dwActivityID);
	DWORD GetAgentState();
	LONG SetAgentState(DWORD dwState, DWORD dwNextState = 0);
	LONG SetAgentGroup(CPtrArray& arrGroups);
	LONG GetCurrentAgentGroupList(CPtrArray& arrGroups);

// Virtual methods for notifications from line
public:
    virtual void OnStateChange (DWORD dwState);
	virtual void OnAgentStateChange (DWORD dwFields, DWORD dwState);

protected:
    void Init (CTapiLine* pLine, DWORD dwAddrId);
    friend CTapiLine;
};

////////////////////////////////////////////////////////////////////////////////////
// CTapiCall
//
// This is the call object class.  It manages the state of the call appearances
// active on a line.
//

class CTapiCall : public CTapiObject
{   
    DECLARE_DYNCREATE (CTapiCall)
// Class data
private:           
    CTapiLine*  m_pLine;        // Line owner for this call
    HCALL       m_hCall;        // Opaque TAPI call handle
    DWORD       m_dwState;      // Internal state
    LPLINECALLINFO m_lpCallInfo;        // Call information (as of last call to GetCallInfo).
    LPLINECALLSTATUS m_lpCallStatus;    // Call status (as of last call to GetCallStatus).
    
// Constructor
public:
    CTapiCall();
    virtual ~CTapiCall();
                                              
// Access functions
public:
    const LPLINECALLINFO GetCallInfo(BOOL fForceRealloc=FALSE);
    const LPLINECALLSTATUS GetCallStatus();

    LONG Accept(LPCSTR lpszUserToUser=NULL, DWORD dwSize=0);
    LONG AddToConference (CTapiCall* pCall);
    LONG Answer(LPCSTR lpszUserToUser=NULL, DWORD dwSize=0);
    LONG BlindTransfer (LPCTSTR lpszDestAddr, DWORD dwCountryCode=0);
    LONG CompleteCall (LPDWORD lpdwCompletionID, DWORD dwCompletionMode, DWORD dwMessageID);
    LONG CompleteTransfer(CTapiCall* pDestCall, CTapiCall** pConfCall, DWORD dwTransferMode);
    LONG Deallocate();
    LONG DevSpecific (LPVOID lpParams, DWORD dwSize);
    LONG Dial (LPCTSTR lpszDestAddr, DWORD dwCountryCode=0);
    LONG Drop (LPCSTR lpszUserToUser=NULL, DWORD dwSize=0);
    LONG GatherDigits(DWORD dwDigitModes, LPTSTR lpsDigits, DWORD dwNumDigits, LPCTSTR lpszTerminationDigits, DWORD dwFirstDigitTimeout, DWORD dwInterDigitTimeout);
    LONG CancelGatherDigits();
    LONG GenerateDigits (DWORD dwDigitMode, LPCTSTR lpszDigits, DWORD dwDuration=0);
    LONG CancelGenerateDigits();
    LONG GenerateTone (DWORD dwToneMode, DWORD dwDuration=0, DWORD dwNumTones=0, LPLINEGENERATETONE const lpTones=NULL);
    LONG CancelGenerateTone();
    LONG GetConferenceRelatedCalls (CObList& lstCalls);
    LONG GetID (LPVARSTRING lpDeviceID, LPCTSTR lpszDeviceClass);
    LONG Handoff (LPCTSTR lpszFilename, DWORD dwMediaMode=0);
    LONG Hold();
    LONG MonitorDigits (DWORD dwDigitModes);
    LONG CancelMonitorDigits();
    LONG MonitorMedia (DWORD dwMediaModes);
    LONG CancelMonitorMedia();
    LONG MonitorTones (LPLINEMONITORTONE const lpToneList, DWORD dwNumEntries);
    LONG CancelMonitorTones();
    LONG Park (DWORD dwParkMode, LPCTSTR lpszDestAddr, LPTSTR lpszReturnBuff, DWORD dwSize);
    LONG PrepareAddToConference (CTapiCall** pCall, LPLINECALLPARAMS const lpCallParams = NULL);
    LONG Redirect (LPCTSTR lpszDialableAddr, DWORD dwCountryCode=0);
    LONG ReleaseUserUserInfo();
    LONG RemoveFromConference();
    LONG SecureCall();
	LONG SetCallData(LPVOID lpBuff, DWORD dwSize);
	LONG SetQualityOfService(LPVOID lpBuff, DWORD dwSize, LPVOID lpBuff2, DWORD dwSize2);
    LONG SendUserUserInfo (LPCSTR lpszUserToUser, DWORD dwSize);
    LONG SetAppSpecificData (DWORD dwData);
    LONG SetCallParams (DWORD dwBearerMode, DWORD dwMinRate, DWORD dwMaxRate, LPLINEDIALPARAMS const lpDialParams = NULL);
    LONG SetPrivilege (DWORD dwCallPrivilege);
    LONG SetMediaControl (LPLINEMEDIACONTROLDIGIT const lpDigitList=NULL, DWORD dwDigitNumEntries=0, LPLINEMEDIACONTROLMEDIA const lpMediaList=NULL, DWORD dwMediaNumEntries=0, 
                          LPLINEMEDIACONTROLTONE const lpToneList=NULL, DWORD dwToneNumEntries=0, LPLINEMEDIACONTROLCALLSTATE const lpCallStateList=NULL, DWORD dwCallStateNumEntries=0);
    LONG SetMediaMode (DWORD dwMediaMode);
    LONG SetTerminal (DWORD dwTerminalMode, DWORD dwTerminalID, BOOL fEnable);
    LONG SetupConference (CTapiCall** pConfCall, CTapiCall** pConstCall, DWORD dwNumParties, LPLINECALLPARAMS const lpCallParams = NULL);
    LONG SetupTransfer (CTapiCall** pXferCall, LPLINECALLPARAMS const lpCallParams = NULL);
    LONG Unhold();

    // Other functions of use    
    CTapiLine* GetLineOwner() const;
    HCALL GetCallHandle() const;
    DWORD GetCallState();
    CTapiAddress* GetAddressInfo();
    CString GetCallStateString();
	CString GetCallStateModeString();
	bool WaitForFeature(DWORD dwFeature, DWORD dwTimeout);

	// CallerID information
	CString GetCallerIDNumber();
	CString GetCallerIDName();
	CString GetCalledIDNumber();
	CString GetCalledIDName();
	CString GetConnectedIDName();
	CString GetConnectedIDNumber();
	CString GetRedirectingIDNumber();
	CString GetRedirectingIDName();
	CString GetRedirectedFromIDNumber();
	CString GetRedirectedFromIDName();
    
    // These functions return the known GetID handles
    DWORD GetWaveInDeviceID();
    DWORD GetWaveOutDeviceID();
    DWORD GetMidiInDeviceID();
    DWORD GetMidiOutDeviceID();
    HANDLE GetCommHandle();
    CString GetCommDevice();
    
protected:
    void Init (CTapiLine* pLine, HCALL hCall);
    
// Overriable methods for notifications
public:
    virtual void OnInfoChange (DWORD dwInfoState);
    virtual void OnStateChange (DWORD dwState, DWORD dwStateDetail, DWORD dwPrivilage);
    virtual void OnGatherDigitsComplete (DWORD dwReason);
    virtual void OnGenerateComplete (DWORD dwReason);
    virtual void OnDigitDetected (DWORD dwDigit, DWORD dwDigitMode);
    virtual void OnMediaModeChange (DWORD dwMediaMode);
    virtual void OnToneDetected (DWORD dwAppSpecific);
    
    // These are called on state changes from the OnStateChange() member.
    virtual void OnCallStateIdle();
    virtual void OnCallStateOffering(DWORD dwOfferingMode);
    virtual void OnCallStateAccepted();
    virtual void OnCallStateDialtone(DWORD dwDialToneMode);
    virtual void OnCallStateDialing();
    virtual void OnCallStateRingback();
    virtual void OnCallStateBusy(DWORD dwBusyMode);
    virtual void OnCallStateConnected(DWORD dwConnectMode);
    virtual void OnCallStateProceeding();
    virtual void OnCallStateHold();
    virtual void OnCallStateHoldPendingConference();
    virtual void OnCallStateHoldPendingTransfer();
    virtual void OnCallStateSpecialInfo(DWORD dwInfo);
    virtual void OnCallStateDisconnected(DWORD dwDisconnectMode);
    virtual void OnCallStateConferenced();
    
    friend class CTapiLine;    
};

////////////////////////////////////////////////////////////////////////////////////
// CTapiPhone
//
// This object encapsulates a phone device in our TAPI world.  Each phone device
// is owned by a CTapiConnection.
//

class CTapiPhone : public CTapiObject
{ 
    DECLARE_DYNCREATE (CTapiPhone)
// Class data
protected:
	enum {
		Removed = 0x01					// Line is not valid and has been removed.
	};

	int					m_iFlags;		// Flags for this phone.
    CTapiConnection*    m_pConn;        // TAPI connection owner
    HPHONE              m_hPhone;       // Opaque Phone handle
    DWORD               m_dwDeviceID;   // Device ID (0-num Devices)
    DWORD               m_dwAPIVersion; // API version negotiated with TAPI
	LPPHONECAPS			m_lpPhoneCaps;	// Phone capabilities
	LPPHONESTATUS		m_lpPhoneStatus; // Phone status
    
// Constructor
public:
    CTapiPhone();
    virtual ~CTapiPhone();
    
// Access methods
public:
    const LPPHONECAPS GetPhoneCaps(DWORD dwAPIVersion=0, DWORD dwExtVersion=0, BOOL fForceRealloc=FALSE);
    const LPPHONESTATUS GetPhoneStatus(BOOL fForceRealloc=FALSE);

    DWORD GetDeviceID() const;
    DWORD GetNegotiatedAPIVersion() const;
    CString GetPhoneName() const;
    CString GetProviderInfo() const;
    CTapiConnection* GetTapiConnection() const;

    // Open the phone device
    LONG Open(DWORD dwPriv, DWORD dwAPIVersion=0, DWORD dwExtVersion=0);
    BOOL IsOpen() const;
	BOOL IsValid() const;
    HPHONE GetPhoneHandle() const;
    LONG Close();

    // Status message notifications
    LONG GetStatusMessages (LPDWORD lpdwPhoneStatus, LPDWORD lpdwButtonModes, LPDWORD lpdwButtonStates);
    LONG SetStatusMessages (DWORD dwPhoneStates, DWORD dwButtonModes, DWORD dwButtonStates);
    
    // Retrieve line handle information
    LONG GetID (LPVARSTRING lpDeviceID, LPCTSTR lpszDeviceClass);
	LONG GetButtonInfo(int iButtonID, LPBUTTONINFO lpButton);

    // Manage configuration
    LONG Config (CWnd* pwndOwner, LPCTSTR lpszDeviceClass);
    LONG DevSpecific (LPVOID lpParams, DWORD dwSize);
    
	// TAPI Functions
	LONG SetVolume(DWORD dwDevice, DWORD dwVolume);
	LONG SetGain(DWORD dwDevice, DWORD dwGain);
	LONG SetHookswitch(DWORD dwDevice, DWORD dwMode);
	LONG SetRing(DWORD dwRingMode, DWORD dwRingVolume);
	LONG SetDisplay(LPCSTR pszDisplay);
	LONG SetHandsetVolume(DWORD dwVolume) { return SetVolume(PHONEHOOKSWITCHDEV_HANDSET, dwVolume); }
	LONG SetHeadsetVolume(DWORD dwVolume) { return SetVolume(PHONEHOOKSWITCHDEV_HEADSET, dwVolume); }
	LONG SetSpeakerVolume(DWORD dwVolume) { return SetVolume(PHONEHOOKSWITCHDEV_SPEAKER, dwVolume); }
	LONG SetHandsetGain(DWORD dwGain) { return SetGain(PHONEHOOKSWITCHDEV_HANDSET, dwGain); }
	LONG SetHeadsetGain(DWORD dwGain) { return SetGain(PHONEHOOKSWITCHDEV_HEADSET, dwGain); }
	LONG SetSpeakerGain(DWORD dwGain) { return SetGain(PHONEHOOKSWITCHDEV_SPEAKER, dwGain); }
	LONG SetHandsetHookswitch(DWORD dwMode) { return SetHookswitch(PHONEHOOKSWITCHDEV_HANDSET, dwMode); }
	LONG SetHeadsetHookswitch(DWORD dwMode) { return SetHookswitch(PHONEHOOKSWITCHDEV_HEADSET, dwMode); }
	LONG SetSpeakerHookswitch(DWORD dwMode) { return SetHookswitch(PHONEHOOKSWITCHDEV_SPEAKER, dwMode); }

    // Misc. functions
    BOOL GetTSPProvider (LPTAPIPROVIDER pProvider) const;
	DWORD GetProviderID() const;
	void GetValidIDs(CStringArray& arrKeys) const;
    HICON GetIcon(LPCTSTR lpszDeviceClass = NULL);
	DWORD GetRelatedLineID();    
	DWORD GetLampMode(int iLampID);
	CString GetDisplay() const;

public:
    void PhoneCallback (DWORD hDevice, DWORD dwMsg, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);

protected:    
    void Init (CTapiConnection* pConn, DWORD dwDeviceID);
    void GatherPhoneCapabilities();
    
// Overriable methods for notifications
protected:
    virtual void OnClose();
    virtual void OnDevSpecific (DWORD dwHandle, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);
	virtual void OnButton(DWORD dwButtonLampID, DWORD dwButtonMode, DWORD dwButtonState);
	virtual void OnDeviceStateChange(DWORD dwPhoneState, DWORD dwDetail);
	virtual void OnDynamicCreate();
	virtual void OnDynamicRemove();
	virtual void OnForceClose();

// Friend definitions
    friend class CTapiConnection;
};

////////////////////////////////////////////////////////////////////////////////////
// IsTapiError
//
// Return TRUE/FALSE if the result represents an error (line/phone)
//
inline BOOL IsTapiError(LONG lResult)
{
	return (lResult > 0x80000000);

}// IsTapiError

////////////////////////////////////////////////////////////////////////////////////
// AllocMem
//
// Basic allocator used by the library
//
inline LPVOID AllocMem(DWORD dwSize)
{
	return calloc(1, dwSize);

}// AllocMem

////////////////////////////////////////////////////////////////////////////////////
// FreeMem
//
// Basic allocator used by the library
//
inline void FreeMem(LPVOID lpMem)
{
	free (lpMem);

}// FreeMem

////////////////////////////////////////////////////////////////////////////////////
// ManageAsynchRequest
//
// Add a request (if necessary) to the asynch list
//
inline LONG ManageAsynchRequest (LONG lRequestID)
{
	CTapiConnection* pConn = GetTAPIConnection();
	pConn->AddRequest(lRequestID);
	return lRequestID;

}// ManageAsynchRequest
                          
#endif // __ATAPI_INC__   
