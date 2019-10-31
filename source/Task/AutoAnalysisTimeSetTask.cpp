
#include "AutoAnalysisTimeSetTask.h"
#include "TranslateDownXML.h"
#include "TranslateUpXML.h"
#include "../BusinessProcess/PSISIMgr.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/XmlParser.h"
#include <vector>
#include <iostream>
#include "ace/Task.h"
#include "../DBAccess/DBManager.h"
#include "../Foundation/TimeUtil.h"
#include "TableQueryTask.h"
#include "EPGQueryTask.h"
#include "ChannelScanQueryTask.h"
#include "MHPQueryTask.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
using namespace std;

AutoAnalysisTimeSetTask::AutoAnalysisTimeSetTask() : DeviceIndependentTask()
{

}

AutoAnalysisTimeSetTask::AutoAnalysisTimeSetTask(std::string strXML) : DeviceIndependentTask(strXML)
{
}

AutoAnalysisTimeSetTask::~AutoAnalysisTimeSetTask()
{

}
void AutoAnalysisTimeSetTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]数据业务分析时间设置任务执行 !\n",DeviceID));
	bRun = true;
	SetRunning();
	XmlParser parser;
	parser.Set_xml(strStandardXML);
	pXMLNODE timeQueryNode = parser.GetNodeFromPath("Msg/AutoAnalysisTimeSet");
	pXMLNODELIST queryNodeList = parser.GetNodeList(timeQueryNode);
	int count = parser.GetChildCount(timeQueryNode);
	XMLTask * pXMLTask = NULL;
	std::string stdXML;


	SetRetValue(RUN_SUCCESS);
	SendXML(TranslateUpXML::TranslateAutoAnalysisTimeSet(this)); //发送到前端

	for (int i=0;i<count;i++)
	{
		std::string StartTime,Type;
        pXMLNODE node = parser.GetNextNode(queryNodeList);
		parser.GetAttrNode(node,"StartTime",StartTime);
		parser.GetAttrNode(node,"Type",Type);
        switch (atoi(Type.c_str()))
        {
		case 1:													//表、EPG查询
			PSISIMGR::instance()->CreateTableXML(DVBType);
			PSISIMGR::instance()->CreateEPGXML(DVBType);
			break;
		case 2:													//频道扫描
			PSISIMGR::instance()->CreateChannelXML(DVBType);				
			break;
		case 3:													//MHP查询
			break;
		default:												//全部业务
			PSISIMGR::instance()->CreateChannelXML(DVBType);
			PSISIMGR::instance()->CreateEPGXML(DVBType);
			PSISIMGR::instance()->CreateTableXML(DVBType);
		}
	}

   
	SetFinised();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]数据业务分析时间设置任务停止 !\n",DeviceID));
}
string AutoAnalysisTimeSetTask::GetTaskName()
{
	return "数据业务分析时间设置任务";
}
std::string AutoAnalysisTimeSetTask::GetObjectName()
{
	return std::string("AutoAnalysisTimeSetTask");
}
std::string AutoAnalysisTimeSetTask::SetTaskXML(std::string strStandardXML,char* taskType)
{ 
	char * source = "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?><Msg></Msg>";
	XmlParser parser(strStandardXML.c_str());
	XmlParser tempParser(source);
	string downType,version,msgid,protocol, dateTime, priority,srccode,dstcode,srcurl;//头信息
	//获取根元素
	pXMLNODE rootNode= parser.GetNodeFromPath( "Msg" );
	parser.GetAttrNode( rootNode,"Version",version );
	parser.GetAttrNode( rootNode,"MsgID",msgid );
	parser.GetAttrNode( rootNode,"Protocol",protocol );
	parser.GetAttrNode( rootNode,"DVBType",downType );
	parser.GetAttrNode( rootNode,"SrcCode",srccode );
	parser.GetAttrNode( rootNode,"DstCode",dstcode );
	parser.GetAttrNode( rootNode,"DateTime",dateTime );
	parser.GetAttrNode( rootNode,"SrcURL",srcurl );
	parser.GetAttrNode( rootNode,"Priority",priority );
	
	//设置头信息
	pXMLNODE tempRootNode = tempParser.GetNodeFromPath( "Msg" );
	tempParser.SetAttrNode( "Version",version,tempRootNode );
	tempParser.SetAttrNode( "MsgID",msgid,tempRootNode );
	tempParser.SetAttrNode( "DVBType",downType,tempRootNode );
	tempParser.SetAttrNode( "TaskType",string(taskType),tempRootNode );
	tempParser.SetAttrNode( "Protocol",protocol,tempRootNode );
	tempParser.SetAttrNode( "DateTime",dateTime,tempRootNode );
	tempParser.SetAttrNode( "SrcCode",srccode,tempRootNode );
	tempParser.SetAttrNode( "DstCode",dstcode,tempRootNode );
	tempParser.SetAttrNode( "SrcURL",srcurl,tempRootNode );
	tempParser.SetAttrNode( "Priority",priority,tempRootNode );

	pXMLNODE node = tempParser.CreateNodePtr("Msg",taskType);
    if (taskType =="TableQuery")
    {  
		pXMLNODE tableNode=tempParser.CreateNodePtr(node,"Table");
		std::string EquCode,STD,Freq,SymbolRate;
		tempParser.SetAttrNode("EquCode",EquCode,tableNode);
		tempParser.SetAttrNode("STD",STD,tableNode);
		tempParser.SetAttrNode("Freq",Freq,tableNode);
		tempParser.SetAttrNode("SymbolRate",SymbolRate,tableNode);
    }
	else if (taskType =="ChannelScanQuery")
	{
		pXMLNODE scanNode=tempParser.CreateNodePtr(node,"ChannelScan");
		std::string ScanTime,STD,Freq,SymbolRate,StartFreq,EndFreq,StepFreq;
		tempParser.SetAttrNode("ScanTime",ScanTime,scanNode);
		tempParser.SetAttrNode("STD",STD,scanNode);
		tempParser.SetAttrNode("Freq",Freq,scanNode);
		tempParser.SetAttrNode("SymbolRate",SymbolRate,scanNode);
		tempParser.SetAttrNode("StartFreq",StartFreq,scanNode);
		tempParser.SetAttrNode("EndFreq",EndFreq,scanNode);
		tempParser.SetAttrNode("StepFreq",StepFreq,scanNode);
	}
	else if(taskType =="MHPQuery")
	{
		std::string ScanTime;
		tempParser.SetAttrNode("ScanTime",ScanTime,node);
	}
	std::string rtnxml;
	tempParser.SaveToString(rtnxml);
	return rtnxml;
}