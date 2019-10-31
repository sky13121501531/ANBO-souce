///////////////////////////////////////////////////////////////////////////////////////////
// ÎÄ¼þÃû£ºUdpAlarmRecvThreadMgr.h
//////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ace/Task.h"
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include <map>
#include "CPUAlarmUpLoad.h"
#include "MEMAlarmUpLoad.h"
#include "DISKAlarmUpLoad.h"

class UdpAlarmRecvThread;
class UdpAlarmRecvThreadMgr : public ACE_Task<ACE_MT_SYNCH>
{
public:
	UdpAlarmRecvThreadMgr();
	virtual ~UdpAlarmRecvThreadMgr();
public:
	int Start();
	int open(void*);

	virtual int svc();
	int Stop();
    CPUAlarmUpLoad* pCPUAlarm;
    MEMAlarmUpLoad* pMEMAlarm;
    DISKAlarmUpLoad* pDISKAlarm;
private:
	bool bFlag;
	ACE_Thread_Mutex EquipMutex;//
	ACE_Thread_Mutex EquipReMutex;//
	std::map<int,UdpAlarmRecvThread*> AlarmProUdpRecvThreadMap;
};
typedef  ACE_Singleton<UdpAlarmRecvThreadMgr,ACE_Mutex>  UDPALARMRECVTHREADMGR;
