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

class CPUAlarmUpLoad : public ACE_Task<ACE_MT_SYNCH>
{
public:
	CPUAlarmUpLoad();
	virtual ~CPUAlarmUpLoad();
public:
	int Start();

	int open(void*);

	virtual int svc();
   
	int Stop();
    __int64 CompareFileTime(FILETIME time1, FILETIME time2);
    void getWin_CpuUsage(int& CPUParam);
    time_t gettimeAlarmUp(){return m_timeUP;};
    time_t gettimeAlarmDel(){return m_timeDel;};
    int CPUAlarmParm;
private:
    bool bFlag;
    time_t m_timeUP;
    time_t m_timeDel;
    
};