
#include "GetLevelForUnLockForATV.h"
#include "CheckLevelForUnLock.h"
#include "AlarmMgr.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/AlarmPropManager.h"

#include "../BusinessProcess/BusinessLayoutMgr.h"

#include "../DeviceAccess/ATVHTTPDeviceAccess.h"
#include "../DeviceAccess/DeviceAccessMgr.h"
#include "../Communications/SysMsgSender.h"
ACE_Thread_Mutex SendDeviceMutex;
extern bool g_freqlockval[64];
extern bool g_atvchanscan;
GetLevelForUnLockForATV::GetLevelForUnLockForATV(void)
{	
	InitDeviceSock();
}

GetLevelForUnLockForATV::~GetLevelForUnLockForATV(void)
{
	if(Device!=NULL)
		delete Device;
}
bool GetLevelForUnLockForATV::Start()
{
	bFlag = true;
	this->open(0);
	return true;
}

int GetLevelForUnLockForATV::open( void* )
{
	bFlag=true;
	activate();
	return 0;
}
bool GetLevelForUnLockForATV::InitDeviceSock()
{
	Device=NULL;

	string strIP;
	int    port,deviceid;

	std::list<int> devicelist;
	PROPMANAGER::instance()->GetTaskDeviceList(std::string("QualityRealtimeQueryTask"),ATV,devicelist);

	if (!devicelist.empty())
	{
		deviceid=devicelist.front();
		PROPMANAGER::instance()->GetDeviceIP(deviceid,strIP);
		PROPMANAGER::instance()->GetDeviceCmdPort(deviceid,port);
		Device =new ATVHTTPDeviceAccess(deviceid,strIP,port); //?可能不需要
	}
	
	return true;
}

//以板卡为单位去获取通道锁定状态
#if 0
int GetLevelForUnLockForATV::svc()
{
	// OSFunction::Sleep(1);
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ATV无载波判断线程开始执行 !\n"));

	std::list<int> devicedlist;
	std::list<int> devicedlist1;
	PROPMANAGER::instance()->GetTaskDeviceList(std::string("AutoRecord"), ATV,devicedlist);	
	PROPMANAGER::instance()->GetTaskDeviceList(std::string("TaskRecord"), ATV,devicedlist1);
	std::list<int>::iterator ptr1=devicedlist1.begin();

	for(;ptr1!=devicedlist1.end();ptr1++)
	{
		bool bnext=false;
		std::list<int>::iterator ptr=devicedlist.begin();		
		for(;ptr!=devicedlist.end();ptr++)
		{
			if(*ptr1 == *ptr)
			{
				bnext = true;
				break;
			}
		}
		if(bnext)
			continue;
		else
			devicedlist.push_back(*ptr1);
	}

	std::map<std::string, VSTuner_AllChanFix_Result_Obj> strIpFixMap;
	std::map<std::string, VSTuner_AllChanFix_Result_Obj>::iterator strIpFixMapPtr;
	std::list<int>::iterator devptr=devicedlist.begin();
	for(; devptr != devicedlist.end(); devptr++)
	{
		std::string strIP;
		VSTuner_AllChanFix_Result_Obj tmpAllChanFixObj;
		memset(&tmpAllChanFixObj, 0, sizeof(VSTuner_AllChanFix_Result_Obj));
		PROPMANAGER::instance()->GetDeviceIP(*devptr, strIP);
		strIpFixMapPtr = strIpFixMap.find(strIP);
		if(strIpFixMapPtr == strIpFixMap.end())
			strIpFixMap.insert(make_pair(strIP, tmpAllChanFixObj));
	}

	cout<<"&&&&&开始检测电视无载波报警&&&&&"<<endl;
	while(bFlag)
	{	
		OSFunction::Sleep(0,100);

		std::map<std::string, VSTuner_AllChanFix_Result_Obj>::iterator ptr = strIpFixMap.begin();
		for(; ptr != strIpFixMap.end(); ptr++)
		{
			OSFunction::Sleep(0,10);
			//通过实际通道号获取板卡任务
			std::list<int> ipDevlist;
			PROPMANAGER::instance()->GetDeviceIDByIP(ptr->first, ipDevlist);
			if(ipDevlist.size() == 0)
				continue;

			if(!g_atvchanscan)
			{
				int devID = ipDevlist.front();
				DEVICEACCESSMGR::instance()->getAllChanFixInfoFor6UInCard(devID, ptr->second);
				//bTmpLocked = DEVICEACCESSMGR::instance()->getChanFixInfoFor6U(*ptr);
			}

			time_t checkTime = time(0);
			for(int i = 0; i < MAX_CHANNEL_NUM; i++)
			{
				sTaskInfo taskinfo;
				int tmpDeviceId;
				PROPMANAGER::instance()->GetDeviceID(ptr->first, StrUtil::Int2Str(i), tmpDeviceId);
				bool Rtn = BUSINESSLAYOUTMGR::instance()->QueryRunTaskInfo(tmpDeviceId, taskinfo);
				bool bTmpLocked = false;

				//如果该通道内无运行任务，或者运行任务不是录制任务，则报警无效
				if (Rtn == false || (taskinfo.taskname != "AutoRecord" && taskinfo.taskname != "TaskRecord"))
				{
					continue;
				}

				if(ptr->second.ChanFixObj[i].status == 1)
				{
					bTmpLocked = true;
				}

				if (!bTmpLocked)//该通道不锁定 
				{
					sCheckParam sCheck;
					sCheck.AlarmType	= ALARM_PROGRAM;
					sCheck.DVBType		= taskinfo.dvbtype;
					sCheck.ChannelID	= taskinfo.channelid;
					sCheck.Freq			= taskinfo.freq;
					sCheck.STD			= "";
					sCheck.SymbolRate	= "";
					sCheck.TypedValue	= "";
					sCheck.DeviceID		= StrUtil::Int2Str(tmpDeviceId);
					sCheck.CheckTime	= checkTime;
					sCheck.TypeID			= "0xE";
					sCheck.TypeDesc			= "无载波";

					for(int i =0; i <= 3; i++)
					{
						sCheckParam sTmpCheck1;
						sTmpCheck1.AlarmType	= sCheck.AlarmType;
						sTmpCheck1.DVBType		= sCheck.DVBType;		
						sTmpCheck1.ChannelID	= sCheck.ChannelID;
						sTmpCheck1.Freq			= sCheck.Freq;
						sTmpCheck1.STD			= sCheck.STD;
						sTmpCheck1.SymbolRate	= sCheck.SymbolRate;
						sTmpCheck1.TypedValue	= sCheck.TypedValue;
						sTmpCheck1.DeviceID		= sCheck.DeviceID;
						sTmpCheck1.CheckTime	= sCheck.CheckTime + 1 + i;
						sTmpCheck1.TypeID		= sCheck.TypeID;
						sTmpCheck1.TypeDesc		= sCheck.TypeDesc;

						ALARMMGR::instance()->CheckAlarm(sTmpCheck1,true);
					}
					ALARMMGR::instance()->CheckAlarm(sCheck,true);
					//cout << "通道[" << deviceId << "]出现报警，报警类型: " << sCheck.TypeDesc	<< endl;
				}
			}
		}
	}
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)录像频点(ATV)电平获取线程停止执行 !\n"));
	return 0;
}
#endif


int GetLevelForUnLockForATV::svc()
{
	// OSFunction::Sleep(1);
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ATV无载波判断线程开始执行 !\n"));
	std::list<int> devicedlist;
	std::list<int> devicedlist1;
	PROPMANAGER::instance()->GetTaskDeviceList(std::string("AutoRecord"), ATV,devicedlist);	
	PROPMANAGER::instance()->GetTaskDeviceList(std::string("TaskRecord"), ATV,devicedlist1);
	std::list<int>::iterator ptr1=devicedlist1.begin();
	bool atvFreqLock[100];
	for(int i = 0; i < 100;  i++)
	{
		atvFreqLock[i] = false;
	}

	for(;ptr1!=devicedlist1.end();ptr1++)
	{
		bool bnext=false;
		std::list<int>::iterator ptr=devicedlist.begin();		
		for(;ptr!=devicedlist.end();ptr++)
		{
			if(*ptr1 == *ptr)
			{
				bnext = true;
				break;
			}
		}
		if(bnext)
			continue;
		else
			devicedlist.push_back(*ptr1);
	}
	cout<<"&&&&&开始检测电视无载波报警&&&&&"<<endl;
	while(bFlag)
	{
		//if(AlarmRecordInfo.size()==0)
		//{
		OSFunction::Sleep(0,300);
		std::list<int>::iterator ptr=devicedlist.begin();
		for(;ptr!=devicedlist.end();ptr++)
		{
			OSFunction::Sleep(0,1);
			//通过实际通道号获取板卡任务
			sTaskInfo taskinfo;
			bool Rtn = BUSINESSLAYOUTMGR::instance()->QueryRunTaskInfo(*ptr,taskinfo);
			bool bTmpLocked = false;
			//如果该通道内无运行任务，或者运行任务不是录制任务，则报警无效
			if (Rtn == false || (taskinfo.taskname != "AutoRecord" && taskinfo.taskname != "TaskRecord"))
			{
				continue;
			}
			if(!g_atvchanscan)
			{
				SendDeviceMutex.acquire();
				bTmpLocked = DEVICEACCESSMGR::instance()->getChanFixInfoFor6U(*ptr);
				SendDeviceMutex.release();
			}
			else
			{
				bTmpLocked = atvFreqLock[*ptr];
			}

			if (!bTmpLocked)//该通道不锁定 
			{
				g_freqlockval[*ptr] = false;
				sCheckParam sCheck;
				sCheck.AlarmType	= ALARM_PROGRAM;
				sCheck.DVBType		= taskinfo.dvbtype;
				sCheck.ChannelID	= taskinfo.channelid;
				sCheck.Freq			= taskinfo.freq;
				sCheck.STD			= "";
				sCheck.SymbolRate	= "";
				sCheck.TypedValue	= "";
				sCheck.DeviceID		= StrUtil::Int2Str(*ptr);
				sCheck.CheckTime	= time(0);
				sCheck.TypeID			= "0xE";
				sCheck.TypeDesc			= "无载波";

				for(int i =0; i <= 3; i++)
				{
					sCheckParam sTmpCheck1;
					sTmpCheck1.AlarmType	= sCheck.AlarmType;
					sTmpCheck1.DVBType		= sCheck.DVBType;		
					sTmpCheck1.ChannelID	= sCheck.ChannelID;
					sTmpCheck1.Freq			= sCheck.Freq;
					sTmpCheck1.STD			= sCheck.STD;
					sTmpCheck1.SymbolRate	= sCheck.SymbolRate;
					sTmpCheck1.TypedValue	= sCheck.TypedValue;
					sTmpCheck1.DeviceID		= sCheck.DeviceID;
					sTmpCheck1.CheckTime	= sCheck.CheckTime + 1 + i;
					sTmpCheck1.TypeID		= sCheck.TypeID;
					sTmpCheck1.TypeDesc		= sCheck.TypeDesc;

					ALARMMGR::instance()->CheckAlarm(sTmpCheck1,true);
				}
				ALARMMGR::instance()->CheckAlarm(sCheck,true);
				//cout << "通道[" << *ptr << "]出现报警，报警类型: " << sCheck.TypeDesc	<< endl;
			}
			else
			{
				g_freqlockval[*ptr] = true;
			}
			atvFreqLock[*ptr] = bTmpLocked;
			time_t m_strEndTime = time(0);
		}
	}
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)录像频点(ATV)电平获取线程停止执行 !\n"));
	return 0;
}
int GetLevelForUnLockForATV::Stop()
{
	bFlag=false;
	this->wait();
	return 0;
}
bool GetLevelForUnLockForATV::AddRecordInfo(sSignalCheck Info)
{
	CheckMutex.acquire();
	AlarmRecordInfo.push_back(Info);
	CheckMutex.release();
	return true;
}
bool GetLevelForUnLockForATV::RemoveRecordInfo(sSignalCheck Info)
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

int GetLevelForUnLockForATV::GetLevel(std::string strfreq)
{
	float freq = StrUtil::Str2Float(strfreq);
	int f_value = 0;
	
// 	if(Device!=NULL)
// 	{
// 		int temfreq = (int)(freq*1000.0);
// 		int level = Device->SendCmdToTVCom(MSG_GET_QUA,(void *)&temfreq,sizeof(int));
// 		if(level>0)
// 		{
// 			f_value=level/100;
// 		}
// 		else
// 		{
// 			f_value= 0;
// 		}
// 	}
	return f_value;
}