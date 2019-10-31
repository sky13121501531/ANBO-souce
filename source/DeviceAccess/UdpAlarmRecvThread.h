///////////////////////////////////////////////////////////////////////////////////////////
// ÎÄ¼þÃû£ºUdpAlarmRecvThread.h
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ace/Task.h"
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include <ace/SOCK_Dgram_Mcast.h>
#include <ace/INET_Addr.h>
#include <string>
#include "../Foundation/TypeDef.h"

using namespace std;

class UdpAlarmRecvThread : public ACE_Task<ACE_MT_SYNCH>
{
public:
	UdpAlarmRecvThread(string ip, string port, std::string clientIP);
	virtual ~UdpAlarmRecvThread();
public:
	int Start();

	int open(void*);

	virtual int svc();
	bool ProcessProgramAlarm(const char* alarmBuf,const int alarmLen);

	int Stop();
private:
	std::string m_strIP;
	ACE_INET_Addr remote_addr;
	ACE_SOCK_Dgram_Mcast DeviceSock;
	bool bFlag;
};