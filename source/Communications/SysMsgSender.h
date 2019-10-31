
#pragma once

#include "ace/Task.h"
#include "ace/SOCK_Dgram_Mcast.h"
#include "ace/INET_Addr.h"
#include "ace/Time_Value.h"
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include "ace/OS.h"
#include "../Foundation/TypeDef.h"
#include <string>



class SysMsgSender : public ACE_Task<ACE_MT_SYNCH>
{
public:
	SysMsgSender();
	~SysMsgSender();
public:
	int open(void*);

	virtual int svc();

	int Start();
	int Stop();

	bool SendMsg(std::string msg,enumDVBType DVBType=UNKNOWN,enumSysMsgType sysmsgtype=VS_MSG_SYSTINFO,eModuleType Module=OTHER,eLogType LogType=LOG_EVENT_DEBUG,bool forceprint=false,bool forcelog=false);
	
	bool SetSendFlag(bool flag);
	bool SetPrintLevel(int level){Print_Level=level;return true;};			//0-2£»0£ºVS_MSG_SYSALARM£»1£ºVS_MSG_SYSALARM£¬VS_MSG_PROALARM£»2£ºALL
	bool SetWriteLogLevel(int level){WriteLog_Level=level;return true;};	//0-2£»0£ºVS_MSG_SYSALARM£»1£ºVS_MSG_SYSALARM£¬VS_MSG_PROALARM£»2£ºALL
private:	
	bool bFlag;
	
	bool bSendFlag;
	int Print_Level;
	int WriteLog_Level;

	ACE_INET_Addr RemoteAddr;
	ACE_INET_Addr LocalAddr;
	ACE_SOCK_Dgram MsgSender;
};

typedef ACE_Singleton<SysMsgSender,ACE_Mutex>  SYSMSGSENDER;