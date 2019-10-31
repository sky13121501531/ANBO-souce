///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����MultiVideoQueryTask.cpp
// ��������:�໭����Ƶ��ѯ����
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
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]�໭���ѯ����ִ�� !\n",DeviceID));
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
	eTaskRetStatus retValue = GetRetValue();//�����ִ�н��

	std::string retXml="";
	XmlParser retParser(cheader);
	pXMLNODE headRootNode = retParser.GetNodeFromPath( "Msg" );//���ڵ�msg

	retParser.SetAttrNode( "Version",GetVersion(),headRootNode );//�汾
	string strDateTime = TimeUtil::GetCurDateTime();
	string  MsgID = OSFunction::GetXmlMsgID(strDateTime);
	retParser.SetAttrNode( "MsgID",MsgID,headRootNode );//��Ϣid����
	retParser.SetAttrNode( "Type",std::string("RadioUp"),headRootNode );//��Ϣ����
	retParser.SetAttrNode( "DateTime",strDateTime/*TimeUtil::GetCurDateTime()*/,headRootNode );//��ǰʱ��
	retParser.SetAttrNode( "SrcCode",GetDstCode(),headRootNode );//������ʶ����ͨ���ӿڻ��
	retParser.SetAttrNode( "DstCode",GetSrcCode(),headRootNode );//Ŀ�������ʶ
	retParser.SetAttrNode( "DstURL", GetSrcURL(),headRootNode);//Ŀ��URL
	retParser.SetAttrNode( "ReplyID", GetMsgID(),headRootNode );//�ظ�����Ϣid

	pXMLNODE retNode = retParser.GetNodeFromPath("Msg/Return");
	retParser.SetAttrNode( "Type",std::string("MultiVideoQuery"), retNode );

	retParser.SetAttrNode( "Value",retValue,retNode );//return�ڵ��value����
	retParser.SetAttrNode( "Desc",Desc,retNode );//return�ڵ��Desc����
	retParser.SetAttrNode( "Comment",Comment,retNode );//return�ڵ��Comment����
	retParser.SaveToString( retXml );

	pXMLNODE streamNode= retParser.CreateNodePtr(headRootNode,"MultiVideoReport");//����Stream�ڵ�


	std::string strBaseUrl = strVideoStreamProtocol + "://" + PROPMANAGER::instance()->GetHttpVideoIp() + ":" + \
		PROPMANAGER::instance()->GetHttpVideoPort()+"/";

	int tmpIndex = 0; 
	//����ʵʱURL�ڵ�
	std::list<int> realTimeVideoList;
	PROPMANAGER::instance()->GetTaskDeviceList("StreamRealtimeQueryTask",ATV, realTimeVideoList);
	for(std::list<int>::iterator videoIter = realTimeVideoList.begin(); videoIter != realTimeVideoList.end(); videoIter++)
	{
		std::string strTsIP, strTsPort;
		PROPMANAGER::instance()->GetDeviceTsIP((*videoIter), strTsIP);
		PROPMANAGER::instance()->GetDeviceTsPort((*videoIter), strTsPort);
		pXMLNODE mediaStreamNode=retParser.CreateNodePtr(streamNode,"MediaStream");//����Stream�ڵ���ӽڵ�MediaStream�ڵ�
		retParser.SetAttrNode("Index",tmpIndex++,mediaStreamNode);//����MediaStream�ڵ��index����
		retParser.SetAttrNode("DESC", std::string("ʵʱ��Ƶ"), mediaStreamNode);//����MediaStream�ڵ��DESC����
		retParser.SetAttrNode("CODE", std::string(""), mediaStreamNode);//����MediaStream�ڵ��CODE����
		retParser.SetAttrNode("FREQ", std::string(""), mediaStreamNode);//����MediaStream�ڵ��CODE����
		retParser.SetAttrNode("NAME", std::string(""), mediaStreamNode);//����MediaStream�ڵ��NAME����
		retParser.SetAttrNode("URL", strBaseUrl + StrUtil::Int2Str(*videoIter), mediaStreamNode);//����MediaStream�ڵ��URL����
		retParser.SetAttrNode("UDP", std::string("udp://@") +  strTsIP + std::string(":") + strTsPort, mediaStreamNode);//����MediaStream�ڵ��UDP����
	}


	std::list<int> realTimeAudioList;
	PROPMANAGER::instance()->GetTaskDeviceList("StreamRealtimeQueryTask",RADIO, realTimeAudioList);
	for(std::list<int>::iterator audioIter = realTimeAudioList.begin(); audioIter != realTimeAudioList.end(); audioIter++)
	{
		std::string strTsIP, strTsPort;
		PROPMANAGER::instance()->GetDeviceTsIP((*audioIter), strTsIP);
		PROPMANAGER::instance()->GetDeviceTsPort((*audioIter), strTsPort);
		pXMLNODE mediaStreamNode=retParser.CreateNodePtr(streamNode,"MediaStream");//����Stream�ڵ���ӽڵ�MediaStream�ڵ�
		retParser.SetAttrNode("Index",tmpIndex++,mediaStreamNode);//����MediaStream�ڵ��index����
		retParser.SetAttrNode("DESC", std::string("ʵʱ��Ƶ"), mediaStreamNode);//����MediaStream�ڵ��DESC����
		retParser.SetAttrNode("CODE", std::string(""), mediaStreamNode);//����MediaStream�ڵ��CODE����
		retParser.SetAttrNode("FREQ", std::string(""), mediaStreamNode);//����MediaStream�ڵ��CODE����
		retParser.SetAttrNode("NAME", std::string(""), mediaStreamNode);//����MediaStream�ڵ��NAME����
		retParser.SetAttrNode("URL", strBaseUrl + StrUtil::Int2Str(*audioIter), mediaStreamNode);//����MediaStream�ڵ��URL����
		retParser.SetAttrNode("UDP", std::string("udp://@") +  strTsIP + std::string(":") + strTsPort, mediaStreamNode);//����MediaStream�ڵ��UDP����
	}


	//����¼��URL�ڵ�
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
			pXMLNODE mediaStreamNode=retParser.CreateNodePtr(streamNode,"MediaStream");//����Stream�ڵ���ӽڵ�MediaStream�ڵ�
			retParser.SetAttrNode("Index",tmpIndex++,mediaStreamNode);//����MediaStream�ڵ��index����
			retParser.SetAttrNode("DESC", std::string("ʵʱ¼��"), mediaStreamNode);//����MediaStream�ڵ��DESC����
			retParser.SetAttrNode("CODE", strCode, mediaStreamNode);//����MediaStream�ڵ��CODE����
			retParser.SetAttrNode("FREQ", (*tmpAudioIter).freq, mediaStreamNode);//����MediaStream�ڵ��CODE����
			retParser.SetAttrNode("NAME", strName, mediaStreamNode);//����MediaStream�ڵ��NAME����
			retParser.SetAttrNode("URL", strBaseUrl +  StrUtil::Int2Str((*tmpAudioIter).deviceid), mediaStreamNode);//����MediaStream�ڵ��URL����
			retParser.SetAttrNode("UDP", std::string("udp://@") +  strTsIP + std::string(":") + strTsPort, mediaStreamNode);//����MediaStream�ڵ��UDP����
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
			pXMLNODE mediaStreamNode=retParser.CreateNodePtr(streamNode,"MediaStream");//����Stream�ڵ���ӽڵ�MediaStream�ڵ�
			retParser.SetAttrNode("Index",tmpIndex++,mediaStreamNode);//����MediaStream�ڵ��index����
			retParser.SetAttrNode("DESC", std::string("ʵʱ¼��"), mediaStreamNode);//����MediaStream�ڵ��DESC����
			retParser.SetAttrNode("CODE", strCode, mediaStreamNode);//����MediaStream�ڵ��CODE����
			retParser.SetAttrNode("FREQ", (*tmpVideoIter).freq, mediaStreamNode);//����MediaStream�ڵ��CODE����
			retParser.SetAttrNode("NAME", strName, mediaStreamNode);//����MediaStream�ڵ��NAME����
			retParser.SetAttrNode("URL", strBaseUrl +  StrUtil::Int2Str((*tmpVideoIter).deviceid), mediaStreamNode);//����MediaStream�ڵ��URL����
			retParser.SetAttrNode("UDP", std::string("udp://@") +  strTsIP + std::string(":") + strTsPort, mediaStreamNode);//����MediaStream�ڵ��UDP����

		}	
	}
	retParser.SaveToString(retXml);
	SendXML(retXml);
	return;

}



std::string MultiVideoQueryTask::GetTaskName()
{
	return "�໭���ѯ����";
}

std::string MultiVideoQueryTask::GetObjectName()
{
	return std::string("MultiVideoQueryTask");
}