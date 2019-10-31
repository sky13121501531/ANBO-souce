///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：UdpAlarmRecvThread.cpp
///////////////////////////////////////////////////////////////////////////////////////////
#include "UdpAlarmRecvThread.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/PropManager.h"
#include "../Alarm/AlarmMgr.h"
#include "../Foundation/AppLog.h"
#include "../DeviceAccess/HTTPDeviceAccess.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include "../DeviceAccess/CardType.h"
#define ALARMLEN   (sizeof(Alarm_Info_t) + 2)
UdpAlarmRecvThread::~UdpAlarmRecvThread()
{
}

UdpAlarmRecvThread::UdpAlarmRecvThread(string ip, string port, std::string clientIP)
{
	string mulitcase = ip + string(":")+ port;
	ACE_INET_Addr multicast_addr_(mulitcase.c_str()); 
	DeviceSock.join(multicast_addr_);
	DeviceSock.set_option(IP_MULTICAST_TTL, 5);
	int nSocketBuffSize = 1024*500;
	ACE_SOCK* sk= &DeviceSock;
	sk->set_option(SOL_SOCKET, SO_RCVBUF, &nSocketBuffSize, sizeof(int));
	m_strIP = clientIP;
}

int UdpAlarmRecvThread::Start()
{
	//发送线程开始
	open(0);
	return 0;
}

int UdpAlarmRecvThread::open(void*)
{
	bFlag = true;
	activate();
	return 0;
}
int UdpAlarmRecvThread::svc()
{
	int RcvBufLen = ALARMLEN*50;
	char RcvBuf[ALARMLEN*50] = {0};
	ssize_t RecvLen = 0;
	ACE_Time_Value RecvTimeOut(5);
	string timestr = "";
	while (bFlag)
	{
		//
		Sleep(5);
		memset(RcvBuf,0,RcvBufLen);
		RecvLen = DeviceSock.recv(RcvBuf,RcvBufLen,remote_addr,0,&RecvTimeOut);	//接收数据
		if(RecvLen <= 0)
		{
			Sleep(1);
            //APPLOG::instance()->WriteLog(DEVICE,LOG_EVENT_DEBUG,"丢失报警信息",LOG_OUTPUT_BOTH);
			continue;
		}
		else
		{
			ProcessProgramAlarm(RcvBuf, RecvLen);
		}
	}
	bFlag = false;
	return 0;
}

int UdpAlarmRecvThread::Stop()
{
	bFlag = false;
	this->wait();
	return 0;
}


bool UdpAlarmRecvThread::ProcessProgramAlarm(const char* alarmBuf,const int alarmLen)
{
	int OneAlarmInfoLen = sizeof(Alarm_Info_t);
	int bufIndex = 0;
	int deviceid =0;
	if(alarmLen < ALARMLEN)
		return false;
	while(alarmLen - bufIndex >= ALARMLEN)//收到的数据够一条报警长度
	{
		if((alarmBuf[bufIndex]== (char)0xff)&&(alarmBuf[bufIndex + OneAlarmInfoLen + 1]== (char)0xee))
		{
			//存在一条报警
			//处理报警
			Alarm_Info_t alarm;
			memcpy(&alarm,&alarmBuf[bufIndex + 1],OneAlarmInfoLen);
			
			eDVBType dvbtype = UNKNOWN;
			PROPMANAGER::instance()->GetDeviceID(m_strIP, StrUtil::Int2Str(alarm.ChannelNO), deviceid);		//数字板卡默认为0;
			PROPMANAGER::instance()->GetDeviceType(deviceid,dvbtype);	//获取板卡监测类型

			sTaskInfo taskinfo;
			bool Rtn = BUSINESSLAYOUTMGR::instance()->QueryRunTaskInfo(deviceid,taskinfo);
			//有AutoRecord和TaskRecord任务，使用报警
			if(Rtn && (taskinfo.taskname == "AutoRecord" || taskinfo.taskname == "TaskRecord"))
			{
				sCheckParam sCheck;
				sCheck.AlarmType	= ALARM_PROGRAM;
				sCheck.DVBType		= taskinfo.dvbtype;
				sCheck.ChannelID	= taskinfo.channelid;
				sCheck.Freq			= taskinfo.freq;
				sCheck.STD			= "";
				sCheck.SymbolRate	= "";
				sCheck.TypedValue	= "";
				sCheck.DeviceID		= StrUtil::Int2Str(deviceid);
				//time_t tmpTime	= (time_t)(((alarm.AlarmTimeMs+500)/1000)-28800);//四舍五入
				sCheck.CheckTime	= time(0);
#if 1
                int nCount = alarm.AlarmType;
                bool alarmN = false;
                string tmpmode = StrUtil::Int2Str(alarm.isAlarm);
                if(tmpmode=="1")
                {
                    sCheck.mode = "0";
                }
                else
                {
                    sCheck.mode = "1";
                }
                switch(nCount)
                {
                    sCheck.DVBType = taskinfo.dvbtype;
                case ALARM_FREEZE:
                    sCheck.TypeID = "0x4";
                    sCheck.TypeDesc = "图像静止";
                    break;
                case ALARM_BLACK:
                    sCheck.TypeID = "0x1";
                    sCheck.TypeDesc = "黑屏";
                    break;
                case ALARM_COLORBAR:
                    sCheck.TypeID = "0x2";
                    sCheck.TypeDesc = "彩条";
                    break;
                case ALARM_LOSTVIDEO:
                    sCheck.TypeID = "0x8";
                    sCheck.TypeDesc = "无图像";
                    break;
                case ALARM_AUDIOLOST:
                    sCheck.TypeID = "0x10";
                    sCheck.TypeDesc = "无声音";
                    break;
                case ALARM_LOSTSIGNAL:
                    sCheck.TypeID = "0xE";		
                    sCheck.TypeDesc = "无载波";
                    if(taskinfo.dvbtype == ATV)
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
                        sTmpCheck1.CheckTime	= sCheck.CheckTime/* + 1 + i*/;
                        sTmpCheck1.TypeID		= sCheck.TypeID;
                        sTmpCheck1.TypeDesc		= sCheck.TypeDesc;
                        sTmpCheck1.mode			= sCheck.mode;
                        ALARMMGR::instance()->CheckAlarm(sTmpCheck1,true);
                        alarmN = true;
                    }
                    break;
                default:
                    alarmN = true;
                    break;
                }
                if(!alarmN)
                {
					//if(sCheck.Freq == "90")
					//{
						//string strmsg = string("/*/*/*/*/*/*/*/*/*/*/*/*/:---")+sCheck.Freq + string("----") + sCheck.TypeDesc + string("----") + sCheck.mode;
						//APPLOG::instance()->WriteLog(DEVICE,LOG_EVENT_DEBUG,strmsg,LOG_OUTPUT_BOTH);
					//}
                    ALARMMGR::instance()->CheckAlarm(sCheck,true);
                }
#else

                if(alarm.AlarmType == ALARM_FREEZE)
                {
                    sCheck.TypeID = "0x4";
                    sCheck.TypeDesc = "图像静止";
                    sCheck.mode			= "0";//报警
                }
                else if(alarm.AlarmType == ALARM_BLACK)
                {
                    sCheck.TypeID = "0x1";
                    sCheck.TypeDesc = "黑屏";
                    sCheck.mode			= "0";//报警
                }
                else if(alarm.AlarmType == ALARM_COLORBAR)
                {
                    sCheck.TypeID = "0x2";
                    sCheck.TypeDesc = "彩条";
                    sCheck.mode			= "0";//报警
                }
                else if(alarm.AlarmType == ALARM_LOSTVIDEO)
                {
                    sCheck.TypeID = "0x8";
                    sCheck.TypeDesc = "无图像";
                    sCheck.mode			= "0";//报警
                }
                else if(alarm.AlarmType == ALARM_AUDIOLOST)
                {
                    sCheck.TypeID = "0x10";
                    //sCheck.TypeID = "24";
                    sCheck.TypeDesc = "无声音";
                    sCheck.mode			= "0";//报警
                    Rtn  = true;
                }
                else if(alarm.AlarmType == ALARM_LOSTSIGNAL)
                {
                    sCheck.TypeID = "0xE";		
                    sCheck.TypeDesc = "无载波";
                    sCheck.mode			= "0";//报警
                }
                else
                {
                    return true;
                }
                if(taskinfo.dvbtype == ATV && alarm.AlarmType == ALARM_LOSTSIGNAL)
                {
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
                        sTmpCheck1.mode			= sCheck.mode;
                        ALARMMGR::instance()->CheckAlarm(sTmpCheck1,true);
                    }
                }
                ALARMMGR::instance()->CheckAlarm(sCheck,true);
#endif
			}
			//多条处理
			bufIndex += ALARMLEN;
		}
		else
		{
			bufIndex ++;
		}
	}
	return true;
}
