///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：UdpAlarmRecvThreadMgr.cpp
///////////////////////////////////////////////////////////////////////////////////////////
#include "UdpAlarmRecvThread.h"
#include "UdpAlarmRecvThreadMgr.h"
#include "../Foundation/PropManager.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../DeviceAccess/HTTPDeviceAccess.h"
#include "../Foundation/StrUtil.h"
#include "../Alarm/AlarmMgr.h"
#include "cping.h"
#include "../DeviceAccess/DeviceAccessMgr.h"
#include "../DBAccess/DBManager.h"
#include "../Foundation/AppLog.h"
#include <list>
#include <vector>
extern BOOL CPUAlarmFlag;
extern BOOL MEMAlarmFlag;
extern BOOL DISKAlarmFlag;
UdpAlarmRecvThreadMgr::UdpAlarmRecvThreadMgr()
{
	 pCPUAlarm= new CPUAlarmUpLoad();
	 pMEMAlarm= new MEMAlarmUpLoad();
	 pDISKAlarm= new DISKAlarmUpLoad();
}

UdpAlarmRecvThreadMgr::~UdpAlarmRecvThreadMgr()
{
	if(pCPUAlarm!=NULL)
	{
		delete pCPUAlarm;
		pCPUAlarm = NULL;
	}
	if(pMEMAlarm!=NULL)
	{
		delete pMEMAlarm;
		pMEMAlarm = NULL;
	}
	if(pDISKAlarm!=NULL)
	{
		delete pDISKAlarm;
		pDISKAlarm = NULL;
	}
}

int UdpAlarmRecvThreadMgr::Start()
{
	//发送线程开始
	open(0);
	return 0;
}
int UdpAlarmRecvThreadMgr::open(void*)
{
    bFlag = false;
	activate();
	return 0;
}
int UdpAlarmRecvThreadMgr::svc()
{
	UdpAlarmRecvThread* pFMUdpRecvAT= new UdpAlarmRecvThread(std::string("238.110.110.110"), std::string("12315"), std::string("172.16.10.201"));
	if(pFMUdpRecvAT!= NULL)
	{
		AlarmProUdpRecvThreadMap.insert(make_pair(1, pFMUdpRecvAT));
		pFMUdpRecvAT->Start();
	}

	UdpAlarmRecvThread* pATVUdpRecvAT= new UdpAlarmRecvThread(std::string("239.110.110.110"), std::string("12315"), std::string("172.16.10.202"));
	if(pATVUdpRecvAT!= NULL)
	{
		AlarmProUdpRecvThreadMap.insert(make_pair(2, pATVUdpRecvAT));
		pATVUdpRecvAT->Start();
	}
    if(pCPUAlarm!= NULL)
    {
        pCPUAlarm->Start();//CPU监控线程
    }
    
    if(pMEMAlarm!= NULL)
    {
        pMEMAlarm->Start();//内存监控线程
    }
   
    if(pDISKAlarm!= NULL)
    {
        pDISKAlarm->Start();//磁盘监控线程
    }
    int DevNum = PROPMANAGER::instance()->GetMonitorDevNum();
	time_t timebegin = time(0);
	time_t time_start_radio = time(0);
	time_t time_start_atv = time(0);
	while(!bFlag)
	{
		Sleep(100);
        std::vector<sAlarmParam> AlarmParamVec;
        DBMANAGER::instance()->QueryAlarmParam(ATV,AlarmParamVec);//查询报警参数;
        for(std::vector<sAlarmParam>::iterator piter = AlarmParamVec.begin();piter!=AlarmParamVec.end();piter++)
        {
            if(piter->TypeID=="31")
            {
                pCPUAlarm->CPUAlarmParm = StrUtil::Str2Int(piter->UpThreshold);
            }
            if(piter->TypeID=="32")
            {
                pMEMAlarm->MEMParam = StrUtil::Str2Int(piter->UpThreshold);
            }
            if(piter->TypeID=="33")
            {
                pDISKAlarm->DISKParam = StrUtil::Str2Int(piter->DownThreshold);
            }
        }
        //调频故障报警
		for(int i = 0;i<DevNum;++i)
		{
			sDeviceInfo DevInfo;
			PROPMANAGER::instance()->GetDevMonitorInfo(i,DevInfo);
			if(DevInfo.tsip!="")
			{
				char* DestIp = (char*)DevInfo.baseIP.c_str();
				CPing ping;
				PingReply pPingReply;
				if(!ping.Ping(DestIp,&pPingReply))
				{
					APPLOG::instance()->WriteLog(DEVICE,LOG_EVENT_WARNNING,string(DevInfo.devicetype+string("--IP:[")+DevInfo.baseIP+string("] 设备故障")),LOG_OUTPUT_SCREEN);
					//调频卡故障报警
					sCheckParam sEquCheck,QualitysEquCheck;
					sEquCheck.AlarmType = ALARM_EQUIPMENT;
					if(DevInfo.devicetype=="RADIO")
					{
						sEquCheck.DVBType = RADIO;
						sEquCheck.TypeDesc = "音频录制单元异常";
						sEquCheck.TypeID = "11";
						QualitysEquCheck.DVBType = RADIO;
						QualitysEquCheck.TypeDesc = "音频指标测量设备异常";
						QualitysEquCheck.TypeID = "13";
					}
					else if(DevInfo.devicetype=="ATV")
					{
						sEquCheck.DVBType = ATV;
						sEquCheck.TypeDesc = "视频录制单元异常";
						sEquCheck.TypeID = "12";
						QualitysEquCheck.DVBType = ATV;
						QualitysEquCheck.TypeDesc = "视频指标测量设备异常";
						QualitysEquCheck.TypeID = "14";
					}
					sEquCheck.mode = "0";
					sEquCheck.DeviceID = DevInfo.baseIP;

					QualitysEquCheck.AlarmType = ALARM_EQUIPMENT;
					QualitysEquCheck.mode = "0";
					QualitysEquCheck.DeviceID = DevInfo.baseIP;
					
					if(time(0) - time_start_radio>=5 && DevInfo.devicetype=="RADIO")
					{
						sEquCheck.CheckTime = time(0) - 5;
						ALARMMGR::instance()->CheckAlarm(sEquCheck,false);
						Sleep(500);
						QualitysEquCheck.CheckTime = time(0) - 5;
						ALARMMGR::instance()->CheckAlarm(QualitysEquCheck,false);
					}
					if(time(0) - time_start_atv>=5 && DevInfo.devicetype=="ATV")
					{
						ALARMMGR::instance()->CheckAlarm(sEquCheck,false);
						Sleep(500);
						ALARMMGR::instance()->CheckAlarm(QualitysEquCheck,false);
					}
				}
				else
				{
					//std::string NetRet = "ip="+DevInfo.baseIP+"--字节="+StrUtil::Int2Str(pPingReply.m_dwBytes) + "--" + "时间<=" + StrUtil::Int2Str(pPingReply.m_dwBytes) + "--" + "TTL=" + StrUtil::Int2Str(pPingReply.m_dwTTL);
					//APPLOG::instance()->WriteLog(DEVICE,LOG_EVENT_WARNNING,NetRet,LOG_OUTPUT_FILE);
					//调频卡故障解除
					sCheckParam sEquCheck,QualitysEquCheck;
					sEquCheck.AlarmType = ALARM_EQUIPMENT;
					QualitysEquCheck.AlarmType = ALARM_EQUIPMENT;
					if(DevInfo.devicetype=="RADIO")
					{
						sEquCheck.DVBType = RADIO;
						sEquCheck.TypeDesc = "音频录制单元异常解除";
						sEquCheck.TypeID = "11";

						QualitysEquCheck.DVBType = RADIO;
						QualitysEquCheck.TypeDesc = "音频指标测量设备异常解除";
						QualitysEquCheck.TypeID = "13";
						time_start_radio = time(0);
					}
					else if(DevInfo.devicetype=="ATV")
					{
						sEquCheck.DVBType = ATV;
						sEquCheck.TypeDesc = "视频录制单元异常解除";
						sEquCheck.TypeID = "12";
						QualitysEquCheck.DVBType = ATV;
						QualitysEquCheck.TypeDesc = "视频指标测量设备异常解除";
						QualitysEquCheck.TypeID = "14";
						time_start_atv = time(0);
					}
					sEquCheck.mode = "1";
					sEquCheck.DeviceID = DevInfo.baseIP;
					sEquCheck.CheckTime = time(0);
					QualitysEquCheck.mode = "1";
					QualitysEquCheck.DeviceID = DevInfo.baseIP;
					QualitysEquCheck.CheckTime = time(0);
					ALARMMGR::instance()->CheckAlarm(sEquCheck,false);
					Sleep(500);
					ALARMMGR::instance()->CheckAlarm(QualitysEquCheck,false);
				}
			}
		}
        //cpu报警
        if(CPUAlarmFlag || MEMAlarmFlag || DISKAlarmFlag)
        {
            //cpu持续使用率高 报警
			//
            sCheckParam sEquCheck;
            sEquCheck.AlarmType = ALARM_EQUIPMENT;
			sEquCheck.DVBType = UNKNOWN;
			sEquCheck.mode = "0";
            if(CPUAlarmFlag)
            {
                sEquCheck.TypeID = "1";
                sEquCheck.TypeDesc = "CPU利用率持续高值";
				sEquCheck.DeviceID = PROPMANAGER::instance()->GetDefSrcCode(ATV);
				sEquCheck.CheckTime = pCPUAlarm->gettimeAlarmUp();
				ALARMMGR::instance()->CheckAlarm(sEquCheck,false);
            }
            if(MEMAlarmFlag)
            {
                sEquCheck.TypeID = "2";
                sEquCheck.TypeDesc = "内存使用率持续高值";
				sEquCheck.DeviceID = PROPMANAGER::instance()->GetDefSrcCode(ATV);
				sEquCheck.CheckTime = pMEMAlarm->getMTimeAlarmUp();
				ALARMMGR::instance()->CheckAlarm(sEquCheck,false);
            }
            if(DISKAlarmFlag)
            {
                for(int i = 0;i<5;++i)
                {
                    string dir = pDISKAlarm->disk[i].disk;
                    if(dir!=""&& pDISKAlarm->disk[i].count!=-1)
                    {
						sEquCheck.TypeID = "3";
						int pos = dir.find(':');
						dir = dir.substr(0,pos+1);
                        sEquCheck.TypeDesc = dir + string("磁盘空间持续不足");
						sEquCheck.DeviceID = dir + PROPMANAGER::instance()->GetDefSrcCode(ATV);
						sEquCheck.CheckTime = pDISKAlarm->disk[i].timeAlarm;
						ALARMMGR::instance()->CheckAlarm(sEquCheck,false);
                    }
                }
            }
        }
        else
        {
            //cpu 恢复正常 解除报警
            //cpu持续使用率高 报警
            sCheckParam sEquCheck;
            sEquCheck.AlarmType = ALARM_EQUIPMENT;
            sEquCheck.DVBType = UNKNOWN;
			sEquCheck.mode = "1";
            if(!CPUAlarmFlag)
            {
                sEquCheck.TypeID = "1";
                sEquCheck.TypeDesc = "CPU利用率持续高值";
				sEquCheck.DeviceID = PROPMANAGER::instance()->GetDefSrcCode(ATV);
				sEquCheck.CheckTime = pCPUAlarm->gettimeAlarmDel();
				ALARMMGR::instance()->CheckAlarm(sEquCheck,false);
            }
            if(!MEMAlarmFlag)
            {
                sEquCheck.TypeID = "2";
                sEquCheck.TypeDesc = "内存使用率持续高值";
				sEquCheck.DeviceID = PROPMANAGER::instance()->GetDefSrcCode(ATV);
				sEquCheck.CheckTime = pMEMAlarm->getMTimeAlarmDel();
				ALARMMGR::instance()->CheckAlarm(sEquCheck,false);
            }
            if(!DISKAlarmFlag)
            {
                for(int i = 0;i<5;++i)
                {
                    string dir = pDISKAlarm->disk[i].disk;
                    if(dir!="" && pDISKAlarm->disk[i].count == -1)
                    {
						sEquCheck.TypeID = "3";
						int pos = dir.find(':');
						dir = dir.substr(0,pos+1);
                        sEquCheck.TypeDesc = dir + string("磁盘空间持续不足");
						sEquCheck.DeviceID = dir + PROPMANAGER::instance()->GetDefSrcCode(ATV);
						sEquCheck.CheckTime = pDISKAlarm->disk[i].timeDel;
						ALARMMGR::instance()->CheckAlarm(sEquCheck,false);
                    }
                }
            }
        }
	}
	return 0;
}

int UdpAlarmRecvThreadMgr::Stop()
{
	if(pCPUAlarm!=NULL)
	    pCPUAlarm->Stop();
	if(pMEMAlarm!=NULL)
		pMEMAlarm->Stop();
	if(pDISKAlarm!=NULL)
		pDISKAlarm->Stop();
    bFlag = true;
    this->wait();
    return 0;
}