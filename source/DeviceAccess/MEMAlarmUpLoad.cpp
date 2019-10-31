///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：UdpAlarmRecvThread.cpp
///////////////////////////////////////////////////////////////////////////////////////////
#include "MEMAlarmUpLoad.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/PropManager.h"
#include "../Alarm/AlarmMgr.h"
#include "../Foundation/AppLog.h"
#include "../DeviceAccess/HTTPDeviceAccess.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include "../DeviceAccess/CardType.h"
#include "../DBAccess/DBManager.h"

BOOL MEMAlarmFlag = FALSE;

MEMAlarmUpLoad::~MEMAlarmUpLoad()
{
}
MEMAlarmUpLoad::MEMAlarmUpLoad()
{
	
    bFlag = true;
    MEMParam = 0;
}

int MEMAlarmUpLoad::Start()
{
	//发送线程开始
	open(0);
	return 0;
}

int MEMAlarmUpLoad::open(void*)
{
	activate();
	return 0;
}
int MEMAlarmUpLoad::svc()
{
    while(bFlag)
    {
        Sleep(100);
        time_t timeNow = time(0);
        bool memFlag = false;
        if(MEMParam==0)
        {
            MEMParam = 90;
        }
        while(!memFlag)
        {
			Sleep(5);
            DWORD MEMRet = getWin_MemUsage();

            if(MEMRet>MEMParam)//门限
            {
                if(time(0) - timeNow >=10)//持续时长
                {
                    MEMAlarmFlag = TRUE;//内存使用率报警
                    m_timeUP = timeNow - 10;
                }
            }
            else
            {
                memFlag = true;
                MEMAlarmFlag = FALSE;
                m_timeDel = time(0);
            }
        }
    }
	return 0;
}

int MEMAlarmUpLoad::Stop()
{
	bFlag = false;
	this->wait();
	return 0;
}

DWORD MEMAlarmUpLoad::getWin_MemUsage()
{
    MEMORYSTATUS ms;
    ::GlobalMemoryStatus(&ms);
    return ms.dwMemoryLoad;
}
