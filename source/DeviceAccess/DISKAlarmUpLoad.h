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

struct DISKAlarm
{
    std::string disk;
    float DiskFree;
    int count;
    time_t timeAlarm;
    time_t timeDel;
};
class DISKAlarmUpLoad : public ACE_Task<ACE_MT_SYNCH>
{
public:
	DISKAlarmUpLoad();
	virtual ~DISKAlarmUpLoad();
public:
	int Start();
	int open(void*);
	virtual int svc();
	int Stop();
    int getWin_DiskUsage(int &DISKAlarmParam);
    int DISKParam;
public:
    char* Dir[5];
	DISKAlarm disk[5];
private:
	int count;
    bool bFlag;
    bool DISKFlag;
	bool diskF[5];
    time_t timeDISK;
};