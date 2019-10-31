///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：MultiVideoQueryTask.cpp
// 内容描述:多画面视频查询任务
///////////////////////////////////////////////////////////////////////////////////////////
#include "MultiVideoQueryTask.h"
#include "TranslateDownXML.h"
#include "ace/Log_Msg.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/PropManager.h"
#include "TranslateUpXML.h"
#include "../DBAccess/DBManager.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/OSFunction.h"
#include "../FileSysAccess/WPLFile.h"
#include <vector>
#include <string>
#include <fstream>
using namespace std;
#include "../Foundation/StrUtil.h"


MultiVideoQueryTask::MultiVideoQueryTask() : DeviceIndependentTask()
{

}

MultiVideoQueryTask::MultiVideoQueryTask(std::string strXML) : DeviceIndependentTask(strXML)
{
}

MultiVideoQueryTask::~MultiVideoQueryTask()
{

}

void MultiVideoQueryTask::Run()
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]多画面查询任务执行 !\n",DeviceID));
	SetRunning();

	bool ret=true;
	RetValue=RUN_SUCCESS;
	std::string strVideoStreamProtocol;
	PROPMANAGER::instance()->GetStreamProtocol(RADIO,strVideoStreamProtocol);

	char * cheader= "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?><Msg><Return/></Msg>";

	// 	string  MsgID ;
	// 	DBMANAGER::instance()->GetMsgID(MsgID);
	// 	int msgid=StrUtil::Str2Int(MsgID);
	// 	DBMANAGER::instance()->UpdateMsgID(StrUtil::Int2Str(++msgid));
	string   Desc ="",Comment = "";
	eTaskRetStatus retValue = GetRetValue();//任务的执行结果

	std::string retXml="";
	XmlParser retParser(cheader);
	pXMLNODE headRootNode = retParser.GetNodeFromPath( "Msg" );//根节点msg

	retParser.SetAttrNode( "Version",GetVersion(),headRootNode );//版本
	string strDateTime = TimeUtil::GetCurDateTime();
	string  MsgID = OSFunction::GetXmlMsgID(strDateTime);
	retParser.SetAttrNode( "MsgID",MsgID,headRootNode );//消息id属性
	retParser.SetAttrNode( "Type",std::string("RadioUp"),headRootNode );//消息类型
	retParser.SetAttrNode( "DateTime",strDateTime/*TimeUtil::GetCurDateTime()*/,headRootNode );//当前时间
	retParser.SetAttrNode( "SrcCode",GetDstCode(),headRootNode );//本机标识，可通过接口获得
	retParser.SetAttrNode( "DstCode",GetSrcCode(),headRootNode );//目标机器标识
	retParser.SetAttrNode( "DstURL", GetSrcURL(),headRootNode);//目标URL
	retParser.SetAttrNode( "ReplyID", GetMsgID(),headRootNode );//回复的消息id

	pXMLNODE retNode = retParser.GetNodeFromPath("Msg/Return");
	retParser.SetAttrNode( "Type",std::string("MultiVideoQuery"), retNode );

	retParser.SetAttrNode( "Value",retValue,retNode );//return节点的value属性
	retParser.SetAttrNode( "Desc",Desc,retNode );//return节点的Desc属性
	retParser.SetAttrNode( "Comment",Comment,retNode );//return节点的Comment属性
	retParser.SaveToString( retXml );

	pXMLNODE streamNode= retParser.CreateNodePtr(headRootNode,"MultiVideoReport");//创建Stream节点


	std::string strBaseUrl = strVideoStreamProtocol + "://" + PROPMANAGER::instance()->GetHttpVideoIp() + ":" + \
		PROPMANAGER::instance()->GetHttpVideoPort()+"/";

	int tmpIndex = 0; 
	//创建实时URL节点
	std::list<int> realTimeVideoList;
	PROPMANAGER::instance()->GetTaskDeviceList("StreamRealtimeQueryTask",ATV, realTimeVideoList);
	for(std::list<int>::iterator videoIter = realTimeVideoList.begin(); videoIter != realTimeVideoList.end(); videoIter++)
	{
		std::string strTsIP, strTsPort;
		PROPMANAGER::instance()->GetDeviceTsIP((*videoIter), strTsIP);
		PROPMANAGER::instance()->GetDeviceTsPort((*videoIter), strTsPort);
		pXMLNODE mediaStreamNode=retParser.CreateNodePtr(streamNode,"MediaStream");//创建Stream节点的子节点MediaStream节点
		retParser.SetAttrNode("Index",tmpIndex++,mediaStreamNode);//设置MediaStream节点的index属性
		retParser.SetAttrNode("DESC", std::string("实时视频"), mediaStreamNode);//设置MediaStream节点的DESC属性
		retParser.SetAttrNode("CODE", std::string(""), mediaStreamNode);//设置MediaStream节点的CODE属性
		retParser.SetAttrNode("FREQ", std::string(""), mediaStreamNode);//设置MediaStream节点的CODE属性
		retParser.SetAttrNode("NAME", std::string(""), mediaStreamNode);//设置MediaStream节点的NAME属性
		retParser.SetAttrNode("URL", strBaseUrl + StrUtil::Int2Str(*videoIter), mediaStreamNode);//设置MediaStream节点的URL属性
		retParser.SetAttrNode("UDP", std::string("udp://@") +  strTsIP + std::string(":") + strTsPort, mediaStreamNode);//设置MediaStream节点的UDP属性
	}


	std::list<int> realTimeAudioList;
	PROPMANAGER::instance()->GetTaskDeviceList("StreamRealtimeQueryTask",RADIO, realTimeAudioList);
	for(std::list<int>::iterator audioIter = realTimeAudioList.begin(); audioIter != realTimeAudioList.end(); audioIter++)
	{
		std::string strTsIP, strTsPort;
		PROPMANAGER::instance()->GetDeviceTsIP((*audioIter), strTsIP);
		PROPMANAGER::instance()->GetDeviceTsPort((*audioIter), strTsPort);
		pXMLNODE mediaStreamNode=retParser.CreateNodePtr(streamNode,"MediaStream");//创建Stream节点的子节点MediaStream节点
		retParser.SetAttrNode("Index",tmpIndex++,mediaStreamNode);//设置MediaStream节点的index属性
		retParser.SetAttrNode("DESC", std::string("实时音频"), mediaStreamNode);//设置MediaStream节点的DESC属性
		retParser.SetAttrNode("CODE", std::string(""), mediaStreamNode);//设置MediaStream节点的CODE属性
		retParser.SetAttrNode("FREQ", std::string(""), mediaStreamNode);//设置MediaStream节点的CODE属性
		retParser.SetAttrNode("NAME", std::string(""), mediaStreamNode);//设置MediaStream节点的NAME属性
		retParser.SetAttrNode("URL", strBaseUrl + StrUtil::Int2Str(*audioIter), mediaStreamNode);//设置MediaStream节点的URL属性
		retParser.SetAttrNode("UDP", std::string("udp://@") +  strTsIP + std::string(":") + strTsPort, mediaStreamNode);//设置MediaStream节点的UDP属性
	}


	//创建录像URL节点
	std::vector<sTaskInfo> taskInfoVec;
	BUSINESSLAYOUTMGR::instance()->QueryAllTaskInfo(RADIO, taskInfoVec);

	for(std::vector<sTaskInfo>::iterator tmpAudioIter = taskInfoVec.begin(); tmpAudioIter != taskInfoVec.end(); tmpAudioIter++)
	{
		if((*tmpAudioIter).taskname =="AutoRecord"||(*tmpAudioIter).taskname == "TaskRecord")
		{
			std::string strCode, strName, strTsIP, strTsPort;
			PROPMANAGER::instance()->GetDeviceTsIP((*tmpAudioIter).deviceid, strTsIP);
			PROPMANAGER::instance()->GetDeviceTsPort((*tmpAudioIter).deviceid, strTsPort);
			CHANNELINFOMGR::instance()->GetChannelCodeByFreq(RADIO, (*tmpAudioIter).freq, strCode);
			CHANNELINFOMGR::instance()->GetProNameByFreq(RADIO, (*tmpAudioIter).freq, strName);
			pXMLNODE mediaStreamNode=retParser.CreateNodePtr(streamNode,"MediaStream");//创建Stream节点的子节点MediaStream节点
			retParser.SetAttrNode("Index",tmpIndex++,mediaStreamNode);//设置MediaStream节点的index属性
			retParser.SetAttrNode("DESC", std::string("实时录音"), mediaStreamNode);//设置MediaStream节点的DESC属性
			retParser.SetAttrNode("CODE", strCode, mediaStreamNode);//设置MediaStream节点的CODE属性
			retParser.SetAttrNode("FREQ", (*tmpAudioIter).freq, mediaStreamNode);//设置MediaStream节点的CODE属性
			retParser.SetAttrNode("NAME", strName, mediaStreamNode);//设置MediaStream节点的NAME属性
			retParser.SetAttrNode("URL", strBaseUrl +  StrUtil::Int2Str((*tmpAudioIter).deviceid), mediaStreamNode);//设置MediaStream节点的URL属性
			retParser.SetAttrNode("UDP", std::string("udp://@") +  strTsIP + std::string(":") + strTsPort, mediaStreamNode);//设置MediaStream节点的UDP属性
		}	
	}

	taskInfoVec.clear();
	BUSINESSLAYOUTMGR::instance()->QueryAllTaskInfo(ATV, taskInfoVec);
	for(std::vector<sTaskInfo>::iterator tmpVideoIter = taskInfoVec.begin(); tmpVideoIter != taskInfoVec.end(); tmpVideoIter++)
	{
		if((*tmpVideoIter).taskname =="AutoRecord"||(*tmpVideoIter).taskname == "TaskRecord")
		{
			std::string strCode, strName, strTsIP, strTsPort;
			PROPMANAGER::instance()->GetDeviceTsIP((*tmpVideoIter).deviceid, strTsIP);
			PROPMANAGER::instance()->GetDeviceTsPort((*tmpVideoIter).deviceid, strTsPort);
			CHANNELINFOMGR::instance()->GetChannelCodeByFreq(ATV, (*tmpVideoIter).freq, strCode);
			CHANNELINFOMGR::instance()->GetProNameByFreq(ATV, (*tmpVideoIter).freq, strName);
			pXMLNODE mediaStreamNode=retParser.CreateNodePtr(streamNode,"MediaStream");//创建Stream节点的子节点MediaStream节点
			retParser.SetAttrNode("Index",tmpIndex++,mediaStreamNode);//设置MediaStream节点的index属性
			retParser.SetAttrNode("DESC", std::string("实时录像"), mediaStreamNode);//设置MediaStream节点的DESC属性
			retParser.SetAttrNode("CODE", strCode, mediaStreamNode);//设置MediaStream节点的CODE属性
			retParser.SetAttrNode("FREQ", (*tmpVideoIter).freq, mediaStreamNode);//设置MediaStream节点的CODE属性
			retParser.SetAttrNode("NAME", strName, mediaStreamNode);//设置MediaStream节点的NAME属性
			retParser.SetAttrNode("URL", strBaseUrl +  StrUtil::Int2Str((*tmpVideoIter).deviceid), mediaStreamNode);//设置MediaStream节点的URL属性
			retParser.SetAttrNode("UDP", std::string("udp://@") +  strTsIP + std::string(":") + strTsPort, mediaStreamNode);//设置MediaStream节点的UDP属性

		}	
	}
	retParser.SaveToString(retXml);
	SendXML(retXml);
	return;

}



std::string MultiVideoQueryTask::GetTaskName()
{
	return "多画面查询任务";
}

std::string MultiVideoQueryTask::GetObjectName()
{
	return std::string("MultiVideoQueryTask");
}