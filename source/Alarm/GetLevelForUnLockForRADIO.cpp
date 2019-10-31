#include "GetLevelForUnLockForRADIO.h"
#include "CheckLevelForUnLock.h"
#include "AlarmMgr.h"
#include "AlarmSender.h"
#include "../DBAccess/DBManager.h"

#include "../Foundation/OSFunction.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/AlarmPropManager.h"
#include "../DeviceAccess/RADIOTcpDeviceAccess.h"
#include "../Communications/SysMsgSender.h"
#include "../DeviceAccess/DeviceAccessMgr.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include "../BusinessProcess/RunPlanInfoMgr.h"
extern bool g_unalarmchanscanval[64];
extern bool g_alarmchanscanval[64];
extern time_t g_freqlocktime[64];
extern bool g_freqlockval[64];
extern bool g_freqneedlock[64];
extern bool   g_realqulity;
extern bool   g_atvchanscan;
extern bool   g_realspec;
extern bool   g_realstream;
extern ACE_Thread_Mutex SendDeviceMutex;
GetLevelForUnLockForRADIO::GetLevelForUnLockForRADIO(void)
{
	m_ChannelscanTime = 0;
	InitDeviceSock();
	pAlarmSender = new AlarmSender();
	//
	std::vector<sCheckParam> vecCheckParam;
	DBMANAGER::instance()->QueryAlarmInfo("0",vecCheckParam,true);
	for (int i=0;i!=vecCheckParam.size();++i)
	{
		sCheckParam sCheck=vecCheckParam[i];
		sCheck.mode = "0";
		std::string key=StrUtil::Int2Str(sCheck.DVBType)+std::string("_")+sCheck.Freq+std::string("_")+sCheck.ChannelID+std::string("_")+sCheck.TypeID;
		std::map<std::string,sCheckParam >::iterator ptr=RadioNosignalAlarmSended.find(key);
		if(ptr==RadioNosignalAlarmSended.end())
		{
			RadioNosignalAlarmSended.insert(make_pair(key,sCheck));
		}
	}
}

GetLevelForUnLockForRADIO::~GetLevelForUnLockForRADIO(void)
{
	if(Device!=NULL)
	{
		// delete Device;
	}
	if (pAlarmSender != NULL)
	{
		delete pAlarmSender;
		pAlarmSender = NULL;
	}	 
}
bool GetLevelForUnLockForRADIO::Start()
{
	bFlag = true;
	if (pAlarmSender != NULL)
	{
		pAlarmSender->open(NULL);
	}
	this->open(0);
	return true;
}

int GetLevelForUnLockForRADIO::open( void* )
{
	bFlag=true;
	activate();
	return 0;
}
bool GetLevelForUnLockForRADIO::InitDeviceSock()
{
	Device=NULL;
	string strIP;
	int    port,deviceid;

	std::list<int> devicelist;

	PROPMANAGER::instance()->GetTaskDeviceList(std::string("QualityRealtimeQueryTask"),RADIO,devicelist);

	if (!devicelist.empty())
	{
		deviceid=devicelist.front();
		PROPMANAGER::instance()->GetDeviceIP(deviceid,strIP);
		PROPMANAGER::instance()->GetDeviceCmdPort(deviceid,port);
		std::string strAddress;
		ACE_SOCK_Stream streamHandle;
		//DEVICEACCESSMGR::instance()->GetTsReceiveHandle(deviceid,strAddress,streamHandle,Device);
		Device =new RADIOHTTPDeviceAccess(deviceid,strIP,port);
	}
	return true;
}
int GetLevelForUnLockForRADIO::svc()
{
	//ʹ��tuner�����ز�����
	// OSFunction::Sleep(1);
	// return 0;
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)¼��Ƶ��(FM)��ƽ��ȡ�߳̿�ʼִ�� !\n"));
	std::list<int> devicedlist;
	std::list<int> devicedlist1;
	PROPMANAGER::instance()->GetTaskDeviceList(std::string("AutoRecord"), RADIO,devicedlist);	
	PROPMANAGER::instance()->GetTaskDeviceList(std::string("TaskRecord"), RADIO,devicedlist1);
// 	PROPMANAGER::instance()->GetTaskDeviceList(std::string("AutoRecord"), AM,devicedlist);	
// 	PROPMANAGER::instance()->GetTaskDeviceList(std::string("TaskRecord"), AM,devicedlist1);

	std::list<int>::iterator ptr1=devicedlist1.begin();		

// 	while(true)
// 	{
// 		OSFunction::Sleep(100);
// 		ACE_DEBUG ((LM_DEBUG,"(%T | %t)¼��Ƶ��(FM)��ƽ��ȡ�߳��ѹر� !\n"));
// 	}
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
	
	int ch[64]={0};

	SpecialRadioRetMessage_Obj Retobj;
	Retobj.len = 0;

	std::map<int,std::list<Tune_Signal_Status_Radio_t>> VSTuner_Quality_ResultMap;
	time_t ScanFreqTime = time(0)+300;//ÿ�������Ƶ���豸��һ��Ƶ��
	vector<string> v_IPDevice;
	VSTuner_Quality_Result_Obj oldretObjArr[100];
	memset(oldretObjArr, 0, sizeof(VSTuner_Quality_Result_Obj)*100);
	int usnNUM = 0;
	while(bFlag)
	{
		/*
		ÿci��Ƶ���豸��һ��Ƶ��/Ƶ��ɨ�裬Ƶ��ɨ��Ľ����tuner�ɼ���ָ����ж�Ӧ
		1��Ƶ��ɨ����������Ƶ�ʶ�Ӧtuner��level��ֵ����Ϊ�����źŵ�ֵ
		2���������������Ƶ�ʣ���Ӧtuner�е�levelֵ����Ϊ�����źŵ�ֵ
		ÿ�������ж�1��tuner��levelֵ
		3�����źţ��ź�ǿ�ȱ�Сֵ����8db����Ϊ�ǲ���������
		4�����źţ��ź�ǿ�ȱ�󣬳���8db��Ϊ��������
		Ƶ��������levelֵ��ӳ�䣿
		1��ʹ�ý��ջ���Ƶ��ɨ��õ���ֵ����tunerlevelֵ���������㣬��tunerlevelֵ���Ӽ�������ΪM����ʹ��ֵ��ͬ
		2������tunnerȡ�õ�levelֵͳһ��/��M,��ָ���У׼
		3�����ƶȵ�ֵ�Ĳ�����levelһ����������ʵ�֣�
		*/
		string retT="";
		retT.append(string(TimeUtil::GetCurTime())).append("AA-");
		if(AlarmRecordInfo.size()==0)
		{
			OSFunction::Sleep(0,50);
			continue;
		}
		std::list<int>::iterator ptr=devicedlist.begin();
		bool Rtn = false;
		bool bNeedScan = false;
		bool nextScanFlag = true;
		v_IPDevice.clear();

		for(;ptr!=devicedlist.end();ptr++)
		{
			sTaskInfo taskinfo;
			VSTuner_Quality_Result_Obj nowretObj;
			nowretObj.status = 0; 
			Rtn = BUSINESSLAYOUTMGR::instance()->QueryRunTaskInfo(*ptr,taskinfo);
			string temp_ip("");
			PROPMANAGER::instance()->GetDeviceIP(*ptr,temp_ip);
			bool isFindIP = false;
			for(std::vector<std::string>::iterator ipIter= v_IPDevice.begin(); ipIter!= v_IPDevice.end(); ipIter++)
			{
				if(*ipIter == temp_ip)
				{
					isFindIP = true;
					break;
				}
			}

			if(Rtn&&nextScanFlag) 
			{
				
				nextScanFlag = false;
				if(!g_atvchanscan && !g_realspec)
				{
					//APPLOG::instance()->WriteLog(DEVICE,LOG_EVENT_WARNNING,string("0"),LOG_OUTPUT_BOTH);
					SendDeviceMutex.acquire();
					DEVICEACCESSMGR::instance()->getQualityInfoFor6U(*ptr, nowretObj);
					SendDeviceMutex.release();
					OSFunction::Sleep(0,100);
					//APPLOG::instance()->WriteLog(DEVICE,LOG_EVENT_WARNNING,string("1"),LOG_OUTPUT_BOTH);
				}
				else
				{
					memcpy(&nowretObj, &oldretObjArr[*ptr], sizeof(VSTuner_Quality_Result_Obj));
				}
			}
			else
			{
				//APPLOG::instance()->WriteLog(DEVICE,LOG_EVENT_WARNNING,string("2"),LOG_OUTPUT_BOTH);
				continue;
			}
			sCheckParam sCheck;	
			/*std::cout<<"ͨ����:" << *ptr;
			std::cout<< " ��ƽֵ:" << nowretObj.tunerQualityObj.level;
			std::cout<< " modulation:" <<nowretObj.tunerQualityObj.modulation;
			std::cout<< " USN:" <<nowretObj.tunerQualityObj.usn;
			std::cout<< " WAM:"  <<nowretObj.tunerQualityObj.wam;
			std::cout<< " offset:" <<nowretObj.tunerQualityObj.offset;
			std::cout<< " bandwith:" <<nowretObj.tunerQualityObj.bandwith<< std::endl;*/
			if(nowretObj.tunerQualityObj.level<50 && (nowretObj.tunerQualityObj.usn>= 13)\
				 && (nowretObj.tunerQualityObj.bandwith < 1000))
			{
				usnNUM = 0;
				nowretObj.status = -1; //0 ���� -1���ز� -2������
				if(*ptr==4)
				{
					string smsg = string("////[3-���ز�]===Ƶ�㣺") + taskinfo.freq + string("��ƽֵ��") + StrUtil::Int2Str(nowretObj.tunerQualityObj.level) +string("---usn:")+StrUtil::Int2Str(nowretObj.tunerQualityObj.usn) + string("---bandwith:")+StrUtil::Int2Str(nowretObj.tunerQualityObj.bandwith);
					APPLOG::instance()->WriteLog(DEVICE,LOG_EVENT_WARNNING,smsg,LOG_OUTPUT_BOTH);
				}
			}
			else
			{
				if(*ptr==4)
				{
					string smsg = string("////[3-����]===Ƶ�㣺") + taskinfo.freq + string("��ƽֵ��") + StrUtil::Int2Str(nowretObj.tunerQualityObj.level) +string("---usn:")+StrUtil::Int2Str(nowretObj.tunerQualityObj.usn) + string("---bandwith:")+StrUtil::Int2Str(nowretObj.tunerQualityObj.bandwith);
					APPLOG::instance()->WriteLog(DEVICE,LOG_EVENT_WARNNING,smsg,LOG_OUTPUT_BOTH);
				}
			}
			if(nowretObj.status == -1)
			{//�ޱ�����������
				sCheck.AlarmType	= ALARM_PROGRAM;
				nowretObj.status = -1; //0 ���� -1���ز� -2������

				sCheck.DVBType		= taskinfo.dvbtype;
				sCheck.ChannelID	= taskinfo.channelid;
				sCheck.Freq			= taskinfo.freq;
				sCheck.STD			= "";
				sCheck.SymbolRate	= "";
				sCheck.TypeDesc		= string("���ز�");
				sCheck.TypedValue	= "";
				sCheck.TypeID		= "0xE";
				sCheck.mode	= "0";//����
				sCheck.DeviceID		= StrUtil::Int2Str(taskinfo.deviceid);
				sCheck.CheckTime	= time(0);
				g_freqlockval[taskinfo.deviceid] = false;
				ALARMMGR::instance()->CheckAlarm(sCheck,true);
			}
			else
			{
				g_freqlockval[taskinfo.deviceid] = true;
				if(/*(nowretObj.tunerQualityObj.usn<= 10)&&*/(nowretObj.tunerQualityObj.modulation<= 2))
				{
					//std::cout <<time(0)<< " ID: " << *ptr << " modulation: " << nowretObj.tunerQualityObj.modulation << std::endl;
					//retT.append(string(TimeUtil::GetCurTime())).append("F2-");
					// sCheck.TypeDesc		= string("������");
					sCheck.AlarmType	= ALARM_PROGRAM;
					sCheck.DVBType		= taskinfo.dvbtype;
					sCheck.ChannelID	= taskinfo.channelid;
					sCheck.Freq			= taskinfo.freq;
					sCheck.DeviceID		= StrUtil::Int2Str(taskinfo.deviceid);
					sCheck.TypedValue	= "";
					sCheck.TypeID		= "0x10";
					sCheck.TypeDesc = "������";
					sCheck.CheckTime	= time(0);
					//ALARMMGR::instance()->CheckAlarm(sCheck,true);
				}
			}
			nextScanFlag = true;
			memcpy(&oldretObjArr[*ptr], &nowretObj, sizeof(VSTuner_Quality_Result_Obj));
		}
		OSFunction::Sleep(0,300);
	}
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)¼��Ƶ��(FM)��ƽ��ȡ�߳�ִֹͣ�� !\n"));
	return 0;
}
int GetLevelForUnLockForRADIO::Stop()
{
	bFlag=false;
	this->wait();
	return 0;
}
bool GetLevelForUnLockForRADIO::AddRecordInfo(sSignalCheck Info)
{
	CheckMutex.acquire();
	AlarmRecordInfo.push_back(Info);
	CheckMutex.release();
	return true;
}
bool GetLevelForUnLockForRADIO::RemoveRecordInfo(sSignalCheck Info)
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

int GetLevelForUnLockForRADIO::GetLevel(std::string strfreq)
{
	float freq = StrUtil::Str2Float(strfreq);
	int f_value = 0;

// 	string scantype=PROPMANAGER::instance()->GetScanType(RADIO);
// // 	if(Device!=NULL&&scantype=="2")		//���������Ŀ�������⴦���ӿ�·����ָ�꿨��ù㲥�ĵ�ƽֵ
// // 	{
// // 		int temfreq = (int)(freq*1000.0);
// // 		int level = Device->SendCmdToTVCom(MSG_GET_QUA,(void *)&temfreq,sizeof(int));
// // 		if(level>0)
// // 		{
// // 			f_value=level/100;
// // 		}
// // 		else
// // 		{
// // 			f_value=0.0f;
// // 		}
// // 	}
// // 	else 
// 	if (Device!=NULL)
// 	{
// 		RadioQuaRetMessage_Obj rqr;
// 		if(Device->GetQuality(freq,rqr))
// 		{
// 			f_value=rqr.level_int;
// 		}
// 	}

	return f_value;
}

bool GetLevelForUnLockForRADIO::CheckSendAlarm()
{
	std::map<std::string,sCheckParam>::iterator programptr=RadioNosignalAlarm.begin();
	for (;programptr!=RadioNosignalAlarm.end();++programptr)
	{
		string alarmxml;
		sCheckParam param=(*programptr).second;
		std::string key=StrUtil::Int2Str(param.DVBType)+std::string("_")+param.Freq+std::string("_")+param.ChannelID+std::string("_")+param.TypeID;
		if(RUNPLANINFOMGR::instance()->InRunPlan(param.DVBType,param.ChannelID)==false)
		{
			//
			std::map<std::string,sCheckParam >::iterator ptr=RadioNosignalAlarmSended.find(key);
			if(ptr!=RadioNosignalAlarmSended.end())
			{
				if((*ptr).second.mode == "0")
				{
					(*ptr).second.CheckTime = time(0);
					DBMANAGER::instance()->QueryFreqAlarmID((*ptr).second.DVBType,(*ptr).second.DeviceID,(*ptr).second.Freq,(*ptr).second.TypeID,(*ptr).second.AlarmID);
					DBMANAGER::instance()->UpdateAlarmInfo((*ptr).second.DVBType,(*ptr).second.AlarmID,"1",TimeUtil::DateTimeToStr((*ptr).second.CheckTime));
					DBMANAGER::instance()->UpdateAlarmRecordExpireTime((*ptr).second.DVBType,(*ptr).second.AlarmID);
					ALARMMGR::instance()->CreateAlarmXML((*ptr).second,"1",(*ptr).second.AlarmID,alarmxml);
					if(alarmxml.length()>0)
					{
						ACE_Message_Block *MBALARM = new ACE_Message_Block(alarmxml.length());
						memcpy(MBALARM->wr_ptr(),alarmxml.c_str(),alarmxml.length());
						MBALARM->wr_ptr(alarmxml.length());
						pAlarmSender->putq(MBALARM);//������xml��ӵ��ϱ�������
					}
					RadioNosignalAlarmSended.erase(ptr);
				}
			}
			//
			(*programptr).second.CheckTime = time(0);
			Sleep(10);
			continue;
		}
		time_t curtime=time(0);
		long alarmid=0;

		//��ȡ��̬����ʱ����������ڽ�ʵ��ʱ��ǰ��
		sAlarmParam alarmparam;
		bool ret = ALARMMGR::instance()->GetAlarmParm(param,alarmparam);
		if (ret == false)
		{
			alarmparam.Duration = "5";
		}
		if(param.mode == "1")//�����������ʱ��4��
		{
			alarmparam.Duration = "4";
		}
		int duration=StrUtil::Str2Int(alarmparam.Duration);
		if(curtime-param.CheckTime >= duration)
		{
			bool bSend = true;
			std::map<std::string,sCheckParam >::iterator ptr=RadioNosignalAlarmSended.find(key);
			if(ptr==RadioNosignalAlarmSended.end())
			{
				if(param.mode == "0")
				{
					RadioNosignalAlarmSended.insert(make_pair(key,param));
				}
				else
				{
					bSend = false;
				}
			}
			else
			{
				if((*ptr).second.mode != param.mode)
				{
					(*ptr).second = param;
				}
				else
				{
					bSend = false;
				}
			}
			//
			if(bSend)
			{
				string alarmxml;
				long alarmid=0;
				switch(StrUtil::Str2Int(param.mode))	//����Ƿ񱨾�
				{
				case 0:		//����
					PROPMANAGER::instance()->GetAlarmID(alarmid);
					param.AlarmID=StrUtil::Long2Str(alarmid);
					//param.CheckTime=curtime - duration;
					DBMANAGER::instance()->AddAlarmInfo(param,"0");
					ALARMMGR::instance()->CreateAlarmXML(param,"0",StrUtil::Long2Str(alarmid),alarmxml);
					ALARMMGR::instance()->CheckAlarm(param,true);
					break;
				case 1:		//�ⱨ 
					//param.CheckTime=curtime - duration;
					DBMANAGER::instance()->QueryFreqAlarmID(param.DVBType,param.DeviceID,param.Freq,param.TypeID,param.AlarmID);
					DBMANAGER::instance()->UpdateAlarmInfo(param.DVBType,param.AlarmID,"1",TimeUtil::DateTimeToStr(param.CheckTime));
					DBMANAGER::instance()->UpdateAlarmRecordExpireTime(param.DVBType,param.AlarmID);
					ALARMMGR::instance()->CreateAlarmXML(param,"1",param.AlarmID,alarmxml);
					ALARMMGR::instance()->CheckAlarm(param,true);
					//
					break;
				default:
					break;
				}
				if(alarmxml.length()>0)
				{
					ACE_Message_Block *MBALARM = new ACE_Message_Block(alarmxml.length());
					memcpy(MBALARM->wr_ptr(),alarmxml.c_str(),alarmxml.length());
					MBALARM->wr_ptr(alarmxml.length());
					pAlarmSender->putq(MBALARM);//������xml��ӵ��ϱ�������
				}
			}
			//
		}
	}
	return true;
}
