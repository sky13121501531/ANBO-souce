#include "AlarmMgr.h"
#include "AlarmSender.h"
#include "ReSendAlarm.h"
#include "../BusinessProcess/AlarmParamInfoMgr.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/AlarmPropManager.h"
#include "../Communications/SysMsgSender.h"
#include "../DBAccess/DBManager.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../BusinessProcess/RunPlanInfoMgr.h"
#include "../DeviceAccess/TCPDeviceAccess.h"
#include "../Communications/CommunicationMgr.h"
#include "CreateAlarmXMLForAM.h"
#include "CreateAlarmXMLForATV.h"
#include "CreateAlarmXMLForRadio.h"
#include "CreateAlarmXMLForDVBC.h"
#include "CreateAlarmXMLForCTTB.h"
#include "CreateAlarmXMLForDVBS.h"
#include "CreateAlarmXMLForCTV.h"
#include "CreateAlarmXMLForTHREED.h"
#include "../Communications/XMLSend.h"
extern bool g_unalarmchanscanval[64];
time_t timeRelease = time(0);
AlarmMgr::AlarmMgr(void)
{
	AlarmID="";
	bFlag=false;
	pAlarmSender = new AlarmSender();
	pAlarmReSender = new ReSendAlarm();
	std::vector<sCheckParam> vecCheckParam;

	SeperateTypeID StypeID;
//	DBMANAGER::instance()->QueryAlarmPriority(vecAlarmPriority);
	DBMANAGER::instance()->QueryAlarmInfo("0",vecCheckParam,true);
	DBMANAGER::instance()->QueryAlarmInfo("0",vecCheckParam,false);
	for (int i=0;i!=vecCheckParam.size();++i)                           
	{
		sCheckParam checkparam=vecCheckParam[i];
		std::string paramid;
		if (checkparam.AlarmType==ALARM_FREQ)
		{
			StypeID.head = StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_");
			StypeID.type = checkparam.TypeID;
			paramid=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+checkparam.TypeID;
		}
		else if (checkparam.AlarmType==ALARM_PROGRAM)
		{
			StypeID.head = StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.ChannelID+std::string("_");
			StypeID.type = checkparam.TypeID;
			paramid=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.ChannelID+std::string("_")+checkparam.TypeID;//报警类型标识
			checkparam.mode = "1";/*处理僵尸报警信息--重启主程序*/
			CheckAlarm(checkparam,false);	
		}
		else /*if (checkparam.AlarmType==ALARM_EQUIPMENT)*/
		{
            if(checkparam.TypeID == "11" || checkparam.TypeID == "12"|| checkparam.TypeID == "13"|| checkparam.TypeID == "14")
            {
                paramid=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.TypeID;
            }
            else if(checkparam.TypeID == "1"|| checkparam.TypeID == "2"|| checkparam.TypeID == "3")
            {
                paramid=checkparam.DeviceID+std::string("_")+checkparam.TypeID;
            }
            AddAlarm(paramid);
		}
		//APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_DEBUG,string("加载报警:"+paramid),LOG_OUTPUT_FILE);
		vecTypeID.push_back(paramid);
		if(StypeID.head.size()>0)
			vecSeperateTypeID.push_back(StypeID);
	}
	//
	m_tvdeciceid = PROPMANAGER::instance()->GetAlarmrecTvDeciceid();
	m_radiodeciceid = PROPMANAGER::instance()->GetAlarmrecRadioDeciceid();
	m_expiredays = PROPMANAGER::instance()->GetAlarmrecExpiredays();
	m_reclength = PROPMANAGER::instance()->GetAlarmrecrecLength();
	//
	m_radiodevalarmunittime = StrUtil::Str2Int(PROPMANAGER::instance()->GetRadiodevalarmUnitTime());
	m_radiodevalarmrate = StrUtil::Str2Float(PROPMANAGER::instance()->GetRadiodevalarmRate());
	m_radiodevalarmunalarmrate = StrUtil::Str2Float(PROPMANAGER::instance()->GetRadiodevalarmUnalarmRate());
}

AlarmMgr::~AlarmMgr(void)
{
	//删除指针，释放内存
	if (pAlarmSender != NULL)
	{
		delete pAlarmSender;
		pAlarmSender = NULL;
	}	 
	if (pAlarmReSender != NULL)
	{
		delete pAlarmReSender;
		pAlarmReSender = NULL;
	}	
	
}
bool AlarmMgr::Start()
{
	if (pAlarmSender == NULL)
		return false;
	//启动报警发送线程
	pAlarmSender->open(NULL);
	if(pAlarmReSender!=NULL)
	{
		pAlarmReSender->open(NULL);
	}
	this->open(0);
	return true;
}

int AlarmMgr::open( void* )
{
	bFlag=true;
	msg_queue()->high_water_mark(1024*10240);
	msg_queue()->low_water_mark(1024*10240);
	activate();
	return 0;
}
int AlarmMgr::svc()
{
	OSFunction::Sleep(30);
	while(bFlag)
	{
		//检查节目报警
		ProgramMutex.acquire();
		std::map<std::string,std::vector<sCheckParam> >::iterator programptr=ProgramAlarm.begin();
		for (;programptr!=ProgramAlarm.end();++programptr)
		{
			if((*programptr).second.size()==0)
			{
				continue;
			}
			sCheckParam param=(*programptr).second[0];
			string alarmxml;
			time_t curtime=time(0);
			int IsProgramAlarm=CheckProgramAlarm(time(0),(*programptr).second,curtime);
			if(IsProgramAlarm == 0)
			{
				if((param.DVBType == RADIO)&&(param.TypeID == "24"))//广播报无载波时不报无伴音
				{
					std::string alarmtemtypeID=StrUtil::Int2Str(param.DVBType)+std::string("_")+param.DeviceID+std::string("_")+param.Freq+std::string("_")+"23";//报警类型标识
					if(IsAlarmed(alarmtemtypeID))
					{
						(*programptr).second.clear();
						continue;
					}
				}
			}
			long alarmid=0;
			std::string typeID=StrUtil::Int2Str(param.DVBType)+std::string("_")+param.DeviceID+std::string("_")+param.ChannelID+std::string("_")+param.TypeID;//报警类型标识
			
			//获取异态持续时间参数，用于将实际时间前移
			sAlarmParam alarmparam;
			bool ret = GetAlarmParm(param,alarmparam);
			if (ret == false)
			{
				alarmparam.Duration = "10";
			}
			int duration=StrUtil::Str2Int(alarmparam.Duration);//异态持续时间

			switch(IsProgramAlarm)	//检查是否报警
			{
			case 0:		//报警
				PROPMANAGER::instance()->GetAlarmID(alarmid);
				param.AlarmID=StrUtil::Long2Str(alarmid);
				param.CheckTime=curtime;
				DBMANAGER::instance()->AddAlarmInfo(param,"0");
				CreateAlarmXML(param,"0",StrUtil::Long2Str(alarmid),alarmxml);
				break;
			case 1:		//解报 
				param.CheckTime=curtime;
				DBMANAGER::instance()->QueryFreqAlarmID(param.DVBType,param.DeviceID,param.Freq,param.TypeID,param.AlarmID);
				DBMANAGER::instance()->UpdateAlarmInfo(param.DVBType,param.AlarmID,"1",TimeUtil::DateTimeToStr(param.CheckTime));
				DBMANAGER::instance()->UpdateAlarmRecordExpireTime(param.DVBType,param.AlarmID);
				CreateAlarmXML(param,"1",param.AlarmID,alarmxml);
				break;
			default:
				break;
			}
			if(alarmxml.length()>0)
			{
				ACE_Message_Block *MBALARM = new ACE_Message_Block(alarmxml.length());
				memcpy(MBALARM->wr_ptr(),alarmxml.c_str(),alarmxml.length());
				MBALARM->wr_ptr(alarmxml.length());
				string retT;
				retT.append(string(TimeUtil::GetCurTime())).append("F2-");
				pAlarmSender->putq(MBALARM);//将报警xml添加到上报队列中
				retT.append(string(TimeUtil::GetCurTime())).append("END2");
			}
		}
		ProgramMutex.release();

		//广播指标报警处理
		RadioQualityAlarmMutex.acquire();
		std::map<std::string,std::vector<sCheckParam> >::iterator pit=RadioQualityAlarm.begin();
		for (;pit!=RadioQualityAlarm.end();++pit)
		{
			if((*pit).second.size()==0)
			{
				continue;
			}
			std::vector<sCheckParam> vecCheckParam=(*pit).second;
			CheckRadioFreqAlarmParam(vecCheckParam);
		}
		RadioQualityAlarmMutex.release();
		//
		OSFunction::Sleep(0,200);
	}
	return 0;
}
int AlarmMgr::Stop()
{
	bFlag=false;
	if (pAlarmSender == NULL)
		return false;
	//停止报警发送线程
	pAlarmSender->Stop();

	if(pAlarmReSender!=NULL)
		pAlarmReSender->Stop();
	this->wait();
	return 0;
}
bool AlarmMgr::CheckAlarm(sCheckParam& checkparam,bool IsConvert)
{
	bool ret = false;//初始化返回值
	sAlarmParam alarmparam;
	std::string alarmxml;
	std::string tmpTypeID;
	std::string tempswith="";
	//
	if(checkparam.AlarmType==ALARM_PROGRAM)
	{
		if((checkparam.DVBType == RADIO)&&(checkparam.TypeID == "0x10"))//无伴音
		{
			ProgramMutex.acquire();
			std::string alarmtemtypeID=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+"23";//报警类型标识
			if(IsAlarmed(alarmtemtypeID))
			{
				ProgramMutex.release();
				return true;
			}
			ProgramMutex.release();
		}
		//
	}
    else if(checkparam.AlarmType==ALARM_EQUIPMENT)
    {
        std::string paramid;
        if(checkparam.TypeID == "11" || checkparam.TypeID == "12"|| checkparam.TypeID == "13"|| checkparam.TypeID == "14")
        {
            paramid=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.TypeID;
        }
        else if(checkparam.TypeID == "1"|| checkparam.TypeID == "2"|| checkparam.TypeID == "3")
        {
            paramid=checkparam.DeviceID+std::string("_")+checkparam.TypeID;
        }

        if(checkparam.mode == "1")
        {
            if(IsAlarmed(paramid))//正常已报过警
            {
                ReleaseAlarm(paramid);//解除报警
                sCheckParam tmpParam=checkparam;
                DBMANAGER::instance()->QueryFreqAlarmID(checkparam.DVBType,checkparam.DeviceID,checkparam.Freq,checkparam.TypeID,tmpParam.AlarmID);
                tmpParam.CheckTime=time(0);
                DBMANAGER::instance()->UpdateAlarmInfo(tmpParam.DVBType,tmpParam.AlarmID,"1",TimeUtil::DateTimeToStr(tmpParam.CheckTime));
                if(checkparam.TypeID == "11" || checkparam.TypeID == "12"|| checkparam.TypeID == "13"|| checkparam.TypeID == "14")
                {
                    CreateAlarmXML(tmpParam,"1",tmpParam.AlarmID,alarmxml);//构造解除报警xml
                }
                else if(checkparam.TypeID == "1"|| checkparam.TypeID == "2"|| checkparam.TypeID == "3")
                {
                    CreateEQUIPMENTAlarmXML(tmpParam,alarmxml);
                }

				OSFunction::ExitProcess("板卡恢复");
            }
        }
        else if (checkparam.mode == "0")
        {
            if (!IsAlarmed(paramid))//不正常未报警
            { 
                AddAlarm(paramid);//添加报警
                long alarmid=0;
                PROPMANAGER::instance()->GetAlarmID(alarmid);
                sCheckParam tmpParam=checkparam;
                tmpParam.AlarmID=StrUtil::Long2Str(alarmid);
                tmpParam.CheckTime=time(0);
                DBMANAGER::instance()->AddAlarmInfo(tmpParam,"0");
                if(checkparam.TypeID == "11" || checkparam.TypeID == "12"|| checkparam.TypeID == "13"|| checkparam.TypeID == "14")
                {
                    CreateAlarmXML(tmpParam,"0",StrUtil::Long2Str(alarmid),alarmxml);//构造报警xml
                }
                else if(checkparam.TypeID == "1"|| checkparam.TypeID == "2"|| checkparam.TypeID == "3")
                {
                    CreateEQUIPMENTAlarmXML(tmpParam,alarmxml);
                }
            }
        }
        ret = true;
    }

	//映射报警类型ID
	if(IsConvert)
	{
		switch(checkparam.AlarmType)//根据报警类型检测是否报警，并生成上报的xml
		{
		case ALARM_ENVIRONMENT:
			{
				StrUtil::ConvertEnvTypeIDToAlarmID(checkparam);//环境报警
				break;
			}
		case ALARM_FREQ:
			{
				StrUtil::ConvertQualityTypeIDToAlarmID(checkparam);//频点报警
				break;
			}
		case ALARM_PROGRAM:
			{
				StrUtil::ConvertProgramTypeIDToAlarmID(checkparam);
				break;
			}
		case ALARM_TR101_290:
			{
				StrUtil::ConvertETR290TypeIDToAlarmID(checkparam);
				break;
			}
		default:
			break;
		}
	}

	//获取报警参数
	if(checkparam.AlarmType == ALARM_FREQ || checkparam.AlarmType == ALARM_PROGRAM || checkparam.AlarmType == ALARM_TR101_290)
	{
		ret = GetAlarmParm(checkparam,alarmparam);
		if (ret==false)
			return false;
	}
	switch(checkparam.AlarmType)//根据报警类型检测是否报警，并生成上报的xml
	{
	case ALARM_ENVIRONMENT:
		{
			ret = CheckEnvironmenAlarmParam(checkparam,alarmparam,alarmxml);	//环境报警
			break;
		}
	case ALARM_FREQ:
		{
			if (checkparam.DVBType == ATV || checkparam.DVBType == CTV)
			{
				ret = CheckFreqAlarmParam(checkparam,alarmparam,alarmxml);			//频点报警
			}
			else if (checkparam.DVBType==RADIO || checkparam.DVBType==AM)
			{
				if(checkparam.TypeID=="1"||checkparam.TypeID=="2")
				{
					ret = CheckFreqAlarmParam(checkparam,alarmparam,alarmxml);			//频点报警
				}
			}
			break;
		}
	case ALARM_PROGRAM:
		{
			ProgramMutex.acquire();		
			std::string key=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+checkparam.ChannelID+std::string("_")+checkparam.TypeID;
			std::map<std::string,std::vector<sCheckParam> >::iterator ptr=ProgramAlarm.find(key);
			if(ptr==ProgramAlarm.end())
			{
				vector<sCheckParam> vecParam;
				vecParam.push_back(checkparam);
				ProgramAlarm.insert(make_pair(key,vecParam));
			}
			else
			{
				ptr->second.push_back(checkparam);
			}
			ProgramMutex.release();
			break;
		}
	case ALARM_TR101_290:
		{
			ETRMutex.acquire();
			std::string key=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+checkparam.TypeID;
			std::map<std::string,std::vector<sCheckParam> >::iterator ptr=ETRAlarm.find(key);
			if(ptr==ETRAlarm.end())
			{
				vector<sCheckParam> vecParam;
				vecParam.push_back(checkparam);
				ETRAlarm.insert(make_pair(key,vecParam));
			}
			else
			{
				(*ptr).second.push_back(checkparam);
			}
			ETRMutex.release();
			break;
		}
	default:
		break;
	}

	if(ret==true && alarmxml.length()>0)
	{
		ACE_Message_Block *MBALARM = new ACE_Message_Block(alarmxml.length());
		memcpy(MBALARM->wr_ptr(),alarmxml.c_str(),alarmxml.length());
		MBALARM->wr_ptr(alarmxml.length());
		ret = pAlarmSender->putq(MBALARM);//将报警xml添加到上报队列中
		//SYSMSGSENDER::instance()->SendMsg(alarmxml,checkparam.DVBType,VS_MSG_PROALARM,DATAANALYSER,LOG_EVENT_DEBUG,false,true);
	}
	return ret;
}

bool AlarmMgr::CheckEnvironmenAlarmParam(const sCheckParam& checkparam,sAlarmParam& alarmparam,std::string& alarmxml)
{
	int check = atoi( checkparam.TypedValue.c_str());//测量值
	int downThreshold = atoi( alarmparam.DownThreshold.c_str());//下限值
	int upThreshold = atoi( alarmparam.UpThreshold.c_str() );//上限值

	if(alarmparam.Switch=="0")
		return true;

	if(check>=downThreshold&&check<=upThreshold&&IsAlarmed(StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.TypeID))//正常值已报过警
	{
		std::string typeID=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.TypeID;
		ReleaseAlarm(typeID);//解除报警

		std::string msg="解除报警,TypeID="+checkparam.TypeID;
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_PROALARM,DATAANALYSER,LOG_EVENT_DEBUG,false,true);
		//CreateAlarmXML(checkparam,"1","测量值正常",alarmxml);//构造解除报警xml
		return true;
	}
	if (check<downThreshold&&!IsAlarmed(StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.TypeID))//不正常值未报警
	{
		string reason="小于下限值";
		std::string typeID=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.TypeID;
		AddAlarm(typeID);//添加报警

		std::string msg="添加报警,TypeID="+checkparam.TypeID;
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_PROALARM,DATAANALYSER,LOG_EVENT_DEBUG,false,true);
		//CreateAlarmXML(checkparam,"0",reason,alarmxml);//构造报警xml
		return true;
	}
	if (check>upThreshold&&!IsAlarmed(StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.TypeID))//不正常值未报警
	{
		string reason="大于上限值";
		std::string typeID=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.TypeID;
		AddAlarm(typeID);//添加报警

		std::string msg="添加报警,TypeID="+checkparam.TypeID;
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_PROALARM,DATAANALYSER,LOG_EVENT_DEBUG,false,true);
		//CreateAlarmXML(checkparam,"0",reason,alarmxml);//构造报警xml
		return true;
	}
	return true;
}
bool AlarmMgr::CheckFreqAlarmParam(sCheckParam& checkparam,sAlarmParam& alarmparam,std::string& alarmxml)
{
	float check = atof( checkparam.TypedValue.c_str())/1000;//测量值
	float downThreshold = atof( alarmparam.DownThreshold.c_str());//下限值
	float upThreshold = atof( alarmparam.UpThreshold.c_str() );//上限值

	if(alarmparam.Switch=="0")
		return true;
#if 1 
	UpAlarmInfo alarminfo;
	PROPMANAGER::instance()->GetUpAlarmInfo(RADIO,alarminfo);
	
	if (checkparam.DVBType==RADIO || checkparam.DVBType==AM)
	{
		bool bcurAlarmState = false; //当前报警状态
		if(checkparam.TypeID=="1"||checkparam.TypeID=="2")
		{
			// 停播 41 劣播 42 
			if(checkparam.TypeID== "1")
			{
				//当前为电平值
				if(check>=downThreshold&&check<=upThreshold)
				{
					checkparam.TypeID="42";
				}
				else if(check<downThreshold) //停播
				{
					checkparam.TypeID="41";
				}
				else
				{
					bcurAlarmState = true;
				}
			}
			else if(checkparam.TypeID=="2")
			{ 
				//当前为调制度 停播43 劣播44
				if(check>=downThreshold&&check<=upThreshold)
				{
					checkparam.TypeID = string("44");
				}
				else if(check<downThreshold) //停播
				{
					checkparam.TypeID = string("43");
				}
				else
				{
					bcurAlarmState = true;
				}
			}
			std::string paramid=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+checkparam.TypeID;
			if(bcurAlarmState) //
			{//此处解除报警
				//是否存在报警？ 如果存在 41 42 51 52 的报警解除报警
				bool bIsUnAlarm = false;
				std::string tempPid = "";
				if(checkparam.TypeID== "1")
				{
					std::string paramid1=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+string("41");
					tempPid = paramid1;
					if(IsAlarmed(tempPid))
					{
						checkparam.TypeID = "41";
						paramid = tempPid;
						bIsUnAlarm = true;
					}
					std::string paramid2=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+string("42");
					tempPid = paramid2;
					if(IsAlarmed(tempPid))
					{
						checkparam.TypeID = "42";
						paramid = tempPid;
						bIsUnAlarm = true;
					}
				}
				else if(checkparam.TypeID== "2")
				{
					std::string paramid3=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+string("43");
					tempPid = paramid3;
					if(IsAlarmed(tempPid))
					{
						checkparam.TypeID = "43";
						paramid = tempPid;
						bIsUnAlarm = true;
					}
					std::string paramid4=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+string("44");
					tempPid = paramid4;
					if(IsAlarmed(tempPid))
					{
						checkparam.TypeID = "44";
						paramid = tempPid;
						bIsUnAlarm = true;
					}
				}
				if(bIsUnAlarm)
				{
					ReleaseAlarm(paramid);//解除报警
					sCheckParam tmpParam=checkparam;
					DBMANAGER::instance()->QueryFreqAlarmID(checkparam.DVBType,checkparam.DeviceID,checkparam.Freq,checkparam.TypeID,tmpParam.AlarmID);
					tmpParam.CheckTime=time(0);
					DBMANAGER::instance()->UpdateAlarmInfo(tmpParam.DVBType,tmpParam.AlarmID,"1",TimeUtil::DateTimeToStr(tmpParam.CheckTime));
					CreateAlarmXML(tmpParam,"1",tmpParam.AlarmID,alarmxml);//构造解除报警xml
					SYSMSGSENDER::instance()->SendMsg(alarmxml,tmpParam.DVBType,VS_MSG_PROALARM,DATAANALYSER,LOG_EVENT_DEBUG,false,true);	
				}
				return true;
			}
			else if(checkparam.TypeID=="44"||checkparam.TypeID=="42") //此处处理劣播报警 
			{//此处报劣播
				//如果已经上报劣播报警，则不再重复上报
				//上报劣播报警则不上报
				if(IsAlarmed(paramid))
				{
					return true;
				}
				// 如果报停播 解除停播 报劣播
				std::string tempPid = "";
				std::string paramid1 = "";
				if(checkparam.TypeID=="42")
				{
					paramid1=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+string("41");
				}
				else
				{
					paramid1=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+string("43");
				}
				tempPid = paramid1;
				if(IsAlarmed(tempPid))
				{
					//paramid = tempPid;
					ReleaseAlarm(tempPid);//解除报警
					sCheckParam tmpParam=checkparam;
					if(checkparam.TypeID=="42") 
						DBMANAGER::instance()->QueryFreqAlarmID(checkparam.DVBType,checkparam.DeviceID,checkparam.Freq,"41",tmpParam.AlarmID);
					else
						DBMANAGER::instance()->QueryFreqAlarmID(checkparam.DVBType,checkparam.DeviceID,checkparam.Freq,"43",tmpParam.AlarmID);
					tmpParam.CheckTime=time(0);
					DBMANAGER::instance()->UpdateAlarmInfo(tmpParam.DVBType,tmpParam.AlarmID,"1",TimeUtil::DateTimeToStr(tmpParam.CheckTime));
					CreateAlarmXML(tmpParam,"1",tmpParam.AlarmID,alarmxml);//构造解除报警xml
					//SYSMSGSENDER::instance()->SendMsg(alarmxml,tmpParam.DVBType,VS_MSG_PROALARM,DATAANALYSER,LOG_EVENT_DEBUG,false,true);
					XMLSend::SendXML(alarminfo.alarmurl,alarmxml);
				}

				AddAlarm(paramid);//添加报警
				long alarmid=0;
				PROPMANAGER::instance()->GetAlarmID(alarmid);

				sCheckParam tmpParam=checkparam;
				tmpParam.AlarmID=StrUtil::Long2Str(alarmid);
				tmpParam.CheckTime=time(0);
				tmpParam.TypeDesc ="调制度劣播";
				if(tmpParam.TypeID=="42")
					tmpParam.TypeDesc ="电平劣播";

				DBMANAGER::instance()->AddAlarmInfo(tmpParam,"0");
				CreateAlarmXML(tmpParam,"0",StrUtil::Long2Str(alarmid),alarmxml);//构造报警xml

				SYSMSGSENDER::instance()->SendMsg(alarmxml,tmpParam.DVBType,VS_MSG_PROALARM,DATAANALYSER,LOG_EVENT_DEBUG,false,true);
				return true;
			}
			else if(checkparam.TypeID=="43"||checkparam.TypeID=="41") //此处处理劣播报警 
			{//此处报停播
				if(IsAlarmed(paramid)) //如果已报则不重复上报
				{
					return true;
				}
				// 如果报劣播 解除劣播 报停播
				std::string tempPid = "";
				std::string paramid1 = "";
				if(checkparam.TypeID=="41") //如果触发 电平停播 查看是否存在劣播报警
					paramid1=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+string("42");
				else
					paramid1=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+string("44");


				tempPid = paramid1;
				if(IsAlarmed(tempPid)) //如果触发 电平停播 查看是否存在劣播报警 
				{
					//paramid = tempPid;
					ReleaseAlarm(tempPid);//解除报警
					sCheckParam tmpParam=checkparam;
					if(checkparam.TypeID=="41") 
						DBMANAGER::instance()->QueryFreqAlarmID(checkparam.DVBType,checkparam.DeviceID,checkparam.Freq,"42",tmpParam.AlarmID);
					else
						DBMANAGER::instance()->QueryFreqAlarmID(checkparam.DVBType,checkparam.DeviceID,checkparam.Freq,"44",tmpParam.AlarmID);
					tmpParam.CheckTime=time(0);
					DBMANAGER::instance()->UpdateAlarmInfo(tmpParam.DVBType,tmpParam.AlarmID,"1",TimeUtil::DateTimeToStr(tmpParam.CheckTime));
					CreateAlarmXML(tmpParam,"1",tmpParam.AlarmID,alarmxml);//构造解除报警xml
					//SYSMSGSENDER::instance()->SendMsg(alarmxml,tmpParam.DVBType,VS_MSG_PROALARM,DATAANALYSER,LOG_EVENT_DEBUG,false,true);
					XMLSend::SendXML(alarminfo.alarmurl,alarmxml);
				}
				//上报停播报警
				AddAlarm(paramid);//添加报警
				long alarmid=0;
				PROPMANAGER::instance()->GetAlarmID(alarmid);

				sCheckParam tmpParam=checkparam;
				tmpParam.AlarmID=StrUtil::Long2Str(alarmid);
				tmpParam.CheckTime=time(0);

				tmpParam.TypeDesc ="调制度停播";
				if(tmpParam.TypeID=="41")
					tmpParam.TypeDesc ="电平停播";

				DBMANAGER::instance()->AddAlarmInfo(tmpParam,"0");
				CreateAlarmXML(tmpParam,"0",StrUtil::Long2Str(alarmid),alarmxml);//构造报警xml

				SYSMSGSENDER::instance()->SendMsg(alarmxml,tmpParam.DVBType,VS_MSG_PROALARM,DATAANALYSER,LOG_EVENT_DEBUG,false,true);
				return true;
			}
			return true;
		}
	}
#else
	if (checkparam.DVBType==RADIO || checkparam.DVBType==AM)
	{
		bool bcurAlarmState = false; //当前报警状态
		if(checkparam.TypeID=="1"||checkparam.TypeID=="2")
		{
			// 停播 41
			if(checkparam.TypeID== "1")
			{
				//当前为电平值
				if(check<downThreshold || check>upThreshold) //停播
				{
					checkparam.mode = "1";
				}
				else
				{
					checkparam.mode = "0";
				}
			}
			else if(checkparam.TypeID=="2")
			{ 
				//当前为调制度 停播51
				if(check<downThreshold || check>upThreshold) //停播
				{
					checkparam.mode = "1";
				}
				else
				{
					checkparam.mode = "0";
				}
			}
			std::string paramid=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+checkparam.TypeID;

			if(checkparam.mode == "1")
			{
				if(IsAlarmed(paramid))
				{
					ReleaseAlarm(paramid);//解除报警
					sCheckParam tmpParam=checkparam;
					DBMANAGER::instance()->QueryFreqAlarmID(checkparam.DVBType,checkparam.DeviceID,checkparam.Freq,checkparam.TypeID,tmpParam.AlarmID);
					tmpParam.CheckTime=time(0);
					DBMANAGER::instance()->UpdateAlarmInfo(tmpParam.DVBType,tmpParam.AlarmID,"1",TimeUtil::DateTimeToStr(tmpParam.CheckTime));
					CreateAlarmXML(tmpParam,"1",tmpParam.AlarmID,alarmxml);//构造解除报警xml
					SYSMSGSENDER::instance()->SendMsg(alarmxml,tmpParam.DVBType,VS_MSG_PROALARM,DATAANALYSER,LOG_EVENT_DEBUG,false,true);	
				}
			}
			else if(checkparam.mode == "0")
			{
				if(!IsAlarmed(paramid)) //如果已报则不重复上报
				{
					//上报停播报警
					AddAlarm(paramid);//添加报警
					long alarmid=0;
					PROPMANAGER::instance()->GetAlarmID(alarmid);
					sCheckParam tmpParam=checkparam;
					tmpParam.AlarmID=StrUtil::Long2Str(alarmid);
					tmpParam.CheckTime=time(0);
					DBMANAGER::instance()->AddAlarmInfo(tmpParam,"0");
					CreateAlarmXML(tmpParam,"0",StrUtil::Long2Str(alarmid),alarmxml);//构造报警xml
					SYSMSGSENDER::instance()->SendMsg(alarmxml,tmpParam.DVBType,VS_MSG_PROALARM,DATAANALYSER,LOG_EVENT_DEBUG,false,true);
				}
			}
			return true;
		}
	}
#endif
	else
	{
		std::string paramid=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+checkparam.TypeID;
		if(check>=downThreshold&&check<=upThreshold&&IsAlarmed(paramid))//正常值已报过警
		{
			ReleaseAlarm(paramid);//解除报警
			sCheckParam tmpParam=checkparam;
			DBMANAGER::instance()->QueryFreqAlarmID(checkparam.DVBType,checkparam.DeviceID,checkparam.Freq,checkparam.TypeID,tmpParam.AlarmID);

			tmpParam.CheckTime=time(0);

			DBMANAGER::instance()->UpdateAlarmInfo(tmpParam.DVBType,tmpParam.AlarmID,"1",TimeUtil::DateTimeToStr(tmpParam.CheckTime));
			CreateAlarmXML(tmpParam,"1",tmpParam.AlarmID,alarmxml);//构造解除报警xml

			SYSMSGSENDER::instance()->SendMsg(alarmxml,tmpParam.DVBType,VS_MSG_PROALARM,DATAANALYSER,LOG_EVENT_DEBUG,false,true);
			return true;
		}
		if ((check<downThreshold||check>upThreshold)&&!IsAlarmed(paramid))//不正常值未报警
		{
			string reason="小于下限值";
			AddAlarm(paramid);//添加报警
			long alarmid=0;
			PROPMANAGER::instance()->GetAlarmID(alarmid);

			sCheckParam tmpParam=checkparam;
			tmpParam.AlarmID=StrUtil::Long2Str(alarmid);
			tmpParam.CheckTime=time(0);

			DBMANAGER::instance()->AddAlarmInfo(tmpParam,"0");
			CreateAlarmXML(tmpParam,"0",StrUtil::Long2Str(alarmid),alarmxml);//构造报警xml

			SYSMSGSENDER::instance()->SendMsg(alarmxml,tmpParam.DVBType,VS_MSG_PROALARM,DATAANALYSER,LOG_EVENT_DEBUG,false,true);
			//AddAlarmRec(checkparam.Freq,1);
			return true;
		}
	}
	return true;
}
bool AlarmMgr::CheckRadioFreqAlarmParam(vector<sCheckParam>& vecParam)
{
	//
	return true;
	//if (vecParam.size()==0)
	//{
	//	return -1;
	//}
	////获取报警参数
	//sAlarmParam param;
	//sCheckParam checkparam=vecParam[0];
	//GetAlarmParm(vecParam[0],param);
	//enumDVBType tempDvbType = vecParam[0].DVBType;
	//std::string tempChanID = vecParam[0].ChannelID;

	//int duration=StrUtil::Str2Int(param.Duration);	//异态持续时间
	//UpAlarmInfo info;
	//PROPMANAGER::instance()->GetUpAlarmInfo(vecParam[0].DVBType,info);
	//std::string typeID=StrUtil::Int2Str(vecParam[0].DVBType)+std::string("_")+vecParam[0].DeviceID+std::string("_")+vecParam[0].ChannelID+std::string("_")+vecParam[0].TypeID;//报警类型标识

	//SeperateTypeID StypeID;
	//StypeID.head = StrUtil::Int2Str(vecParam[0].DVBType)+std::string("_")+vecParam[0].DeviceID+std::string("_")+vecParam[0].ChannelID+std::string("_");
	//StypeID.type = vecParam[0].TypeID;
	////先移除判断时间点回退持续时间之前的异态数据
	//int alarmflag = 0;//报警标识 1报警 2解除报警 其他数字不处理
	//time_t rectime=time(0);
	//time_t curTime=rectime;
	//string mode="-1";
	//vector<sCheckParam>::iterator nextptr;
	//std::string paramid=StrUtil::Int2Str(checkparam.DVBType)+std::string("_")+checkparam.DeviceID+std::string("_")+checkparam.Freq+std::string("_")+checkparam.TypeID;
	//if(StypeID.type=="2")
	//{
	//	int checktime=0;
	//	float userate=0.0f;
	//	int alarmty=0;//1表示报警 2表示解除报警
	//	if(!IsAlarmed(paramid))
	//	{
	//		checktime = duration;
	//		alarmty = 1;
	//		userate = m_radiodevalarmrate;
	//	}
	//	else
	//	{
	//		checktime = m_radiodevalarmunittime;
	//		alarmty = 2;
	//		userate = m_radiodevalarmunalarmrate;
	//	}
	//	int alarmunm=0;
	//	int unalarmnum=0;
	//	bool valid=true;
	//	for (vector<sCheckParam>::iterator ptr = vecParam.begin();ptr!=vecParam.end();)
	//	{
	//		nextptr=ptr;
	//		++nextptr;
	//		if(nextptr==vecParam.end())
	//		{
	//			break;
	//		}
	//		if ((curTime-ptr->CheckTime) > checktime&&(curTime-nextptr->CheckTime) > checktime)
	//		{
	//			ptr = vecParam.erase(ptr);
	//		}
	//		else
	//		{
	//			break;
	//		}
	//	}
	//	for (vector<sCheckParam>::iterator ptr = vecParam.end();ptr!=vecParam.begin();)
	//	{
	//		ptr--;
	//		if(alarmflag != 1 && alarmflag != 2)
	//		{
	//			if(ptr->mode=="0")
	//			{
	//				alarmunm++;
	//			}
	//			else if(ptr->mode=="1")
	//			{
	//				unalarmnum++;
	//			}
	//			if ((curTime-ptr->CheckTime) > m_radiodevalarmunittime)
	//			{
	//				if(valid)
	//				{
	//					if(alarmty == 1)
	//					{
	//						float ra = alarmunm/(alarmunm+unalarmnum);
	//						if(ra>userate)
	//						{
	//							valid = true;
	//						}
	//						else
	//						{
	//							valid = false;
	//						}
	//					}
	//					else if(alarmty == 2)
	//					{
	//						float ra = unalarmnum/(alarmunm+unalarmnum);
	//						if(ra>userate)
	//						{
	//							valid = true;
	//						}
	//						else
	//						{
	//							valid = false;
	//						}
	//					}
	//				}
	//				alarmunm=0;
	//				unalarmnum=0;
	//			}
	//			if ((curTime-ptr->CheckTime) > checktime)
	//			{
	//				if((alarmty == 1)&&valid)
	//				{
	//					alarmflag=1;
	//					break;
	//				}
	//				else if((alarmty == 2)&&valid)
	//				{
	//					alarmflag=2;
	//					break;
	//				}
	//				else
	//				{
	//					alarmflag=0;
	//				}
	//			}
	//			else
	//			{
	//				alarmflag=0;
	//			}
	//		}
	//		if(ptr==vecParam.begin())
	//		{
	//			break;
	//		}
	//	}
	//}
	//else
	//{
	//	for (vector<sCheckParam>::iterator ptr = vecParam.begin();ptr!=vecParam.end();)
	//	{
	//		nextptr=ptr;
	//		++nextptr;
	//		if(nextptr==vecParam.end())
	//		{
	//			break;
	//		}
	//		if ((curTime-ptr->CheckTime) > duration&&(curTime-nextptr->CheckTime) > duration)
	//		{
	//			ptr = vecParam.erase(ptr);
	//		}
	//		else
	//		{
	//			break;
	//		}
	//	}
	//	for (vector<sCheckParam>::iterator ptr = vecParam.begin();ptr!=vecParam.end();)
	//	{
	//		nextptr=ptr;
	//		++nextptr;
	//		if(ptr->mode=="0")
	//		{
	//			if(mode!="0")
	//			{
	//				mode="0";
	//				rectime = ptr->CheckTime;
	//			}
	//			if ((curTime-rectime) > duration)
	//			{
	//				alarmflag=1;
	//			}
	//			else
	//			{
	//				alarmflag=0;
	//			}
	//		}
	//		else if(ptr->mode=="1")
	//		{
	//			if(mode!="1")
	//			{
	//				mode="1";
	//				rectime = ptr->CheckTime;
	//			}
	//			if ((curTime-rectime) > 5)
	//			{
	//				alarmflag=2;
	//			}
	//			else
	//			{
	//				alarmflag=0;
	//			}
	//		}
	//		if(nextptr==vecParam.end())
	//		{
	//			if(ptr->mode==mode)
	//			{
	//				ptr->CheckTime=rectime;
	//			}
	//			break;
	//		}
	//		ptr = vecParam.erase(ptr);
	//		continue;
	//	}
	//}
	//string alarmxml;
	//if (vecParam.size()>0 && alarmflag==1 && !IsAlarmed(paramid))
	//{
	//	string reason="小于下限值";
	//	AddAlarm(paramid);//添加报警
	//	long alarmid=0;
	//	PROPMANAGER::instance()->GetAlarmID(alarmid);

	//	sCheckParam tmpParam=checkparam;
	//	tmpParam.AlarmID=StrUtil::Long2Str(alarmid);
	//	tmpParam.CheckTime=time(0)-duration;
	//	
	//	DBMANAGER::instance()->AddAlarmInfo(tmpParam,"0");
	//	CreateAlarmXML(tmpParam,"0",StrUtil::Long2Str(alarmid),alarmxml);//构造报警xml
	//	//
	//	if(alarmxml.length()>0)
	//	{
	//		ACE_Message_Block *MBALARM = new ACE_Message_Block(alarmxml.length());
	//		memcpy(MBALARM->wr_ptr(),alarmxml.c_str(),alarmxml.length());
	//		MBALARM->wr_ptr(alarmxml.length());
	//		pAlarmSender->putq(MBALARM);//将报警xml添加到上报队列中

	//		SYSMSGSENDER::instance()->SendMsg(alarmxml,param.DVBType,VS_MSG_PROALARM,DATAANALYSER,LOG_EVENT_DEBUG,false,true);
	//	}
	//	//
	//	//AddAlarmRec(checkparam.Freq,2);
	//}
	//else if (vecParam.size()>0 && alarmflag==2 && IsAlarmed(paramid))
	//{
	//	ReleaseAlarm(paramid);//解除报警
	//	sCheckParam tmpParam=checkparam;
	//	DBMANAGER::instance()->QueryFreqAlarmID(checkparam.DVBType,checkparam.DeviceID,checkparam.Freq,checkparam.TypeID,tmpParam.AlarmID);
	//	
	//	tmpParam.CheckTime=time(0)-6;

	//	DBMANAGER::instance()->UpdateAlarmInfo(tmpParam.DVBType,tmpParam.AlarmID,"1",TimeUtil::DateTimeToStr(tmpParam.CheckTime));
	//	CreateAlarmXML(tmpParam,"1",tmpParam.AlarmID,alarmxml);//构造解除报警xml
	//	//
	//	if(alarmxml.length()>0)
	//	{
	//		ACE_Message_Block *MBALARM = new ACE_Message_Block(alarmxml.length());
	//		memcpy(MBALARM->wr_ptr(),alarmxml.c_str(),alarmxml.length());
	//		MBALARM->wr_ptr(alarmxml.length());
	//		pAlarmSender->putq(MBALARM);//将报警xml添加到上报队列中

	//		SYSMSGSENDER::instance()->SendMsg(alarmxml,param.DVBType,VS_MSG_PROALARM,DATAANALYSER,LOG_EVENT_DEBUG,false,true);
	//	}
	//}
	//return 1;
}
int AlarmMgr::CheckETRAlarm( time_t curTime,vector<sCheckParam>& vecParam )//290报警逻辑
{
	if (vecParam.size()==0)
		return -1;

	//获取报警参数
	sAlarmParam param;
	GetAlarmParm(vecParam[0],param);

//	bool ret = GetAlarmParm(vecParam[0],param);
// 	if (ret == false)
// 		return -1;

	int timeinterval=StrUtil::Str2Int(param.TimeInterval);//报警时间间隔
	int errornum=StrUtil::Str2Int(param.Num);//错误次数
	std::string typeID=StrUtil::Int2Str(param.DVBType)+std::string("_")+param.DeviceID+std::string("_")+param.Freq+std::string("_")+param.TypeID;//报警类型标识

	int num=0;//初始化错误次数
	int i=vecParam.size()-1;
	bool bFind=false;//标识是否找到与当前时间间隔大于等于报警时间
	for(;i>=0;--i)
	{
		if(curTime-vecParam[i].CheckTime<=timeinterval)
			num+=StrUtil::Str2Int(vecParam[i].TypedValue);//累加错误
		else
		{
			bFind=true;//找到删除点
			break;
		}
	}
	if(bFind)
	{
		for(int j=0;j<=i;++j)//删除时间点以前的数据
		{
			std::vector<sCheckParam>::iterator ptr=vecParam.begin();
			if(ptr!=vecParam.end())
			vecParam.erase(ptr);
		}
	}
	if(num>=errornum&&!IsAlarmed(typeID))//实际错误次数大于报警次数且没报过警
	{
		AddAlarm(typeID);	//添加报警
		return 0;			//需要报警
	}
	if(num<errornum&&bFind&&IsAlarmed(typeID))//没有达到报警次数但找到时间点
	{
		ReleaseAlarm(typeID);//解除报警
		return 1;
	}
	return -1;
}

int AlarmMgr::CheckProgramAlarm( time_t curTime,vector<sCheckParam>& vecParam,time_t& alarmtime )
{
	if (vecParam.size()==0)
		return -1;


	//获取报警参数
	sAlarmParam param;
	sCheckParam tempparam=vecParam[0];
	GetAlarmParm(vecParam[0],param);
	enumDVBType tempDvbType = vecParam[0].DVBType;
	std::string tempChanID = vecParam[0].ChannelID;

	int duration=StrUtil::Str2Int(param.Duration);	//异态持续时间
	UpAlarmInfo info;
	PROPMANAGER::instance()->GetUpAlarmInfo(vecParam[0].DVBType,info);
	std::string typeID=StrUtil::Int2Str(vecParam[0].DVBType)+std::string("_")+vecParam[0].DeviceID+std::string("_")+vecParam[0].ChannelID+std::string("_")+vecParam[0].TypeID;//报警类型标识
	SeperateTypeID StypeID;
	StypeID.head = StrUtil::Int2Str(vecParam[0].DVBType)+std::string("_")+vecParam[0].DeviceID+std::string("_")+vecParam[0].ChannelID+std::string("_");
	StypeID.type = vecParam[0].TypeID;
	//先移除判断时间点回退持续时间之前的异态数据
	//ProgramMutex.acquire();
	/*反复上报解除，处理逻辑*/
	int TmpCount=0;
	time_t tmepTime = time(0);
	bool isRelease = false;
	if(IsAlarmed(typeID))/*解报*/
	{
		time_t TmptimeSame = vecParam[0].CheckTime;
		bool isReal = false;
		int TmpNum = 0;
		if(vecParam[0].mode=="1")
		{
			TmpCount = 1;
		}
		for(int i=0;i<vecParam.size();i++)
		{
			if(TimeUtil::DiffSecond(TmptimeSame,vecParam[i].CheckTime)==0)
			{
				TmpNum += StrUtil::Str2Int(vecParam[i].mode);
				if(TmpNum==1)
				{
					TmpNum = 0;
					isReal = true;
				}
			}
			else
			{
				int DiffCount = 1;
				DiffCount = TimeUtil::DiffSecond(vecParam[i].CheckTime,TmptimeSame);
				if(isReal)/*确保解报连续性，超过三秒，按解报3秒*/
				{
					TmpCount += DiffCount;
					if(DiffCount>=3)
					{	
						tmepTime = vecParam[i].CheckTime - TmpCount;
						isRelease = true;
						break;
					}
				}
				else
				{
					TmpCount = 0;
				}
				if(TmpCount>=4)/*时间上连续有报有解4次----解除*/
				{
					tmepTime = vecParam[i].CheckTime - TmpCount;
					isRelease = true;
					break;
				}
				isReal = false;
			}
			TmptimeSame = vecParam[i].CheckTime;
		}
	}
	time_t alarmlasttime = time(0);
	time_t alarmfirsttime = time(0);
    string strmode = "1";
	for (vector<sCheckParam>::iterator ptr = vecParam.begin();ptr!=vecParam.end();)
	{
		alarmlasttime = ptr->CheckTime;
        strmode = ptr->mode;
		if ((time(0)-ptr->CheckTime) > duration)
		{
			ptr = vecParam.erase(ptr);
			continue;
		}
		++ptr;
	}
	if(vecParam.size()>0 && !IsAlarmed(typeID))/*上报逻辑*/
	{
		int Ncount = 0;
		for(int i=0;i<vecParam.size();i++)
		{
			if(vecParam[i].mode=="1")
			{
				Ncount = i + 1;
			}
		}
		if(Ncount>0)
		{
			vector<sCheckParam>::iterator tempfirstptr = vecParam.begin();
			vecParam.erase(tempfirstptr,tempfirstptr + Ncount);
		}
		if (vecParam.size()==0)
			return -1;
		vector<sCheckParam>::iterator firstptr = vecParam.begin();
		alarmfirsttime = firstptr->CheckTime;
	}
	if(isRelease && IsAlarmed(typeID))
	{
		if(strmode == "0")
		{
			strmode = "1";
		}
		alarmlasttime = tmepTime;
	}
	//ProgramMutex.release();
	if(info.onceday=="1")
	{
		string strtime=TimeUtil::GetCurTime();
		string daytime=TimeUtil::GetCurDate();
		long nowTime=TimeUtil::StrToSecondTime(strtime);
		time_t thistime=time(0);
		long daysecond=3600*24;
		if((daysecond-nowTime)<=2)
		{	
			//ProgramMutex.acquire();
			vecParam.clear();
			//ProgramMutex.release();
			if(IsAlarmed(typeID))
			{
				string strnowtime=daytime+" "+"23:59:59";
				alarmtime=TimeUtil::StrToDateTime(strnowtime)+duration;
				ReleaseAlarm(typeID);
				return 1;
			}
		}	
		else if(nowTime<=2)
		{
			//ProgramMutex.acquire();
			vecParam.clear();
			//ProgramMutex.release();
			if(IsAlarmed(typeID))
			{
				alarmtime=thistime-nowTime-1+duration;
				ReleaseAlarm(typeID);
				return 1;
			}
		}
	}

	//有异态报警，但是在运行图内，
	if(vecParam.size()>0 && RUNPLANINFOMGR::instance()->InRunPlan(vecParam[0].DVBType,vecParam[0].ChannelID)==false)
	{
		//ProgramMutex.acquire();
		vecParam.clear();
		//ProgramMutex.release();
		if(IsAlarmed(typeID))
		{
			alarmtime=time(0)+duration;
			ReleaseAlarm(typeID);
			return 1;
		}
	}

	//通过报警个数判断报警或者解报
	int ParamNum = 0;					//已有异态个数
	time_t time_last = 0;
	for(int i=0;i<vecParam.size();i++)
	{
		if(TimeUtil::DiffSecond(time_last,vecParam[i].CheckTime)!=0)
		{
			ParamNum++;
		}
		time_last = vecParam[i].CheckTime;
	}
	int AlarmCheckNum = duration;
	if(StypeID.type == "24" || StypeID.type == "11" || StypeID.type == "12" || StypeID.type == "14" || StypeID.type == "18" || (StypeID.type == "13"&&vecParam.size()>0))
	{
		if(StypeID.type == "24" || StypeID.type == "12")
			AlarmCheckNum = AlarmCheckNum * 80/100;
		else
			AlarmCheckNum = AlarmCheckNum * 70/100;
	}
	if((StypeID.type == "10" || StypeID.type == "23") && vecParam.size()>0) //无载波报警
	{
		AlarmCheckNum = AlarmCheckNum*80/100;
	}
	if (vecParam.size()>0 && !IsAlarmed(typeID))
	{
		if((strmode == "0") && (/*ParamNum >= AlarmCheckNum || */(alarmlasttime-alarmfirsttime >= duration)&&(ParamNum>=AlarmCheckNum))&&(time(0)-alarmlasttime<=1))
		{
			alarmtime=time(0)-duration;
			RUNPLANINFOMGR::instance()->CheckAlarmTime(tempDvbType,tempChanID,alarmtime);		//判断报警时间是否在运行图时间内
			alarmtime += duration;
			std::vector<int> vecAlarmTypeID;

			GetAlarmTypeIDEx(StypeID.head,vecAlarmTypeID);
			if(vecAlarmTypeID.size() > 0)
			{
				if(StypeID.type == "11" || StypeID.type == "12")
				{
					for(int i=0;i<vecAlarmTypeID.size();i++)
					{
						if(vecAlarmTypeID[i] == 10)
						{
							return -1;
						}
					}
				}
				else if(StypeID.type == "13")
				{
					for(int i=0;i<vecAlarmTypeID.size();i++)
					{
						if(vecAlarmTypeID[i] == 10 || vecAlarmTypeID[i] == 11 || vecAlarmTypeID[i] == 14 || vecAlarmTypeID[i] == 18 )
						{
							return -1;
						}
					}
				}
				else if(StypeID.type == "14" || StypeID.type == "18")
				{
					for(int i=0;i<vecAlarmTypeID.size();i++)
					{
						if(vecAlarmTypeID[i] == 10 || vecAlarmTypeID[i] == 11)
						{
							return -1;
						}
					}
				}
			}
			if(StypeID.type == "23")
			{
				alarmtime = alarmfirsttime + 2;
			}
			else if(StypeID.type == "10")
			{
				alarmtime = alarmfirsttime;
			}
			else if( StypeID.type == "11")
			{
				alarmtime = alarmfirsttime - 2;
			}
			else
			    alarmtime = alarmfirsttime + 2;
			AddAlarm(typeID);
			cout<<"报警......3:typeid-"<<typeID<<endl;
			return 0;
		}
	}
	else if ((strmode == "1") &&  (IsAlarmed(typeID)) && ((time(0)-alarmlasttime >= 3) || isRelease))
	{
		cout<<"解除报警1111111111111111"<<endl;
		if(StypeID.type == "23")//无载波缓存，业务提3s
		{
			alarmtime = alarmlasttime + 2;
		}
		else if( StypeID.type == "10")
		{
			alarmtime = alarmlasttime;
		}
		else if(StypeID.type == "11")
		{
			alarmtime = alarmlasttime - 3;
		}
		else
		    alarmtime = alarmlasttime + 2;
		ReleaseAlarm(typeID);
		vecParam.clear();
		return 1;
	}
	return -1;
}
bool AlarmMgr::IsAlarmed( std::string typeId )
{
	bool ret=false;
	std::vector<string>::iterator ptr=vecTypeID.begin();
	for (;ptr!=vecTypeID.end();++ptr)//查找typeid是否存在内存中
	{
		if(typeId==(*ptr))
		{
			ret=true;
			break;
		}
	}
	return ret;
}

bool AlarmMgr::ReleaseAlarm( std::string typeId )
{
	ACE_Guard<ACE_Thread_Mutex> guard(MutexTypeID);
	std::vector<string>::iterator ptr=vecTypeID.begin();
	for (;ptr!=vecTypeID.end();++ptr)//找到值为typeid项，删除报警
	{
		if(typeId==(*ptr))
		{
			vecTypeID.erase(ptr);
			break;
		}
	}
	std::vector<SeperateTypeID>::iterator itr= vecSeperateTypeID.begin();
	for(;itr!=vecSeperateTypeID.end();itr++)
	{
		if((*itr).head+(*itr).type==typeId)
		{
			vecSeperateTypeID.erase(itr);
			break;
		}
	}
	return true;
}

bool AlarmMgr::AddAlarm( std::string typeId )
{
	ACE_Guard<ACE_Thread_Mutex> guard(MutexTypeID);
	vecTypeID.push_back(typeId);//添加报警
	size_t pos= typeId.find_last_of("_");
	SeperateTypeID temp;
	temp.type = typeId.substr(pos+1);
	temp.head = typeId.substr(0,pos+1);
	if(temp.head.size()>0)
	vecSeperateTypeID.push_back(temp);
	return true;
}
bool AlarmMgr::AddAlarmID( std::string type,int alarmid )
{
	MutexAlarmID.acquire();
	map<string,int>::iterator ptr=mapAlarmID.find(type);
	if (ptr!=mapAlarmID.end())
	{
		ptr->second=alarmid;
	}
	else
	{
		mapAlarmID.insert(make_pair(type,alarmid));
	}
	MutexAlarmID.release();
	return true;
}

bool AlarmMgr::GetAlarmID( std::string type,int& alarmid )
{
	MutexAlarmID.acquire();
	map<string,int>::iterator ptr=mapAlarmID.find(type);
	if (ptr!=mapAlarmID.end())
	{
		alarmid=ptr->second;
	}
	MutexAlarmID.release();
	return true;

}

bool AlarmMgr::CreateAlarmXML( const sCheckParam& checkparam,std::string mode,std::string alarmid,std::string& alarmxml )
{
	switch(checkparam.AlarmType)//根据监测类型生成不同的节点，并添加节点属性
	{
	case ALARM_ENVIRONMENT://环境报警
		{
			alarmxml = CreateEnvironmentAlarmXML(checkparam,mode);
			break;
		}
	case ALARM_PROGRAM://节目报警
		{
			alarmxml=CreateProgramAlarmXML(checkparam,mode,alarmid);
			break;
		}
	case ALARM_FREQ:
	case ALARM_TR101_290://290
		{
			alarmxml=CreateFreqAlarmXML(checkparam,mode,alarmid);
			break;
		}
    case ALARM_EQUIPMENT:
        {
            alarmxml=CreateEQUIPMENTAlarmXML(checkparam,mode,alarmid);
            break;
        }
	default:
		break;
	}
	return true;
}

std::string AlarmMgr::CreateFreqAlarmXML(const sCheckParam& checkparam,std::string alarmvalue,std::string alarmid)
{
	switch(checkparam.DVBType)
	{
	case ATV:
		return CreateAlarmXMLForATV::CreateFreqAlarmXML(checkparam,alarmvalue,alarmid);
	case RADIO:
		return CreateAlarmXMLForRADIO::CreateFreqAlarmXML(checkparam,alarmvalue,alarmid);
	case AM:
		return CreateAlarmXMLForAM::CreateFreqAlarmXML(checkparam,alarmvalue,alarmid);
	case CTTB:
		return CreateAlarmXMLForCTTB::CreateFreqAlarmXML(checkparam,alarmvalue,alarmid);
	case DVBC:
		return CreateAlarmXMLForDVBC::CreateFreqAlarmXML(checkparam,alarmvalue,alarmid);
	case DVBS:
		return CreateAlarmXMLForDVBS::CreateFreqAlarmXML(checkparam,alarmvalue,alarmid);
	case CTV:
		return CreateAlarmXMLForCTV::CreateFreqAlarmXML(checkparam,alarmvalue,alarmid);
	case THREED:
		return CreateAlarmXMLForTHREED::CreateFreqAlarmXML(checkparam,alarmvalue,alarmid);
	default:
		return "";
	}
	return "";
}
std::string AlarmMgr::CreateProgramAlarmXML(const sCheckParam& checkparam,std::string alarmvalue,std::string alarmid)
{
	switch(checkparam.DVBType)
	{
	case ATV:
		return CreateAlarmXMLForATV::CreateProgramAlarmXML(checkparam,alarmvalue,alarmid);
	case RADIO:
		return CreateAlarmXMLForRADIO::CreateProgramAlarmXML(checkparam,alarmvalue,alarmid);
	case AM:
		return CreateAlarmXMLForAM::CreateProgramAlarmXML(checkparam,alarmvalue,alarmid);
	case CTTB:
		return CreateAlarmXMLForCTTB::CreateProgramAlarmXML(checkparam,alarmvalue,alarmid);
	case DVBC:
		return CreateAlarmXMLForDVBC::CreateProgramAlarmXML(checkparam,alarmvalue,alarmid);
	case DVBS:
		return CreateAlarmXMLForDVBS::CreateProgramAlarmXML(checkparam,alarmvalue,alarmid);
	case CTV:
		return CreateAlarmXMLForCTV::CreateProgramAlarmXML(checkparam,alarmvalue,alarmid);
	case THREED:
		return CreateAlarmXMLForTHREED::CreateProgramAlarmXML(checkparam,alarmvalue,alarmid);
	default:
		return "";
	}
	return "";
}
std::string AlarmMgr::CreateEnvironmentAlarmXML(const sCheckParam& checkparam,std::string alarmvalue)
{
	switch(checkparam.DVBType)
	{
	case ATV:
		return CreateAlarmXMLForATV::CreateEnvironmentAlarmXML(checkparam,alarmvalue);
	case RADIO:
		return CreateAlarmXMLForRADIO::CreateEnvironmentAlarmXML(checkparam,alarmvalue);
	case AM:
		return CreateAlarmXMLForAM::CreateEnvironmentAlarmXML(checkparam,alarmvalue);
	case CTTB:
		return CreateAlarmXMLForCTTB::CreateEnvironmentAlarmXML(checkparam,alarmvalue);
	case DVBC:
		return CreateAlarmXMLForDVBC::CreateEnvironmentAlarmXML(checkparam,alarmvalue);
	case DVBS:
		return CreateAlarmXMLForDVBS::CreateEnvironmentAlarmXML(checkparam,alarmvalue);
	case CTV:
		return CreateAlarmXMLForCTV::CreateEnvironmentAlarmXML(checkparam,alarmvalue);
	case THREED:
		return CreateAlarmXMLForTHREED::CreateEnvironmentAlarmXML(checkparam,alarmvalue);
	default:
		return "";
	}
	return "";
}
bool AlarmMgr::ClearAlarm(string Freq)
{
	if(Freq=="ALL")
	{
		ETRMutex.acquire();
		ETRAlarm.clear();
		ETRMutex.release();
		ProgramMutex.acquire();
		ProgramAlarm.clear();
		ProgramMutex.release();
	}
	else
	{
		string key="_"+Freq+"_";
		ETRMutex.acquire();
		std::map<std::string,std::vector<sCheckParam> >::iterator itorE=ETRAlarm.begin();
		for(;itorE!=ETRAlarm.end();itorE++)
		{
			if(itorE->first.find(key.c_str())!=string::npos)
			{
				ETRAlarm.erase(itorE);
				break;
			}
		}
		ETRMutex.release();
		ProgramMutex.acquire();
		std::map<std::string,std::vector<sCheckParam> >::iterator itorP=ProgramAlarm.begin();
		for(;itorP!=ProgramAlarm.end();itorP++)
		{
			if(itorP->first.find(key.c_str())!=string::npos)
			{
				ProgramAlarm.erase(itorP);
				break;
			}
		}
		ProgramMutex.release();
	}
	return true;
}
/**********************************************/
// 如果报警开关关闭，则放弃报警检查（当前以及通用参数），返回失败
// 如果未设置报警门限，则放弃报警检查（当前以及通用参数），返回失败
// 否则返回成功并将报警参数返回
/**********************************************/
bool AlarmMgr::GetAlarmParm(const sCheckParam& checkparam,sAlarmParam& param)
{
	sAlarmParam tempalarmparam;
	bool ret = false;

	//获取报警开关
	ret = ALARMPARAMINFOMGR::instance()->GetAlarmParam(checkparam,tempalarmparam);			//根据测量的报警值获得相应的报警参数设置
	if (ret==false || tempalarmparam.Switch=="0")											//如果当前频点没有找到参数或者开关参数为关闭，则找通用参数
	{
		sCheckParam temp=checkparam;
		temp.Freq="All";								
		ret=ALARMPARAMINFOMGR::instance()->GetAlarmParam(temp,tempalarmparam);
	}
	if (ret==false || tempalarmparam.Switch=="0")											//当前类型以及通用参数都没有找到或者开关都为关闭，则放弃当前报警检测
		return false;

	//获取报警门限
	switch(checkparam.AlarmType)//根据报警类型检测是否报警，并生成上报的xml
	{
	case ALARM_ENVIRONMENT:
		{
			ret = ALARMPARAMINFOMGR::instance()->GetAlarmParam(checkparam,tempalarmparam);			//根据测量的报警值获得相应的报警参数设置
			if (ret==false || tempalarmparam.DownThreshold=="" || tempalarmparam.UpThreshold=="")	//如果当前频点没有找到参数，则找通用参数
			{
				sCheckParam temp=checkparam;
				temp.Freq="All";															
				ret=ALARMPARAMINFOMGR::instance()->GetAlarmParam(temp,tempalarmparam);
			}
			if (ret==false || tempalarmparam.DownThreshold=="" || tempalarmparam.UpThreshold=="")	//当前类型以及通用参数都没有找到，则放弃当前报警检测
				return false;

			break;
		}
	case ALARM_FREQ:
		{
			ret = ALARMPARAMINFOMGR::instance()->GetAlarmParam(checkparam,tempalarmparam);			//根据测量的报警值获得相应的报警参数设置
			if (ret==false || tempalarmparam.DownThreshold=="" || tempalarmparam.UpThreshold=="")	//如果当前频点没有找到参数，则找通用参数
			{
				sCheckParam temp=checkparam;
				temp.Freq="All";															
				ret=ALARMPARAMINFOMGR::instance()->GetAlarmParam(temp,tempalarmparam);
			}
			if (ret==false || tempalarmparam.DownThreshold=="" || tempalarmparam.UpThreshold=="")	//当前类型以及通用参数都没有找到，则放弃当前报警检测
				return false;

			break;
		}
	case ALARM_PROGRAM:
		{
			ret = ALARMPARAMINFOMGR::instance()->GetAlarmParam(checkparam,tempalarmparam);			//根据测量的报警值获得相应的报警参数设置
			if (ret==false || tempalarmparam.Duration=="" || tempalarmparam.Duration=="0")			//如果当前频点没有找到参数，则找通用参数
			{
				sCheckParam temp=checkparam;
				temp.Freq="All";															
				ret=ALARMPARAMINFOMGR::instance()->GetAlarmParam(temp,tempalarmparam);
			}
			if (ret==false || tempalarmparam.Duration=="" || tempalarmparam.Duration=="0")			//当前类型以及通用参数都没有找到，则放弃当前报警检测
				return false;

			break;
		}
	case ALARM_TR101_290:
		{
			ret = ALARMPARAMINFOMGR::instance()->GetAlarmParam(checkparam,tempalarmparam);							//根据测量的报警值获得相应的报警参数设置
			if (ret==false || tempalarmparam.TimeInterval=="" || tempalarmparam.Num=="" || tempalarmparam.Num=="0")	//如果当前频点没有找到参数，则找通用参数
			{
				sCheckParam temp=checkparam;
				temp.Freq="All";															
				ret=ALARMPARAMINFOMGR::instance()->GetAlarmParam(temp,tempalarmparam);
			}
			if (ret==false || tempalarmparam.TimeInterval=="" || tempalarmparam.Num=="" || tempalarmparam.Num=="0")	//当前类型以及通用参数都没有找到，则放弃当前报警检测
				return false;
			break;
		}
	default:
		break;
	}
	//将报警参数返回
	param=tempalarmparam;
	param.Switch="1";
	return true;
}

string AlarmMgr::GetAlarmPriority(eDVBType dvbtype,string type)
{
	ACE_Guard<ACE_Thread_Mutex> guard(MutexTypeID);
	if(vecAlarmPriority.size()>0)
	{
		std::vector<sAlarmPriority>::iterator itr=vecAlarmPriority.begin();
		for(;itr!=vecAlarmPriority.end();itr++)
		{
			if((*itr).dvbtype==dvbtype&&(*itr).type==type)
				return ((*itr).priority);
		}
	}
	return string("1");
}

string AlarmMgr::GetAlarmTypeID(string headstr)
{
	ACE_Guard<ACE_Thread_Mutex> guard(MutexTypeID);
	if(vecSeperateTypeID.size()>0)
	{
		std::vector<SeperateTypeID>::iterator itr = vecSeperateTypeID.begin();
		for(;itr!=vecSeperateTypeID.end();itr++)
		{
			if((*itr).head==headstr)
			{
				return ((*itr).type);	
			}			
		}
	}
	
	return string("");
}
bool AlarmMgr::GetAlarmTypeIDEx(string headstr,vector<int>& alarmtypelist)
{
	ACE_Guard<ACE_Thread_Mutex> guard(MutexTypeID);
	if(vecSeperateTypeID.size()>0)
	{
		std::vector<SeperateTypeID>::iterator itr = vecSeperateTypeID.begin();
		for(;itr!=vecSeperateTypeID.end();itr++)
		{
			if((*itr).head==headstr)
			{
				alarmtypelist.push_back(StrUtil::Str2Int((*itr).type));	
			}
		}
	}
	return true;
}
void AlarmMgr::SendAlarm(std::string alarmxml)
{
	if(alarmxml.length()>0)
	{
		ACE_Message_Block *MBALARM = new ACE_Message_Block(alarmxml.length());
		memcpy(MBALARM->wr_ptr(),alarmxml.c_str(),alarmxml.length());
		MBALARM->wr_ptr(alarmxml.length());
		pAlarmSender->putq(MBALARM);//将报警xml添加到上报队列中
	}
}
//
void AlarmMgr::AddAlarmRec(string Freq,int iType)
{
//<?xml version="1.0" encoding="GB2312" standalone="yes"?>
//<Msg Version="2" MsgID="23065248" Type="TVMonDown" DateTime="2012-07-30 14:37:12" SrcCode="900000X20" DstCode="340700AB01" Priority="10004" UserName="George" Pa
//ssWord="HowAreYou">
// <TaskSet>
//  <Task EquCode="340700AB00001" TaskID="1000903765" Action="Set" ChCode="AHTV-YS" Freq="" DeviceID="" Width="352" Height="288" Fps="25" Bps="700000">
//   <RecordTime StartDateTime="2012-07-30 14:29:58" EndDateTime="2012-07-30 14:34:58" ExpireDays="2" />
//  </Task>
// </TaskSet>
//</Msg>
	//attention taskid=666666or888888 is used in file recordtask.cpp
	long sttime=(long)time(0);
	long entime=0;
	alarmrecinfo alarmrecin;
	alarmrecin.freq=Freq;
	MutexAlarmRecTime.acquire();
	if(IsAlarmrecExist(Freq))
	{
		return;
	}
	if(sttime<m_lsttime)
	{
		sttime=m_lsttime;
	}
	entime=sttime+StrUtil::Str2Int(m_reclength);
	alarmrecin.entime=entime;
	m_lsttime=entime+1;
	AlarmRec.push_back(alarmrecin);
	MutexAlarmRecTime.release();
	//
	std::string alarmrecxml ="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\" ?> ";
	if(iType==1)
	{
		alarmrecxml+="<Msg Version=\"2\" MsgID=\"23065248\" Type=\"TVMonDown\" DateTime=\"";
		alarmrecxml+=TimeUtil::GetCurDateTime();
		alarmrecxml+="\" SrcCode=\"111111\" DstCode=\"222222\" Priority=\"10004\" UserName=\"George\" PassWord=\"HowAreYou\">";
		alarmrecxml+="<TaskSet>";
		alarmrecxml+="<Task EquCode=\"333333\" TaskID=\"888888\" Action=\"Set\" ChCode=\"\" Freq=\"";
		alarmrecxml+=Freq;
		alarmrecxml+="\" DeviceID=\"";
		alarmrecxml+=m_tvdeciceid;
		alarmrecxml+="\" Width=\"352\" Height=\"288\" Fps=\"25\" Bps=\"700000\">";
		alarmrecxml+="<RecordTime StartDateTime=\"";
		alarmrecxml+=TimeUtil::DateTimeToStr(sttime);
		alarmrecxml+="\" EndDateTime=\"";
		alarmrecxml+=TimeUtil::DateTimeToStr(entime);
		alarmrecxml+="\" ExpireDays=\"";
		alarmrecxml+=m_expiredays;
		alarmrecxml+="\" />";
		alarmrecxml+="</Task>";
		alarmrecxml+="</TaskSet>";
		alarmrecxml+="</Msg>";
	}
	else if(iType==2)
	{
		alarmrecxml+="<Msg Version=\"2\" MsgID=\"23065248\" Type=\"RadioDown\" DateTime=\"";
		alarmrecxml+=TimeUtil::GetCurDateTime();
		alarmrecxml+="\" SrcCode=\"111111\" DstCode=\"222222\" Priority=\"10004\" UserName=\"George\" PassWord=\"HowAreYou\">";
		alarmrecxml+="<TaskSet>";
		alarmrecxml+="<Task EquCode=\"333333\" TaskID=\"666666\" Action=\"Set\" Freq=\"";
		alarmrecxml+=Freq;
		alarmrecxml+="\" ChannelCODE=\"\" DeviceID=\"";
		alarmrecxml+=m_radiodeciceid;
		alarmrecxml+="\" Bps=\"700000\">";
		alarmrecxml+="<RecordTime StartDateTime=\"";
		alarmrecxml+=TimeUtil::DateTimeToStr(sttime);
		alarmrecxml+="\" EndDateTime=\"";
		alarmrecxml+=TimeUtil::DateTimeToStr(entime);
		alarmrecxml+="\" ExpireDays=\"";
		alarmrecxml+=m_expiredays;
		alarmrecxml+="\" />";
		alarmrecxml+="</Task>";
		alarmrecxml+="</TaskSet>";
		alarmrecxml+="</Msg>";
	}
	//
	ACE_Message_Block *mbXML = new ACE_Message_Block(alarmrecxml.length());
	memcpy(mbXML->wr_ptr(),alarmrecxml.c_str(),alarmrecxml.length());
	mbXML->wr_ptr(alarmrecxml.length());

	if (mbXML != NULL)
	{
		ACE_Time_Value OutTime(ACE_OS::time(0)+1);
		COMMUNICATIONMGR::instance()->AccessOrderReceive()->putq(mbXML,&OutTime);
		OSFunction::Sleep(0,500);
	}
}
bool AlarmMgr::IsAlarmrecExist(string Freq)
{
	std::vector<alarmrecinfo>::iterator it = AlarmRec.begin();
	for(;it!=AlarmRec.end();)
	{
		if(time(0)>(*it).entime)
		{
			it=AlarmRec.erase(it);	
		}
		else
		{
			it++;
		}
	}
	//
	std::vector<alarmrecinfo>::iterator itr = AlarmRec.begin();
	for(;itr!=AlarmRec.end();itr++)
	{
		if((*itr).freq==Freq)
		{
			return true;	
		}
	}
	return false;
}

std::string AlarmMgr::CreateEQUIPMENTAlarmXML( const sCheckParam& checkparam,std::string alarmvalue,std::string alarmid )
{
    switch(checkparam.DVBType)
    {
    case ATV:
        return CreateAlarmXMLForATV::CreateEQUIPMENTAlarmXML(checkparam,alarmvalue,alarmid);
    case RADIO:
        return CreateAlarmXMLForRADIO::CreateEQUIPMENTAlarmXML(checkparam,alarmvalue,alarmid);
    default:
        return "";
    }
    return "";
}

std::string AlarmMgr::CreateEQUIPMENTAlarmXML( const sCheckParam& checkparam,std::string &alarmxml )
{
    std::string xml ="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\" ?> ";//返回xml头
    xml+="<Msg><Return/><EquipmentAlarmHistoryReport></EquipmentAlarmHistoryReport></Msg>";
    XmlParser parser(xml.c_str());
    string desc = "";
    pXMLNODE root=parser.GetRootNode();
    parser.SetAttrNode( "Version",string("1"),root );//版本

    string strDateTime = TimeUtil::GetCurDateTime();
    string MsgID = OSFunction::GetXmlMsgID(strDateTime);
    parser.SetAttrNode( "MsgID",MsgID,root );//消息id属性
    parser.SetAttrNode( "DateTime",strDateTime/*TimeUtil::GetCurDateTime()*/,root );//当前时间

    //parser.SetAttrNode( "Type",string("TVMonUp"),root );//消息类型
    parser.SetAttrNode( "SrcCode",PROPMANAGER::instance()->GetDefSrcCode(ATV),root );//本机标识，可通过接口获得
    parser.SetAttrNode( "DstCode",PROPMANAGER::instance()->GetDefDstCode(ATV),root );//目标机器标识
    parser.SetAttrNode( "ReplyID",string("-1"),root );//回复的消息id

    pXMLNODE RetNode=parser.GetNodeFromPath("Msg/Return");
    parser.SetAttrNode("Type",string("EquipmentAlarmHistoryReport"),RetNode);
    parser.SetAttrNode("Value",string("0"),RetNode);
    parser.SetAttrNode("Desc",string("成功"),RetNode);
    pXMLNODE node =parser.GetNodeFromPath("Msg/EquipmentAlarmHistoryReport");

    pXMLNODE EquipmentAlarmnode=parser.CreateNodePtr(node,"EquipmentAlarm");
    parser.SetAttrNode("EquCode",PROPMANAGER::instance()->GetDefSrcCode(ATV),EquipmentAlarmnode);
    parser.SetAttrNode("AlarmID",checkparam.AlarmID,EquipmentAlarmnode);
    parser.SetAttrNode("Mode",checkparam.mode,EquipmentAlarmnode);
    if(checkparam.TypeID=="1")
    {
        desc = "CPU使用率";
    }
    else if(checkparam.TypeID=="2")
    {
        desc = "内存使用率";
    }
	else if(checkparam.TypeID=="3")
    {
        desc = "磁盘空间";
    }
    string TypeDesc="";
    if(checkparam.mode=="1")
    {
        TypeDesc = checkparam.TypeDesc + std::string("--解除");
    }
    else
        TypeDesc = checkparam.TypeDesc;
    parser.SetAttrNode("Desc",desc,EquipmentAlarmnode);
    parser.SetAttrNode("Type",checkparam.TypeID,EquipmentAlarmnode);
    parser.SetAttrNode("Reason",TypeDesc,EquipmentAlarmnode);
    parser.SetAttrNode("CheckDateTime",TimeUtil::DateTimeToStr(checkparam.CheckTime),EquipmentAlarmnode);
    parser.SaveToString(alarmxml);
    return alarmxml;
}
