
#include "GetLevelForUnLockForAM.h"
#include "CheckLevelForUnLock.h"
#include "AlarmMgr.h"

#include "../Foundation/OSFunction.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/AlarmPropManager.h"
#include "../DeviceAccess/AMTcpDeviceAccess.h"
#include "../Communications/SysMsgSender.h"

GetLevelForUnLockForAM::GetLevelForUnLockForAM(void)
{	
	InitDeviceSock();
}

GetLevelForUnLockForAM::~GetLevelForUnLockForAM(void)
{
	if(Device!=NULL)
		delete Device;
}
bool GetLevelForUnLockForAM::Start()
{
	bFlag = true;
	this->open(0);
	return true;
}

int GetLevelForUnLockForAM::open( void* )
{
	bFlag=true;
	activate();
	return 0;
}
bool GetLevelForUnLockForAM::InitDeviceSock()
{
	Device=NULL;
	string strIP;
	int    port,deviceid;

	std::list<int> devicelist;
	PROPMANAGER::instance()->GetTaskDeviceList(std::string("QualityRealtimeQueryTask"),AM,devicelist);

	if (!devicelist.empty())
	{
		deviceid=devicelist.front();
		PROPMANAGER::instance()->GetDeviceIP(deviceid,strIP);
		PROPMANAGER::instance()->GetDeviceCmdPort(deviceid,port);
		Device =new AmTcpDeviceAccess(deviceid,strIP,port);
	}

	return true;
}
int GetLevelForUnLockForAM::svc()
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)录像频点(AM)电平获取线程开始执行 !\n"));
	while(bFlag)
	{
		//if(AlarmRecordInfo.size()==0)
		//{
			OSFunction::Sleep(0,500);
		//	continue;
		//}
		//CheckMutex.acquire();
		//TempInfo.clear();
		//TempInfo=AlarmRecordInfo;
		//CheckMutex.release();

		//std::vector<sSignalCheck>::iterator ptr=TempInfo.begin();
		//for(;ptr!=TempInfo.end();ptr++)
		//{
		//	(*ptr).level=GetLevel((*ptr).Freq);
		//	if ((*ptr).level > 0)
		//	{
		////		std::string info = std::string("模块 AM 频点: ")+ (*ptr).Freq +std::string("电平: ") + StrUtil::Int2Str((*ptr).level);
		////		SYSMSGSENDER::instance()->SendMsg(info,AM,VS_MSG_SYSTINFO,DATAANALYSER,LOG_EVENT_DEBUG,false,true);
		//		CHECKLEVELFORUNLOCK::instance()->UpdateLevel(*ptr);
		//	}
		//	OSFunction::Sleep(0,50);
		//}
	}
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)录像频点(AM)电平获取线程停止执行 !\n"));
	return 0;
}
int GetLevelForUnLockForAM::Stop()
{
	bFlag=false;
	this->wait();
	return 0;
}
bool GetLevelForUnLockForAM::AddRecordInfo(sSignalCheck Info)
{
	CheckMutex.acquire();
	AlarmRecordInfo.push_back(Info);
	CheckMutex.release();
	return true;
}
bool GetLevelForUnLockForAM::RemoveRecordInfo(sSignalCheck Info)
{
	CheckMutex.acquire();
	std::vector<sSignalCheck>::iterator ptr=AlarmRecordInfo.begin();
	for(;ptr!=AlarmRecordInfo.end();ptr++)
	{
		if( (*ptr).dvbtype==Info.dvbtype && (*ptr).ChannelID==Info.ChannelID && (*ptr).DeviceID==Info.DeviceID)
		{
			AlarmRecordInfo.erase(ptr);
			break;
		}
	}
	CheckMutex.release();
	return true;
}

int GetLevelForUnLockForAM::GetLevel(std::string strfreq)
{
	float freq = StrUtil::Str2Float(strfreq);
	int f_value = 0;
	
	if(Device!=NULL)
	{
		for (int i=0;i<5;++i)
		{
			RadioQuaRetMessage_Obj rqr;
			if(Device->GetQuality(freq,rqr))
				f_value=rqr.level_int;

			OSFunction::Sleep(1);
		}
	}
	return f_value;
}