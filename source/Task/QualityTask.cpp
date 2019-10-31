
#pragma warning(disable:4996)

#include "QualityTask.h"
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
#include "../DeviceAccess/FetchQualityLevel.h"
#include "../Alarm/AlarmMgr.h"
using namespace std;


QualityTask::QualityTask() : DeviceRelatedTask()
{

}

QualityTask::QualityTask(std::string strXML) : DeviceRelatedTask(strXML)
{
	TaskScheduler =new Scheduler();

	int mCheckInterVal = 0;
	int cycleInterval = 0;
	//设置任务的运行时间
	XmlParser parser;
	parser.Set_xml(strXML);
	pXMLNODE qualityTaskNode = parser.GetNodeFromPath("Msg/QualityTask");
	std::string taskid;
	parser.GetAttrNode(qualityTaskNode,"TaskID",taskid);
	SetTaskID(taskid);

	pXMLNODE node=parser.GetNodeFromPath("Msg/QualityTask/TimeParam/TimeIndex");
	std::string DayOfWeek, StartTime, EndTime,StartDateTime, EndDateTime, CheckInterVal;

	parser.GetAttrNode(node,"DayOfWeek",DayOfWeek);
	parser.GetAttrNode(node,"StartTime",StartTime);
	parser.GetAttrNode(node,"EndTime",EndTime);
	parser.GetAttrNode(node,"StartDateTime",StartDateTime);
	parser.GetAttrNode(node,"EndDateTime",EndDateTime);
	parser.GetAttrNode(node,"CheckInterval",CheckInterVal);
	mCheckInterVal=TimeUtil::StrToSecondTime(CheckInterVal);
	if(!(DayOfWeek == ""))
	{
		int differ = TimeUtil::StrToSecondTime(EndTime) - TimeUtil::StrToSecondTime(StartTime);
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
			std::string curStartDateTime=startDate+" "+StartTime;
			std::string curEndDateTime=startDate+" "+EndTime;
			cycleInterval = 7*ONEDAYLONG-differ;
			TaskScheduler->SetRunTime(curStartDateTime,curEndDateTime,EndDateTime,cycleInterval,mCheckInterVal);
		}
		else //每天都做
		{
			cycleInterval = ONEDAYLONG - differ;
			std::string curDate = TimeUtil::GetCurDate();
			std::string curStartDateTime=curDate+" "+StartTime;
			std::string curEndDateTime=curDate+" "+EndTime;
			TaskScheduler->SetRunTime(curStartDateTime,curEndDateTime,EndDateTime,cycleInterval,mCheckInterVal);
		}
	}
	else if (StartDateTime != "" && EndDateTime != "")
	{
		TaskScheduler->SetRunTime(StartDateTime,EndDateTime,EndDateTime,0,mCheckInterVal);
	}
}

QualityTask::~QualityTask()
{

}

void QualityTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]指标任务执行 !\n",DeviceID));

	bRun = true;
	SetRunning();
	RetValue = RUN_SUCCESS;
	XmlParser parser;
	std::string TaskID;
	parser.Set_xml(strStandardXML);
	pXMLNODE qualityNode = parser.GetNodeFromPath("Msg/QualityTask");
	parser.GetAttrNode(qualityNode,"TaskID",TaskID);

	mVecStdStr=TranslateXMLForDevice::TranslateQualityTask(strStandardXML);

	for (size_t i=0;i!=mVecStdStr.size();++i)
	{
	
		std::string retDeviceXml;
		std::string	Freq;
		std::string strTime = TimeUtil::GetCurDateTime();
		std::vector<eQualityInfo> vecqInfo;

		//发送指标查询命令
		if(DEVICEACCESSMGR::instance()->SendTaskMsg(DeviceID,mVecStdStr[i],retDeviceXml) == false || retDeviceXml.empty())
			continue;

		std::string value;
		XmlParser parser(retDeviceXml.c_str());
		pXMLNODE returtnNode = parser.GetNodeFromPath("Msg/Return");
		parser.GetAttrNode(returtnNode,"Value",value);

		if (value != "0")
			continue;
		if(DVBType==DVBC||DVBType==CTTB||DVBType==DVBS)//获取数字电视场强卡场强值
		{
			string freq;
			XmlParser rtnparser(retDeviceXml.c_str());
			pXMLNODE returtnNode = rtnparser.GetNodeFromPath("Msg/Return");
			pXMLNODE ReportNode = rtnparser.GetNodeFromPath("Msg/QualityQueryReport");
			rtnparser.GetAttrNode(ReportNode,"Freq",freq);
			pXMLNODE paramNode = rtnparser.GetNodeFromPath("Msg/QualityQueryReport/QualityParam");
			pXMLNODELIST paramList = rtnparser.GetNodeList(paramNode);
			int count=rtnparser.GetChildCount(paramNode);
			for(int k=0;k<count;k++)
			{
				string type,desc,val;
				pXMLNODE paramnode = rtnparser.GetNextNode(paramList);
				rtnparser.GetAttrNode(paramnode,"Type",type);
				rtnparser.GetAttrNode(paramnode,"Desc",desc);
				rtnparser.GetAttrNode(paramnode,"Value",val);
				if(type=="1")
				{
					string ip;
					int port;
					PROPMANAGER::instance()->GetQualityCardInfo(DVBType,ip,port);
					int Ifreq=StrUtil::Str2Int(freq);
					FetchQualityLevel * FetchLevel=new FetchQualityLevel(DVBType,ip,port);
					int level=FetchLevel->GetLevelFromCard(Ifreq);
					if(level>0)
					{   
						float f_level=(float)level/100.0;
						rtnparser.SetAttrNode("Value",StrUtil::Float2Str(f_level),paramnode);
						rtnparser.SaveToString(retDeviceXml);
					}
					delete FetchLevel;
					break;
				}					
			}
		}
		
		std::vector<sCheckParam> alarmVec;//报警信息处理
		std::vector<sCheckParam>::iterator itr;
		ReadyForAlarm(retDeviceXml,alarmVec);
		for(itr=alarmVec.begin();itr!=alarmVec.end();itr++)
		{
			ALARMMGR::instance()->CheckAlarm(*itr,true);
		}
		//获得指标查询结果
		if(GetQualityInfo(mVecStdStr[i],retDeviceXml,Freq,TaskID,vecqInfo)==false)
			continue;

		//指标信息入库
		for (size_t j =0;j!=vecqInfo.size();++j)
		{
			DBMANAGER::instance()->AddQualityInfo(DVBType,Freq,TaskID,vecqInfo[j],strTime);
		}
	}
	
	if (TaskScheduler != NULL)
		TaskScheduler->SetNextRunTimeEx();

	SetFinised();

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]指标任务停止 !\n",DeviceID));
}
string QualityTask::GetTaskName()
{
	return std::string("指标任务");
}
std::string QualityTask::GetObjectName()
{
	return std::string("QualityTask");
}
bool  QualityTask::GetQualityInfo(std::string orgXML,std::string rtnXML, std::string &Freq,std::string &taskID,std::vector<eQualityInfo> & vecQualityInfo)
{
	XmlParser parser(orgXML.c_str());
	XmlParser rtnParser(rtnXML.c_str());
	pXMLNODE  querySetNode= parser.GetNodeFromPath("Msg/QualityQuery");
	parser.GetAttrNode( querySetNode,"Freq",Freq );

	pXMLNODE paramNode = rtnParser.GetNodeFromPath("Msg/QualityQueryReport/QualityParam");
	pXMLNODELIST indexList = rtnParser.GetNodeList( paramNode );
	int count = rtnParser.GetChildCount( paramNode );
	std::string Type ="",Desc = "", Value = "";
	for ( int i = 0;i<count;i++ )
	{
		pXMLNODE indexNode = rtnParser.GetNextNode( indexList );
		//获取eQualityInfo结构体的值
		rtnParser.GetAttrNode( indexNode,"Type",Type );
		rtnParser.GetAttrNode( indexNode,"Desc",Desc );
		rtnParser.GetAttrNode( indexNode,"Value",Value );
		eQualityInfo  mqInfo;
		if (DVBType == CTTB && Type=="2")
		{
			mqInfo.type = "3";
		}
		else if (DVBType == CTTB && Type=="3")
		{
			mqInfo.type = "2";
		}
		else
		{
			mqInfo.type = Type;
		}
		
		mqInfo.desc = Desc;
		//mqInfo.valu = Value;
		mqInfo.valu = StrUtil::Float2Str(StrUtil::Str2Float(Value)/1000.0f);//2015-10-21广播电视指标任务值不需乘1000
		vecQualityInfo.push_back(mqInfo);
	}
	return true;
}

void QualityTask::SetDeciveID( int deviceid )
{
	DeviceID=deviceid;
}

bool QualityTask::AddTaskXml()
{
	XmlParser psr;
	psr.Set_xml(strStandardXML);
	pXMLNODE node=psr.GetNodeFromPath("Msg/QualityTask");
	if (node)
	{
		psr.SetAttrNode("DeviceID",DeviceID,node); //分配任务的deviceid
		psr.Get_xml(strStandardXML);
	}
	return DBMANAGER::instance()->AddXmlTask(DVBType,strStandardXML);
}

bool QualityTask::DelTaskXml()
{
	return DBMANAGER::instance()->DeleteTask(DVBType,strStandardXML);
}
bool QualityTask::ReadyForAlarm(std::string strXML,std::vector<sCheckParam>& alarmVec)
{
	//声明解析类的对象
	XmlParser devParser;
	devParser.Set_xml(strXML);
	string freq,STD,SymbolRate,type,desc,val;
	pXMLNODE devRootNode= devParser.GetRootNode();

	pXMLNODE ReportNode = devParser.GetNodeFromPath("Msg/QualityQueryReport");
	devParser.GetAttrNode(ReportNode,"Freq",freq);
	devParser.GetAttrNode(ReportNode,"STD",STD);
	devParser.GetAttrNode(ReportNode,"SymbolRate",SymbolRate);

	pXMLNODE paramNode = devParser.GetNodeFromPath("Msg/QualityQueryReport/QualityParam");

	pXMLNODELIST paramList = devParser.GetNodeList(paramNode);

	int count=devParser.GetChildCount(paramNode);

	for(int k=0;k<count;k++)
	{
		string add;
		pXMLNODE paramnode = devParser.GetNextNode(paramList);

		devParser.GetAttrNode(paramnode,"Type",type);
		devParser.GetAttrNode(paramnode,"Desc",desc);
		devParser.GetAttrNode(paramnode,"Value",val);
		int val_int = StrUtil::Str2Int(val);
		sCheckParam param;
		param.DVBType = this->GetDVBType();
		param.AlarmType = ALARM_FREQ;
		param.TypeID = type;
		param.TypeDesc = desc;
		param.STD = STD;
		param.Freq = freq;
		param.SymbolRate = SymbolRate;
		param.ChannelID = this->GetChannelID();
		if(DVBType==ATV||DVBType==RADIO||DVBType==AM||DVBType==CTV)
		{
			param.TypedValue = StrUtil::Float2Str((float)val_int);
		}
		else if(DVBType==DVBC||DVBType==CTTB||DVBType==DVBS)
		{
			param.TypedValue = StrUtil::Float2Str((float)val_int);
		}
		param.DeviceID = StrUtil::Int2Str(DeviceID);
		param.CheckTime=time(0);

		alarmVec.push_back(param);
	}
	return true;
}