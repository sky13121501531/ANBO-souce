///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：UdpAlarmRecvThread.cpp
///////////////////////////////////////////////////////////////////////////////////////////
#include "DISKAlarmUpLoad.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/PropManager.h"
#include "../Alarm/AlarmMgr.h"
#include "../Foundation/AppLog.h"
#include "../DeviceAccess/HTTPDeviceAccess.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include "../DeviceAccess/CardType.h"
#include "../DBAccess/DBManager.h"
#include <vector>
BOOL DISKAlarmFlag = FALSE;

DISKAlarmUpLoad::~DISKAlarmUpLoad()
{
}

DISKAlarmUpLoad::DISKAlarmUpLoad()
{
    bFlag = true;
    for(int i=0;i<5;++i)
    {
        Dir[i] = "";
		diskF[i]=false;
		disk[i].disk = "";
    }
    DISKParam = 0;
}

int DISKAlarmUpLoad::Start()
{
	//发送线程开始
	open(0);
	return 0;
}

int DISKAlarmUpLoad::open(void*)
{
	activate();
	return 0;
}
int DISKAlarmUpLoad::svc()
{
    while(bFlag)
    {
        Sleep(100);
		DISKFlag = false;
        timeDISK = time(0);
		for(int i=0;i<5;++i)
		{
			diskF[i] = false;
			//disk[i].disk = "";
		}
		count = 0;
        while(!DISKFlag)
        {
			Sleep(5);
			getWin_DiskUsage(DISKParam);
        }
    }
	return 0;
}

int DISKAlarmUpLoad::Stop()
{
	bFlag = false;
	this->wait();
	return 0;
}
int DISKAlarmUpLoad::getWin_DiskUsage(int &DISKAlarmParam)
{
    int DiskCount = 0;
    DWORD DiskInfo = GetLogicalDrives();
    while (DiskInfo)
    {
        if (DiskInfo & 1)
        {
            ++DiskCount;
        }
        DiskInfo = DiskInfo >> 1;
    }
    int DSLength = GetLogicalDriveStrings(0, NULL);
    char* DStr = new char[DSLength];
    GetLogicalDriveStrings(DSLength, DStr);

    int DType;
    int si = 0;
    BOOL fResult;
    DWORD64 i64FreeBytesToCaller;
    DWORD64 i64TotalBytes;//总容量
    DWORD64 i64FreeBytes;//剩余空间

	if(DISKAlarmParam==0)
	{
		DISKAlarmParam = 10;
	}
    /*为了显示每个驱动器的状态，则通过循环输出实现，由于DStr内部保存的数据是A:\NULLB:\NULLC:\NULL，这样的信息，所以DSLength/4可以获得具体大循环范围*/
					
    for (int i = 0; i<DSLength / 4; ++i)
    {
		char dir[3]={DStr[si],':','\\'};
		DType = GetDriveType(DStr+i*4);
		if(DType == DRIVE_FIXED)//硬盘
		{
			char *tmp = &DStr[si];
			string tmpDStr = tmp;
			fResult = GetDiskFreeSpaceEx(tmpDStr.c_str(),(PULARGE_INTEGER)&i64FreeBytesToCaller,(PULARGE_INTEGER)&i64TotalBytes,(PULARGE_INTEGER)&i64FreeBytes);
			if(fResult)
			{
				diskF[i] = false;
				float DiskFree = (float)i64FreeBytesToCaller / 1024 / 1024 /1024;
				disk[i].DiskFree = DiskFree;
				disk[i].count = -1;
				if(disk[i].DiskFree < DISKAlarmParam)
				{
					disk[i].count = i;
					DISKFlag = false;
					time_t timedisk = time(0);
					if(timedisk - timeDISK >= 10)
					{
						disk[i].disk = tmpDStr;
						DISKAlarmFlag = TRUE;
						disk[i].timeAlarm = timeDISK - 10;
					}
				}
				else
				{
					//每次进来做一次判断
					for(int k =0;k<DiskCount;++k)
					{
						if(disk[k].count == k)
						{
							diskF[k] = true;
						}
						else
						{
							disk[i].disk = tmpDStr;
							DISKAlarmFlag = FALSE;
							disk[i].timeDel = time(0);
						}
					}
					if(!diskF[0] && !diskF[1] && !diskF[2] && !diskF[3] && !diskF[4])
					{
						DISKFlag = true;
					}
				}
			}
			si += 4;
		}
    }
    return 0;
}