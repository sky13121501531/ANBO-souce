///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����RecordTask.cpp
// �����ߣ�jiangcheng
// ����ʱ�䣺2009-06-01
// ����������¼��������
///////////////////////////////////////////////////////////////////////////////////////////
#include "RecordTask.h"
#include "TranslateDownXML.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/PropManager.h"
#include <vector>
#include <iostream>
#include "ace/Task.h"
#include "ace/OS.h"
#include "../Foundation/TypeDef.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
#include "../FileSysAccess/TSRecorder.h"
#include "Scheduler.h"
#include "../Foundation/XmlParser.h"
#include "../DeviceAccess/TsFetcherMgr.h"
#include "../DeviceAccess/DeviceAccessMgr.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Foundation/AppLog.h"
#include "../Task/TranslateXMLForDevice.h"
#include "../DBAccess/DBManager.h"
#include "../Alarm/CheckLevelForUnLock.h"
using namespace std;

class Record_Task : public ACE_Task<ACE_MT_SYNCH>
{
public:
	Record_Task();
	~Record_Task(){};

};

Record_Task::Record_Task()
{
	msg_queue()->high_water_mark(376*8192);
	msg_queue()->low_water_mark(376*8192);
}

RecordTask::RecordTask() : DeviceRelatedTask()
{
	
}

RecordTask::RecordTask(std::string strXML) : DeviceRelatedTask(strXML)
{
	XmlParser parser;
	parser.Set_xml(strStandardXML);
	
	pXMLNODE recordNode;
	if(strStandardXML.find("AutoRecord")!=string::npos)
	{
		recordNode=parser.GetNodeFromPath("Msg/AutoRecord/Record");
	}
	else if(strStandardXML.find("TaskRecord")!=string::npos)
	{
		recordNode=parser.GetNodeFromPath("Msg/TaskRecord/Record");
	}
	string OrgNetID, TsID, ServiceID,VideoPID,AudioPID,Code;
	string dayofweek,starttime,endtime,startdatetime,enddatetime;
	if(recordNode)
	{
		parser.GetAttrNode(recordNode,"TaskID",TaskID);
		parser.GetAttrNode(recordNode,"DeviceID",DeviceID);
		parser.GetAttrNode(recordNode,"Freq",Freq);
		parser.GetAttrNode(recordNode,"OrgNetID",OrgNetID);
		parser.GetAttrNode(recordNode,"TsID",TsID);
		parser.GetAttrNode(recordNode,"ServiceID",ServiceID);
		parser.GetAttrNode(recordNode,"VideoPID",VideoPID);
		parser.GetAttrNode(recordNode,"AudioPID",AudioPID);
		parser.GetAttrNode(recordNode,"Code",Code);
		parser.GetAttrNode(recordNode,"DayofWeek",dayofweek);
		parser.GetAttrNode(recordNode,"DayofWeek",dayofweek);
		parser.GetAttrNode(recordNode,"StartTime",starttime);
		parser.GetAttrNode(recordNode,"EndTime",endtime);
		parser.GetAttrNode(recordNode,"StartDateTime",startdatetime);
		parser.GetAttrNode(recordNode,"EndDateTime",enddatetime);
	}
	CHANNELINFOMGR::instance()->GetChannelID(DVBType,OrgNetID,TsID,ServiceID,VideoPID,AudioPID,Code,ChannelID);

	if(ChannelID=="")
		ChannelID=Freq;
	//if(ChannelID=="")
	//	ChannelID=Code;	

	TaskScheduler=new Scheduler();//�����ִ��ʱ�����
	//��������ִ������ʱ��
	if (dayofweek.length()==0)
	{
		if (TaskID=="0")//��¼��
		{
			TaskScheduler->SetRunTime(TimeUtil::GetCurDateTime());
		}
		else//�������е�����¼��
		{
			TaskScheduler->SetRunTime(startdatetime,enddatetime,enddatetime);
		}
	}
	else
	{
		long diffSec=TimeUtil::StrToSecondTime(endtime)-TimeUtil::StrToSecondTime(starttime);
		//�������
		
		if(dayofweek=="ALL"||dayofweek=="All")//ÿ������
		{
			std::string StartDateTime=TimeUtil::GetCurDate()+" "+starttime;
			std::string EndDateTime=TimeUtil::GetCurDate()+" "+endtime;
			long cycleSec=ONEDAYLONG-diffSec;
			TaskScheduler->SetRunTime(StartDateTime,EndDateTime,TimeUtil::GetEndLessTime(),cycleSec);//���õ�һ������ʱ��
		}
		else//һ�ܵ�ĳ������
		{
			std::string dateTime,startDate;
			long today=TimeUtil::DateIsWeekDay(time(0));//�������ڼ�
			int diffDay=StrUtil::Str2Int(dayofweek)-today;//����֮�����������
			if (diffDay<0)
			{
				diffDay+=7;
			}
			dateTime=TimeUtil::CalDay(time(0),diffDay);
			startDate=TimeUtil::GetDateFromDatetime(dateTime);
			std::string StartDateTime=startDate+" "+starttime;
			std::string EndDateTime=startDate+" "+endtime;
			long cycleSec=ONEDAYLONG*7-diffSec;
			TaskScheduler->SetRunTime(StartDateTime,EndDateTime,TimeUtil::GetEndLessTime(),cycleSec);//���õ�һ������ʱ��
		}
		
	}
	//
	//if(((TaskID!="888888")&&(DeviceID==StrUtil::Str2Int(PROPMANAGER::instance()->GetAlarmrecTvDeciceid())))
	//	||((TaskID!="666666")&&(DeviceID==StrUtil::Str2Int(PROPMANAGER::instance()->GetAlarmrecRadioDeciceid()))))//����¼��������ռ�ñ���¼��ͨ��
	//{
	//	DeviceID = 0;
	//}
	//
	if (DeviceID == 0)		//��ʼ��ͨ��
	{
		DeviceID = -1;// ����Ϊ-1�����Ȼ����һ��ͨ��
	}
}
RecordTask::~RecordTask()
{
}
void RecordTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]¼������ִ�� !\n",DeviceID));
	bRun = true;
	if(DeviceID == -1)//δ���䵽ͨ�����������
		SetExpired();
	SetRunning();

	strDeviceXML = TranslateXMLForDevice::TranslateRecordTask(strStandardXML );//�Զ����׼XML��Ӳ���ӿ�XMLת��
	
	
	TSFETCHERMGR::instance()->SetTsDeviceXml(DeviceID,strDeviceXML);
	// UDP�����ӳ�1600ms
	// ��̨��ȴ�1600ms���ٽ�������
	// OSFunction::Sleep(2);
	TSFETCHERMGR::instance()->IncreaseTaskNum(DeviceID);
	//if(DeviceID == 14)
	// OSFunction::Sleep(1,500);
	ACE_Task<ACE_MT_SYNCH>* Task = new Record_Task;
	TSFETCHERMGR::instance()->SetRecordTask(DeviceID,Task);
	//����ģ�����ز�����ģ���¼����Ϣ
	sSignalCheck param;
	if(DVBType==ATV||DVBType==RADIO||DVBType==AM||DVBType==CTV)
	{	
		param.DeviceID	= StrUtil::Int2Str(DeviceID);
		param.dvbtype	= DVBType;
		param.Freq		= Freq;
		param.ChannelID	= ChannelID;
		CHECKLEVELFORUNLOCK::instance()->AddRecordInfo(param);
	}

	TSRecorder record(DeviceID,strStandardXML);

	int BufLen = 188*70;

	//if (DVBType == RADIO  || DVBType==AM)
	//{
	//	BufLen   /= 16;
	//}

	unsigned char* TsBuf = new unsigned char[BufLen];//������
	memset(TsBuf,0,BufLen);//��ʼ��������
	
	size_t PacketLen = 0;

	while(bRun == true && IsRunning())
	{
		try
		{
			ACE_Message_Block *mb = NULL;

			ACE_Time_Value OutTime(ACE_OS::gettimeofday()+ACE_Time_Value(2));
			if (Task->getq(mb,&OutTime) != -1 && mb != NULL)//�����ݽ����߳�ȡ����
			{
				if (mb->length() > 0)
				{
					if (PacketLen>=BufLen-mb->length())
					{
						record.SaveFile(TsBuf,PacketLen);//����д���ļ�
						memset(TsBuf,0,BufLen);
						PacketLen = 0;
					}
				
					memcpy(TsBuf+PacketLen,(unsigned char*)mb->rd_ptr(),mb->length());//�������ڴ�
					PacketLen += mb->length();
				}
				
				mb->release();
			}
		}
		catch(...)
		{
			string error = "¼�������쳣";
			APPLOG::instance()->WriteLog( OTHER,LOG_EVENT_ERROR,error,LOG_OUTPUT_BOTH );			
			OSFunction::Sleep(0,100);
		}
	}

	delete[] TsBuf;//�ͷ��ڴ�
	TSFETCHERMGR::instance()->DelRecordTask(DeviceID,Task);

	if (TaskScheduler != NULL)
		TaskScheduler->SetNextRunTime();//����������´�ִ��ʱ��
	
	TSFETCHERMGR::instance()->DecreaseTaskNum(DeviceID);
	if(DVBType==ATV||DVBType==RADIO||DVBType==AM)
	{
		CHECKLEVELFORUNLOCK::instance()->RemoveRecordInfo(param);
	}

	SetFinised();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]¼������ֹͣ !\n",DeviceID));
}

std::string RecordTask::GetObjectName()
{
	if (TaskID == "0")
		return std::string("AutoRecord");
	else
		return std::string("TaskRecord");
}
string RecordTask::GetTaskName()
{
	if (TaskID == "0")
		return std::string("�Զ�¼��");
	else
		return std::string("����¼��");

}

void RecordTask::SetDeciveID( int deviceid )
{
	DeviceID=deviceid;
}

bool RecordTask::AddTaskXml( void )
{
	XmlParser psr;
	psr.Set_xml(strStandardXML);
	pXMLNODE node=NULL;
	if(TaskID == "0")
		node=psr.GetNodeFromPath("Msg/AutoRecord/Record");
	else
		node=psr.GetNodeFromPath("Msg/TaskRecord/Record");
	if (node)
	{
		psr.SetAttrNode("DeviceID",DeviceID,node); //���������deviceid
		psr.Get_xml(strStandardXML);
	}
	return DBMANAGER::instance()->AddXmlTask(DVBType,strStandardXML);
}

bool RecordTask::DelTaskXml( void )
{
	return DBMANAGER::instance()->DeleteTask(DVBType,strStandardXML);
}

std::string RecordTask::CreateSchedulerAlarm()
{
	std::string amxml ="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\" ?> ";//����xmlͷ
	amxml+="<Msg><RecordTaskAlarmReport></RecordTaskAlarmReport></Msg>";
	XmlParser amparser(amxml.c_str());

	std::string Uptype;
	if(DVBType==RADIO)
	{
		Uptype="RadioUp";
	}
	else if(DVBType == ATV)
	{
		Uptype="TVMonUp";
	}
	long alarmid=0;
	pXMLNODE amroot=amparser.GetRootNode();
	amparser.SetAttrNode( "Version",string("2"),amroot );//�汾

	string strDateTime = TimeUtil::GetCurDateTime();
	string  MsgID = OSFunction::GetXmlMsgID(strDateTime); 
	amparser.SetAttrNode( "MsgID",MsgID,amroot );//��Ϣid����
	amparser.SetAttrNode( "DateTime",strDateTime/*TimeUtil::GetCurDateTime()*/,amroot );//��ǰʱ��
	amparser.SetAttrNode( "Type",Uptype,amroot );//��Ϣ����
	amparser.SetAttrNode( "SrcCode",PROPMANAGER::instance()->GetDefSrcCode(DVBType),amroot );//������ʶ����ͨ���ӿڻ��
	amparser.SetAttrNode( "DstCode",PROPMANAGER::instance()->GetDefDstCode(DVBType),amroot );//Ŀ�������ʶ
	amparser.SetAttrNode( "ReplyID",string("-1"),amroot );//�ظ�����Ϣid

	string code=PROPMANAGER::instance()->GetDefSrcCode(DVBType);
	pXMLNODE amreportnode=amparser.GetNodeFromPath("Msg/RecordTaskAlarmReport");
	pXMLNODE amalarmnode=amparser.CreateNodePtr(amreportnode,"RecordTaskAlarm ");
	amparser.SetAttrNode("EquCode",string(""),amalarmnode);
	PROPMANAGER::instance()->GetAlarmID(alarmid);
	amparser.SetAttrNode("AlarmID",alarmid,amalarmnode);	
	amparser.SetAttrNode("ChCode",GetChannelID(),amalarmnode);
	amparser.SetAttrNode("TaskID",GetTaskID(),amalarmnode);
	amparser.SetAttrNode("Desc",string("¼����Դ��ͻ����"),amalarmnode);
	amparser.SetAttrNode("Reason",string(""),amalarmnode);
	amparser.SetAttrNode("CheckDateTime",TimeUtil::DateTimeToStr(time(0)),amalarmnode);
	amparser.SetAttrNode("StartDateTime",TimeUtil::DateTimeToStr(GetStartDateTime()),amalarmnode);
	amparser.SetAttrNode("EndDateTime",TimeUtil::DateTimeToStr(GetEndDateTime()),amalarmnode);

	string TypeID="";
	if(TaskID=="0")
		TypeID = strMsgID;
	else
		TypeID = TaskID;

	std::string retXML;
	amparser.SaveToString(retXML);
	sCheckParam tmpParam;
	tmpParam.DVBType=DVBType;
	tmpParam.AlarmType = ALARM_RECORDSCHEDULER;
	tmpParam.Freq = Freq;
	tmpParam.DeviceID = StrUtil::Int2Str(DeviceID);
	tmpParam.ChannelID = ChannelID;
	tmpParam.TypeID = TypeID;
	tmpParam.TypeDesc =" ¼���ͻ����";
	tmpParam.CheckTime = time(0);
	tmpParam.AlarmID = StrUtil::Long2Str(alarmid);
	DBMANAGER::instance()->AddAlarmInfo(tmpParam,"1");
	return retXML;
}
