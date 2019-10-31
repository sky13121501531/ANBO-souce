///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：UdpAlarmRecvThread.cpp
///////////////////////////////////////////////////////////////////////////////////////////
#include "CPUAlarmUpLoad.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/PropManager.h"
#include "../Alarm/AlarmMgr.h"
#include "../Foundation/AppLog.h"
#include "../DeviceAccess/HTTPDeviceAccess.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include "../DeviceAccess/CardType.h"
#include "../DBAccess/DBManager.h"
#include <vector>
BOOL CPUAlarmFlag = FALSE;

CPUAlarmUpLoad::~CPUAlarmUpLoad()
{
}

CPUAlarmUpLoad::CPUAlarmUpLoad()
{
    bFlag = true;
    CPUAlarmParm = 0;
}

int CPUAlarmUpLoad::Start()
{
	//发送线程开始
	open(0);
	return 0;
}

int CPUAlarmUpLoad::open(void*)
{
	activate();
	return 0;
}
int CPUAlarmUpLoad::svc()
{
    //cpu使用率
	
    while(bFlag)
    {
        Sleep(100);
        getWin_CpuUsage(CPUAlarmParm);
    }
	return 0;
}

int CPUAlarmUpLoad::Stop()
{
	bFlag = false;
	this->wait();
	return 0;
}
__int64 CPUAlarmUpLoad::CompareFileTime(FILETIME time1, FILETIME time2)
{
    __int64 a = time1.dwHighDateTime << 32 | time1.dwLowDateTime;
    __int64 b = time2.dwHighDateTime << 32 | time2.dwLowDateTime;

    return (b - a);
}
void CPUAlarmUpLoad::getWin_CpuUsage(int& CPUParam) 
{
    HANDLE hEvent;
    BOOL res;
    BOOL ret = FALSE;
    FILETIME preidleTime;
    FILETIME prekernelTime;
    FILETIME preuserTime;
    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;

    res = GetSystemTimes(&idleTime, &kernelTime, &userTime);
    preidleTime = idleTime;
    prekernelTime = kernelTime;
    preuserTime = userTime;
	if(CPUParam==0)
	{
		CPUParam = 90;
	}
    hEvent = CreateEventA(NULL, FALSE, FALSE, NULL); // 初始值为 nonsignaled ，并且每次触发后自动设置为nonsignaled
	
    time_t timeNow = time(0);
    while (!ret) 
    {
		Sleep(5);
        WaitForSingleObject(hEvent, 1000);
        res = GetSystemTimes(&idleTime, &kernelTime, &userTime);

        __int64 idle = CompareFileTime(preidleTime, idleTime);
        __int64 kernel = CompareFileTime(prekernelTime, kernelTime);
        __int64 user = CompareFileTime(preuserTime, userTime);

        __int64 tempcpu = (kernel + user - idle) * 100 / (kernel + user);
		int cpu = (int)tempcpu;
        if(cpu>=CPUParam)
        {
			time_t time0 = time(0);
            if(time0 - timeNow >= 10)
            {
                CPUAlarmFlag = TRUE;
                m_timeUP = timeNow - 10;
            }
            ret = false;
        }
        else
        {
            ret = true;
            CPUAlarmFlag = FALSE;
            m_timeDel = time(0);
        }
        preidleTime = idleTime;
        prekernelTime = kernelTime;
        preuserTime = userTime;
    }
	CloseHandle(hEvent);
}