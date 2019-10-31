
#include "ChannelScanQueryTask.h"
#include "TranslateDownXML.h"
#include "TranslateUpXML.h"
#include "Scheduler.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/PropManager.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../BusinessProcess/PSISIMgr.h"
#include "../DeviceAccess/DeviceAccessMgr.h"
#include "../DBAccess/DBManager.h"
#include <vector>
#include <iostream>
#include "ace/Task.h"
#include "../Foundation/XmlParser.h"
#include "./TranslateXMLForDevice.h"
#include "../Foundation/OSFunction.h"
using namespace std;


extern bool g_realqulity;

ChannelScanQueryTask::ChannelScanQueryTask() : DeviceIndependentTask()
{

}

ChannelScanQueryTask::ChannelScanQueryTask(std::string strXML) : DeviceIndependentTask(strXML)
{
	//TaskScheduler = new Scheduler();
	//TaskScheduler->SetRunTime(TimeUtil::GetCurDateTime());
}

ChannelScanQueryTask::~ChannelScanQueryTask()
{

}

void ChannelScanQueryTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)######Ƶ��ɨ�裺ͨ��[%d]Ƶ��ɨ������ִ�� !\n",DeviceID));
	bRun = true;
	g_realqulity = true;

	OSFunction::Sleep(0,500);
	SetRunning();
	string strRtnXml; //XML�ļ�����
	//����Ƶ��ɨ��

	if(!PROPMANAGER::instance()->IsScanFile(DVBType))
	{
		PSISIMGR::instance()->Init(DVBType,GetObjectName());
		PSISIMGR::instance()->CreateChannelXML(DVBType);
	}

	string strFileName;  //Ƶ��ɨ�����ļ�·��
	OSFunction::GetFileName(DVBType,GetObjectName(),strFileName);

	XmlParser parser;
	parser.LoadFromFile(strFileName.c_str());
	parser.Get_xml(strRtnXml); //��ȡƵ��ɨ����
	if (strRtnXml.empty())
	{
		SetRetValue(RUN_FAILED);
	}
	else
	{
		SetRetValue(RUN_SUCCESS);
	}

	
	SendXML(TranslateUpXML::TranslateChannelScan(this,strRtnXml));//����xml
	g_realqulity = false;
	//����Ƶ��ɨ����
    //CHANNELINFOMGR::instance()->UpdateChannelInfo(GetDVBType(), strRtnXml);

	bRun = false;
	if (TaskScheduler != NULL)
		TaskScheduler->ModifyExpiredTime(TimeUtil::DateTimeToStr(time(0)-1));

    SetFinised();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]Ƶ��ɨ������ֹͣ !\n",DeviceID));
}
string ChannelScanQueryTask::GetTaskName()
{
	return std::string("Ƶ��ɨ������");
}
std::string ChannelScanQueryTask::GetObjectName()
{
	return std::string("ChannelScanQueryTask");
}
