//
//	Last Modified: $Date: 2010-07-20 09:48:13 $
//
//	$Log: TapiConn.h,v $
//	Revision 1.2  2010-07-20 09:48:13  lgrave
//	corrected windows crlf to unix lf
//

//	Revision 1.1  2010-07-19 23:40:44  lgrave

//	1st version added to cvs

//
//

#ifndef TA_TAPICONN_H
#define TA_TAPICONN_H

#include <map>
#include <string.h>
#include <sstream>
#include <vector>

#include "Atapi.h"
#include "Log.h"
#include "XmlRpc.h"

using namespace XmlRpc;

#define LINEDEVSTATE_ALL 0x01FFFFFF
#define LINEADDRESSSTATE_ALL 0x000001FF


#define SHOW(x)		do {\
							std::stringstream _ss;\
							std::string _str;\
							_ss << (x);\
							_ss >> _str;\
							theLog->Debug(#x "=" + _str);\
					} while(0)
					
#define xSHOW(x)	SHOW(x)	//((void)0)


std::string TapiErrMsg(DWORD err);

class LineDevStatus
{
public:
	std::string Phone_Extension;
	bool Forward_on_busy;
	bool Forward_on_no_answer;
	bool Forward_unconditional;
	bool Forward_hunt_group_flag;
	bool Do_Not_Disturb;
	bool Outgoing_call_bar_flag;
	bool Call_waiting_on_flag;
	bool Voicemail_on_flag;
	bool Voicemail_ring_back_flag;
	int Number_of_read_voicemail_messages;
	int Number_of_unread_voicemail_messages;
	int Outside_call_sequence_number;
	int Inside_call_sequence_number;
	int Ring_back_sequence_number;
	int No_answer_timeout_period;
	int Wrap_up_time_period;
	bool Can_intrude_flag;
	bool Cannot_be_intruded_upon_flag;
	bool X_directory_flag;
	bool Force_login_flag;
	bool Forced_account_code_flag;
	bool Login_code_flag;
	bool System_phone_flag;
	int Absent_message_id;
	bool Absent_message_set_flag;
	bool Voicemail_email_mode;
	std::string Extn;
	std::string locale;
	std::string Forward_destination;
	std::string Follow_me_number;
	std::string Absent_text;
	std::string Do_not_disturb_exception_list;
	std::string Forward_on_busy_number;
	int User_s_priority;
	int Group_membership;
	int Groups_out_of_time;
	int Disabled_groups;
	int Groups_out_of_service;
	int Night_service_groups;
	std::string Working_Hours_User_Rights_Group_Name;
	std::string Out_of_Hours_User_Rights_Group_Name;
	std::string User_Restriction_Name_List;
	
	LineDevStatus(unsigned char *buffer, int size);
	LineDevStatus(void) {}
	void Show(void)
	{
		SHOW(Phone_Extension);
		SHOW(Forward_on_busy);
		SHOW(Forward_on_no_answer);
		SHOW(Forward_unconditional);
		SHOW(Forward_hunt_group_flag);
		SHOW(Do_Not_Disturb);
		SHOW(Outgoing_call_bar_flag);
		SHOW(Call_waiting_on_flag);
		SHOW(Voicemail_on_flag);
		SHOW(Voicemail_ring_back_flag);
		SHOW(Number_of_read_voicemail_messages);
		SHOW(Number_of_unread_voicemail_messages);
		SHOW(Outside_call_sequence_number);
		SHOW(Inside_call_sequence_number);
		SHOW(Ring_back_sequence_number);
		SHOW(No_answer_timeout_period);
		SHOW(Wrap_up_time_period);
		SHOW(Can_intrude_flag);
		SHOW(Cannot_be_intruded_upon_flag);
		SHOW(X_directory_flag);
		SHOW(Force_login_flag);
		SHOW(Forced_account_code_flag);
		SHOW(Login_code_flag);
		SHOW(System_phone_flag);
		SHOW(Absent_message_id);
		SHOW(Absent_message_set_flag);
		SHOW(Voicemail_email_mode);
		SHOW(Extn);
		SHOW(locale);
		SHOW(Forward_destination);
		SHOW(Follow_me_number);
		SHOW(Absent_text);
		SHOW(Do_not_disturb_exception_list);
		SHOW(Forward_on_busy_number);
		SHOW(User_s_priority);
		SHOW(Group_membership);
		SHOW(Groups_out_of_time);
		SHOW(Disabled_groups);
		SHOW(Groups_out_of_service);
		SHOW(Night_service_groups);
		SHOW(Working_Hours_User_Rights_Group_Name);
		SHOW(Out_of_Hours_User_Rights_Group_Name);
		SHOW(User_Restriction_Name_List);
	}

};

class CMyLine : public CTapiLine
{
	DECLARE_DYNCREATE (CMyLine)
	
	bool init;
	std::string central_id;
	std::string line_extension;
	LineDevStatus line_dev_status;
	void Init(void);
	std::string LineExtension(void) { return line_dev_status.Phone_Extension;}
	std::string UserExtension(void) { return line_dev_status.Extn;}
	XmlRpcClient *the_client1;
	XmlRpcClient *the_client2;
	std::string method;
	int order;
	
	void Send(std::map<std::string, std::string> arguments, int client = 1, std::string oldord = "");
	
//	void Send(std::vector<std::string> arguments);
	
	std::string HCALL2string(HCALL hCall)
	{
		char buf[1024] = {0};
		sprintf(buf, "%lX", (unsigned long)hCall);	
		return buf;
	}
	static HCALL string2HCALL(std::string str)
	{
		HCALL hCall = 0;
		sscanf(str.c_str(), "%lX", &hCall);
		return hCall;
	}
	

public:
	CMyLine();

// Overrides
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
	
};


class TapiConn
{
	std::map<std::string, CTapiLine*> lines;
public:
	void Connect();
	void Disconnect();
	void Logging_ON(std::string line_extension, std::string user_extension);
	void Logging_OFF(std::string line_extension, std::string user_extension);
	void Do_Not_Disturb_ON(std::string line_extension, std::string user_extension);
	void Do_Not_Disturb_OFF(std::string line_extension, std::string user_extension);
	void Listen(std::string line_extension, std::string user_extension, std::string extension_2_listen);
	void MakeCall(std::string line_extension,  std::string user_extension, std::string destination);
	void Answer(std::string call_id, std::string line_extension,  std::string user_extension);
	void Hold(std::string call_id, std::string line_extension,  std::string user_extension);
	void Unhold(std::string call_id, std::string line_extension,  std::string user_extension);
	void Hangup(std::string call_id, std::string line_extension,  std::string user_extension);
	void BlindTransfer(std::string call_id, std::string line_extension,  std::string user_extension, std::string extension_2_transfer);
};

extern TapiConn *theTapiConn;



#endif //TA_TAPICONN_H
