
#pragma warning(disable:4996)

#include "SpectrumTask.h"
#include "TranslateDownXML.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/PropManager.h"
#include <vector>
#include <iostream>
#include "ace/Task.h"
#include "../Foundation/TypeDef.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
#include "../DeviceAccess/DeviceAccessMgr.h"
#include "Scheduler.h"
#include "../Foundation/XmlParser.h"
#include "../DBAccess/DBManager.h"
#include "./TranslateDownXML.h"
#include "./TranslateXMLForDevice.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Alarm/AlarmMgr.h"
using namespace std;


SpectrumTask::SpectrumTask() : DeviceRelatedTask()
{

}

SpectrumTask::SpectrumTask(std::string strXML) : DeviceRelatedTask(strXML)
{
	TaskScheduler =new Scheduler();

	CycleTask=false;
	int mCheckInterVal = 0;
	int cycleInterval = 0;
	//设置任务的运行时间
	XmlParser parser;
	parser.Set_xml(strXML);
	pXMLNODE qualityTaskNode = parser.GetNodeFromPath("Msg/SpectrumTask");
	std::string taskid;
	parser.GetAttrNode(qualityTaskNode,"TaskID",taskid);
	SetTaskID(taskid);

	pXMLNODE specQueryIndexNode = parser.GetNodeFromPath("Msg/SpectrumTask/TaskParam/SpectrumQueryIndex");
	parser.GetAttrNode(specQueryIndexNode, "StartFreq", m_AMStartFreq);
	parser.GetAttrNode(specQueryIndexNode, "EndFreq", m_AMEndFreq);

	pXMLNODE node=parser.GetNodeFromPath("Msg/SpectrumTask/TimeParam/TimeIndex");
	std::string DayOfWeek, StartTime,StartDateTime,EndDateTime;
	parser.GetAttrNode(node,"DayOfWeek",DayOfWeek);
	parser.GetAttrNode(node,"StartTime",StartTime);
	parser.GetAttrNode(node,"StartDateTime",StartDateTime);
	parser.GetAttrNode(node,"EndDateTime",EndDateTime);
	if(!(DayOfWeek == ""))
	{
		CycleTask=true;
		if (stricmp(DayOfWeek.c_str(),"ALL"))  //每周的某一天
		{
			std::string dateTime,startDate;
			long today=TimeUtil::DateIsWeekDay(time(0));//今天星期几
			int diffDay=StrUtil::Str2Int(DayOfWeek)-today;//星期之间所差的天数
			if (diffDay<0)
			{
				diffDay+=7;
			}
			dateTime=TimeUtil::CalDay(time(0),diffDay);
			startDate=TimeUtil::GetDateFromDatetime(dateTime);
			std::string newStartDateTime=startDate+" "+StartTime;

			while(TimeUtil::DiffSecond(newStartDateTime,TimeUtil::GetCurDateTime()) < 0)
			{
				newStartDateTime = TimeUtil::CalDay(newStartDateTime,1);
			}

			long cycleSec=ONEDAYLONG*7;
			TaskScheduler->SetRunTime(newStartDateTime,EndDateTime,EndDateTime,cycleSec);//设置第一次运行时间
		}
		else //每天都做
		{
			std::string curDate = TimeUtil::GetCurDate();
			std::string newStartDateTime=curDate+" "+StartTime;

			while(TimeUtil::DiffSecond(newStartDateTime,TimeUtil::GetCurDateTime()) < 0)
			{
				newStartDateTime = TimeUtil::CalDay(newStartDateTime,1);
			}

			long cycleSec=ONEDAYLONG;
			TaskScheduler->SetRunTime(newStartDateTime,EndDateTime,EndDateTime,cycleSec);//设置第一次运行时间
		}
	}
	else
	{
		CycleTask=false;
		TaskScheduler->SetRunTime(StartDateTime);
	}
}

SpectrumTask::~SpectrumTask()
{

}

void SpectrumTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]频谱任务执行 !\n",DeviceID));

	bRun = true;
	SetRunning();
	RetValue = RUN_SUCCESS;
	std::list<int> tasklist;
	int scandeviceid=0;
	string strRtnXml,chanelscanXml;

	PROPMANAGER::instance()->GetTaskDeviceList("ATVChannelScanQueryTask",DVBType,tasklist);
	if(!tasklist.empty())
		scandeviceid=tasklist.front();
	if(DVBType==RADIO)
	{
		chanelscanXml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg Version=\"1\" MsgID=\"23011119\" DVBType=\"RADIO\" TaskType=\"ChannelScanQuery\">\
					  <ChannelScanQuery><Channel StartFreq=\"87\" EndFreq=\"108\" StepFreq=\"0.1\" /></ChannelScanQuery></Msg>";
	}
	else if(DVBType==ATV)
	{
		chanelscanXml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg Version=\"1\" MsgID=\"23011120\" DVBType=\"ATV\" TaskType=\"ChannelScanQuery\">\
					  <ChannelScanQuery><Channel StartFreq=\"49.25\" EndFreq=\"855.25\" StepFreq=\"8\" /></ChannelScanQuery></Msg>";	
	}
	else if(DVBType==AM)
	{
		chanelscanXml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg Version=\"1\" MsgID=\"23011121\" DVBType=\"AM\" TaskType=\"ChannelScanQuery\">\
					  <ChannelScanQuery><Channel StartFreq=\"531\" EndFreq=\"700\" StepFreq=\"1\" /></ChannelScanQuery></Msg>";
		/* AM如果做全频点的频道扫描，会花费较长时间（4分钟左右）,所以只做任务范围内的频道扫描 */
		if (m_AMStartFreq!=0 && m_AMEndFreq!=0)
		{
			XmlParser chanScanParser(chanelscanXml.c_str());
			pXMLNODE chanNode = chanScanParser.GetNodeFromPath("Msg/ChannelScanQuery/Channel");
			chanScanParser.SetAttrNode("StartFreq", m_AMStartFreq, chanNode);
			chanScanParser.SetAttrNode("EndFreq", m_AMEndFreq, chanNode);
			chanScanParser.SaveToString( chanelscanXml );
		}
	}
	else if(DVBType==CTV)
	{
		chanelscanXml="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\
					  <Msg Version=\"1\" MsgID=\"23011120\" DVBType=\"CTV\" TaskType=\"ChannelScanQuery\">\
					  <ChannelScanQuery><Channel StartFreq=\"49.25\" EndFreq=\"855.25\" StepFreq=\"8\" /></ChannelScanQuery></Msg>";	
	}
	
	bool rtn=DEVICEACCESSMGR::instance()->SendTaskMsg(scandeviceid,chanelscanXml,strRtnXml);
	std::vector<string> vecChannel;//频道扫描出的频点
	if(rtn)
	{
		XmlParser xparser;
		xparser.Set_xml(strRtnXml);
		pXMLNODE ChannelNode=xparser.GetNodeFromPath("Msg/ChannelScan");
		pXMLNODELIST ChannelList=xparser.GetNodeList(ChannelNode);
		for(int k=0;k<xparser.GetChildCount(ChannelNode);k++)
		{
			string tempfreq;
			pXMLNODE ChildNode=xparser.GetNextNode(ChannelList);
			xparser.GetAttrNode(ChildNode,"Freq",tempfreq);
			vecChannel.push_back(tempfreq);
		}
	}

	XmlParser parser;
	std::string TaskID;
	parser.Set_xml(strStandardXML);
	pXMLNODE qualityNode = parser.GetNodeFromPath("Msg/SpectrumTask");
	parser.GetAttrNode(qualityNode,"TaskID",TaskID);
	pXMLNODE ParamNode = parser.GetNodeFromPath("Msg/SpectrumTask/TaskParam/SpectrumQueryIndex");
	parser.GetAttrNode(ParamNode,"ReferDbu",ReferLevel);
	strDeviceXML=TranslateXMLForDevice::TranslateSpectrumTask(strStandardXML);

	std::vector<std::string> vecChanInfo;//频道表信息
	CHANNELINFOMGR::instance()->GetFreqInfoByDvbtype(DVBType,vecChanInfo);
	for(int i=0;i<1;i++)
	{
		std::string retDeviceXml;
		std::string	Freq;
		

		//发送指标查询命令
		if(DEVICEACCESSMGR::instance()->SendTaskMsg(DeviceID,strDeviceXML,retDeviceXml) == false || retDeviceXml.empty())
			continue;

		std::string value;
		XmlParser parser(retDeviceXml.c_str());
		pXMLNODE returtnNode = parser.GetNodeFromPath("Msg/Return");
		parser.GetAttrNode(returtnNode,"Value",value);

		if (value != "0")
			continue;

		//获得指标查询结果
		GetSpecInfo(retDeviceXml,TaskID,vecChannel);
		CheckNewFreqAlarm(retDeviceXml,vecChanInfo,vecChannel);
	}
	
	if (TaskScheduler != NULL)
	{	
		if(CycleTask)
		{
			TaskScheduler->SetNextStartDateTime();
		}
		else
		{
			TaskScheduler->ModifyExpiredTime(TimeUtil::DateTimeToStr(time(0)-1));
		}
	}

	SetFinised();

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]频谱任务停止 !\n",DeviceID));
}
string SpectrumTask::GetTaskName()
{
	return std::string("频谱任务");
}
std::string SpectrumTask::GetObjectName()
{
	return std::string("SpectrumTask");
}
bool  SpectrumTask::GetSpecInfo(std::string rtnXML,std::string &taskID,std::vector<string> vecChannel)
{	
	XmlParser rtnParser(rtnXML.c_str());

	pXMLNODE paramNode = rtnParser.GetNodeFromPath("Msg/SpectrumQueryReport/SpectrumParam");
	pXMLNODELIST indexList = rtnParser.GetNodeList( paramNode );
	int count = rtnParser.GetChildCount( paramNode );
	std::string Type ="",Desc = "", Value = "";
	for ( int i = 0;i<count;i++ )
	{
		string Freq;
		std::string strTime = TimeUtil::GetCurDateTime();
		pXMLNODE indexNode =rtnParser.GetNextNode( indexList );
		//获取eQualityInfo结构体的值
		string tempFreq;
		rtnParser.GetAttrNode( indexNode,"Freq",tempFreq );
		rtnParser.GetAttrNode( indexNode,"Value",Value );
		float fFreq=StrUtil::Str2Float(tempFreq);
		fFreq=fFreq/1000;
		if (GetDVBType() == ATV || GetDVBType() == CTV)
		{
			Freq= StrUtil::RoundFloat(fFreq,2);
		}
		else if (GetDVBType()==RADIO || GetDVBType()==AM)
		{
			Freq= StrUtil::Float2Str1(fFreq);
		}
		std::vector<string>::iterator iter;
		iter=find(vecChannel.begin(),vecChannel.end(),Freq);
		
		eSpecInfo  mqInfo;
		mqInfo.type = "100";
		mqInfo.desc = "频谱";
		mqInfo.valu = Value;
		if(iter!=vecChannel.end())
		{
			mqInfo.status="1";
		}
		else
		{
			mqInfo.status="0";
		}
		DBMANAGER::instance()->AddSpectrumInfo(DVBType,Freq,TaskID,mqInfo,strTime);
	}

	return true;
}

bool SpectrumTask::CheckNewFreqAlarm(std::string rtnXML,std::vector<std::string> vecLocalChannel,std::vector<std::string> vecNewChannel)
{
	XmlParser rtnParser(rtnXML.c_str());

	string LastFreq="";
	pXMLNODE paramNode = rtnParser.GetNodeFromPath("Msg/SpectrumQueryReport/SpectrumParam");
	pXMLNODELIST indexList = rtnParser.GetNodeList( paramNode );
	int count = rtnParser.GetChildCount( paramNode );
	std::string Type ="",Desc = "", Value = "";
	for ( int i = 0;i<count;i++ )
	{
		string Freq;
		std::string strTime = TimeUtil::GetCurDateTime();
		pXMLNODE indexNode =rtnParser.GetNextNode( indexList );
		//获取eQualityInfo结构体的值
		string tempFreq;
		rtnParser.GetAttrNode( indexNode,"Freq",tempFreq );
		rtnParser.GetAttrNode( indexNode,"Value",Value );
		float fFreq=StrUtil::Str2Float(tempFreq);
		fFreq=fFreq/1000;
		if (GetDVBType() == ATV || GetDVBType() == CTV)
		{
			Freq= StrUtil::RoundFloat(fFreq,2);
		}
		else if (GetDVBType()==RADIO || GetDVBType()==AM)
		{
			Freq= StrUtil::RoundFloat(fFreq,2);
			//Freq= StrUtil::Float2Str1(fFreq);
		}
		if(LastFreq==Freq)
			continue;
		if(StrUtil::Str2Float(Value)>ReferLevel)
		{
			std::vector<string>::iterator iter,iter1;
			iter  = find(vecLocalChannel.begin(),vecLocalChannel.end(),Freq);
			iter1 = find(vecNewChannel.begin(),vecNewChannel.end(),Freq);
			if(iter==vecLocalChannel.end()&&iter1!=vecNewChannel.end())
			{
				LastFreq = Freq;
				ALARMMGR::instance()->SendAlarm(CreateNewFreqAlarmXml(Freq,Value));
				//
				if (GetDVBType() == ATV || GetDVBType() == CTV)
				{
					ALARMMGR::instance()->AddAlarmRec(Freq,1);
				}
				else if (GetDVBType()==RADIO || GetDVBType()==AM)
				{
					ALARMMGR::instance()->AddAlarmRec(Freq,2);
				}
			}
		}
	}

	return true;
}

void SpectrumTask::SetDeciveID( int deviceid )
{
	DeviceID=deviceid;
}

bool SpectrumTask::AddTaskXml()
{
	return DBMANAGER::instance()->AddXmlTask(DVBType,strStandardXML);
}

bool SpectrumTask::DelTaskXml()
{
	return DBMANAGER::instance()->DeleteTask(DVBType,strStandardXML);
}
std::string SpectrumTask::CreateNewFreqAlarmXml(std::string freq,std::string level)
{
	std::string amxml ="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\" ?> ";//返回xml头
	amxml+="<Msg><NewFreqAlarmReport></NewFreqAlarmReport></Msg>";
	XmlParser amparser(amxml.c_str());

	string Typeid="";
	std::string Uptype;
	if(DVBType==RADIO)
	{
		Uptype="RadioUp";
		Typeid="51";
	}
	else if(DVBType == ATV)
	{
		Uptype="TVMonUp";
		Typeid="50";
	}
	long alarmid=0;
	pXMLNODE amroot=amparser.GetRootNode();
	amparser.SetAttrNode( "Version",string("2"),amroot );//版本

	string strDateTime = TimeUtil::GetCurDateTime();
	string  MsgID = OSFunction::GetXmlMsgID(strDateTime);
	amparser.SetAttrNode( "MsgID",MsgID,amroot );//消息id属性
	amparser.SetAttrNode( "DateTime",strDateTime/*TimeUtil::GetCurDateTime()*/,amroot );//当前时间
	amparser.SetAttrNode( "Type",Uptype,amroot );//消息类型
	amparser.SetAttrNode( "SrcCode",PROPMANAGER::instance()->GetDefSrcCode(DVBType),amroot );//本机标识，可通过接口获得
	amparser.SetAttrNode( "DstCode",PROPMANAGER::instance()->GetDefDstCode(DVBType),amroot );//目标机器标识
	amparser.SetAttrNode( "ReplyID",string("-1"),amroot );//回复的消息id

	string code=PROPMANAGER::instance()->GetDefSrcCode(DVBType);
	pXMLNODE amreportnode=amparser.GetNodeFromPath("Msg/NewFreqAlarmReport");
	pXMLNODE amalarmnode=amparser.CreateNodePtr(amreportnode,"NewFreqAlarm");
	amparser.SetAttrNode("EquCode",string(""),amreportnode);
	PROPMANAGER::instance()->GetAlarmID(alarmid);
	amparser.SetAttrNode("AlarmID",alarmid,amalarmnode);	
	amparser.SetAttrNode("Mode",string("0"),amalarmnode);
	amparser.SetAttrNode("Type",Typeid,amalarmnode);
	amparser.SetAttrNode("Desc",string("新频报警"),amalarmnode);
	amparser.SetAttrNode("Reason",string("发现新频"),amalarmnode);
	amparser.SetAttrNode("CheckTime",TimeUtil::DateTimeToStr(time(0)),amalarmnode);
	amparser.SetAttrNode("Freq",freq,amalarmnode);
	amparser.SetAttrNode("Value",level,amalarmnode);

	std::string retXML;
	amparser.SaveToString(retXML);
	return retXML;
}