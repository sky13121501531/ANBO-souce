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

class MEMAlarmUpLoad : public ACE_Task<ACE_MT_SYNCH>
{
public:
	MEMAlarmUpLoad();
	virtual ~MEMAlarmUpLoad();
public:
	int Start();

	int open(void*);

	virtual int svc();
    DWORD getWin_MemUsage();
	int Stop();
    time_t getMTimeAlarmUp(){return m_timeUP;};
    time_t getMTimeAlarmDel(){return m_timeDel;};
    int MEMParam;
private:
    bool bFlag;
    time_t m_timeUP;
    time_t m_timeDel;
    
};