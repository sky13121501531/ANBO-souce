
#include "ServerStatusSetTask.h"
#include "TranslateDownXML.h"
#include "TranslateUpXML.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/XmlParser.h"
#include <vector>
#include <iostream>
#include "ace/Task.h"
#include "../DBAccess/DBManager.h"
#include "../BusinessProcess/AlarmParamInfoMgr.h"
#include "../Foundation/TimeUtil.h"
#include "./TranslateDownXML.h"
#include "../Foundation/AlarmPropManager.h"
#include "../DeviceMgr/ServerManager.h"
using namespace std;

ServerStatusSetTask::ServerStatusSetTask() : DeviceIndependentTask()
{

}

ServerStatusSetTask::ServerStatusSetTask(std::string strXML) : DeviceIndependentTask(strXML)
{
}

ServerStatusSetTask::~ServerStatusSetTask()
{

}
void ServerStatusSetTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]设备运行状态设置任务执行 !\n",DeviceID));
	bRun = true;
	SetRunning();
	RetValue = RUN_SUCCESS;
	XmlParser parser;
	parser.Set_xml(strStandardXML);
	pXMLNODE SetNode=parser.GetNodeFromPath("Msg/ServerStatusSet");
	std::string EquCode,Action;
	parser.GetAttrNode(SetNode,"EquCode",EquCode);
	parser.GetAttrNode(SetNode,"Action",Action);
	pXMLNODELIST NodeList=parser.GetNodeList(SetNode);
	int count=parser.GetChildCount(SetNode);

	std::string DayOfWeek,StartTime,StartDateTime,URL;
	if(count==0)
	{
		pXMLNODE chilNode = parser.CreateNodePtr(SetNode,"SingleReportTime");
		parser.SetAttrNode("StartDateTime",TimeUtil::GetCurDateTime(),chilNode);
		parser.SaveToString(strStandardXML);
	}
	for(int i=0;i<count;i++)
	{
		pXMLNODE ChildNode=parser.GetNextNode(NodeList);

		parser.GetAttrNode(ChildNode,"DayOfWeek",DayOfWeek);//DayofWeek 和StartTime是循环任务包含的属性
		parser.GetAttrNode(ChildNode,"StartTime",StartTime);
		parser.GetAttrNode(ChildNode,"StartDateTime",StartDateTime);//StartDateTime 为单次任务包含的属性
		parser.GetAttrNode(ChildNode,"URL",URL);
	}
	if(TimeUtil::StrToDateTime(StartDateTime) - time(0) <= 0)
	{
		RetValue = RUN_FAILED;
		SendXML(TranslateUpXML::TranslateServerStatusSet(this));//发送回复xml
	}
	else
	{
		std::string strTime;
		int Type=0;
		if(count==0)
		{
			Type = -1;
			strTime = TimeUtil::GetCurDateTime();
		}
		else if(DayOfWeek=="")
		{
			Type = -1;
			strTime = StartDateTime;
		}
		else 
		{
			Type = StrUtil::Str2Int(DayOfWeek);
			strTime = StartTime;
		}

		SEVERMANAGER::instance()->SetStatus(Type,Action,strTime);
		DelTaskXml();
		AddTaskXml();
		SendXML(TranslateUpXML::TranslateServerStatusSet(this));//发送回复xml
	}
	SetFinised();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]设备运行状态设置任务停止 !\n",DeviceID));
}
string ServerStatusSetTask::GetTaskName()
{
	return "设备运行状态设置任务";
}
std::string ServerStatusSetTask::GetObjectName()
{
	return std::string("ServerStatusSetTask");
}
bool ServerStatusSetTask::DelTaskXml()
{
	return DBMANAGER::instance()->DeleteTask(UNKNOWN,strStandardXML);
}
bool ServerStatusSetTask::AddTaskXml()
{
	return DBMANAGER::instance()->AddXmlTask(UNKNOWN,strStandardXML);
}