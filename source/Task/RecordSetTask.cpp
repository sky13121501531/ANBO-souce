///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����RecordSetTask.cpp
// �����ߣ�jiangcheng
// ����ʱ�䣺2009-06-01
// �����������Զ�¼������������
///////////////////////////////////////////////////////////////////////////////////////////
#include "RecordSetTask.h"
#include "TranslateDownXML.h"
#include "TranslateUpXML.h"
#include "RecordTask.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include "../DBAccess/DBManager.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/StrUtil.h"
#include <vector>
#include <iostream>
#include "ace/Task.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
using namespace std;

RecordSetTask::RecordSetTask() : DeviceIndependentTask()
{

}

RecordSetTask::RecordSetTask(std::string strXML) : DeviceIndependentTask(strXML)
{
	
}

RecordSetTask::~RecordSetTask()
{

}
void RecordSetTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]����¼������ִ�� !\n",DeviceID));
	bRun = true;
	SetRunning();

	std::string Action;
	std::vector<std::string> TaskXMLVec;
	std::vector<XMLTask*> vecTask;
	RetValue = RUN_SUCCESS;
	TranslateDownXML::TranslateRecordTask(strStandardXML,TaskXMLVec);
	//���⴦��ģ�����Զ�¼�������߼�
	//�����Զ�¼���߼�ͬһ����������ɾ��������ʷ�Զ�¼��		-- Modify by gxd 2010-11-02
	if ((GetDVBType() == ATV || GetDVBType() == RADIO || GetDVBType() == AM ||GetDVBType() == CTV) && strStandardXML.find("AutoRecord")!=string::npos)
	{
		BUSINESSLAYOUTMGR::instance()->DelRecInfo(GetDVBType(),"0");	//ɾ�����������Զ�¼��,����ɾ����ִ�ж���
		BUSINESSLAYOUTMGR::instance()->DelRecInfo(AM, "0");	//ɾ��AM���������Զ�¼��,����ɾ����ִ�ж���
		DBMANAGER::instance()->DeleteTask(GetDVBType(),string("Record TaskID=\"0\""));
		OSFunction::Sleep(1);
	}
	for (size_t i=0; i< TaskXMLVec.size();++i)
	{
		XmlParser psr;
		int deviceid = -1;
		psr.Set_xml(TaskXMLVec[i]);
		pXMLNODE recordNode;
		if(TaskXMLVec[i].find("AutoRecord")!=string::npos)
		{
			recordNode=psr.GetNodeFromPath("Msg/AutoRecord/Record");
		}
		else
		{
			recordNode=psr.GetNodeFromPath("Msg/TaskRecord/Record");
		}
		psr.GetAttrNode(recordNode,"Action",Action);
		psr.GetAttrNode(recordNode,"TaskID",TaskID);
	 	psr.GetAttrNode(recordNode,"DeviceID",deviceid);
		std::string strTmpFreq;
		psr.GetAttrNode(recordNode,"Freq",strTmpFreq);
		/*2018-10-15
		 *
		 *ɽ��ͭ�� û��AM�ӿڵ�����ҪAM��ҵ�� 
		 *
		 *����ֻ�Ǽ򵥵��ж�Ƶ��Ĵ�С��С��ĳһֵ�ͼ���ΪAMҵ��
		 */
// 		if(StrUtil::Str2Float(strTmpFreq) < 30.0)
// 		{
// 			pXMLNODE rootNode = psr.GetNodeFromPath( "Msg" );
// 			psr.SetAttrNode("DVBType", std::string("AM"), rootNode);
// 			psr.SaveToString(TaskXMLVec[i]);
// 		}
 		//if ((GetDVBType()==ATV) && (deviceid !=-1))
		//{
		//	deviceid = deviceid+1;
		//}
		//if ((GetDVBType()==RADIO) && (deviceid !=-1))
		//{
		//	deviceid = deviceid+9;
		//}

		if(Action=="Set"||Action=="set")
		{
			RecordTask* recordtask = new RecordTask(TaskXMLVec[i]);
			BUSINESSLAYOUTMGR::instance()->AddRecTask(recordtask);
			
			vecTask.push_back(recordtask); //
			// OSFunction::Sleep(0,500);
		 }
		else if(Action=="Del"||Action=="del")
		{	
			if(deviceid != 0 && !BUSINESSLAYOUTMGR::instance()->DelTask(deviceid,TaskID))
			{
				RetValue=DATABASEERROR;
			}
			else if(deviceid == 0 && !BUSINESSLAYOUTMGR::instance()->DelTask(TaskID))
			{
				RetValue=DATABASEERROR;
			}
			RetValue=RUN_SUCCESS;
		}
	}
	SendXML(TranslateUpXML::TranslateRecord(this,vecTask));//���͵�ǰ��
	OSFunction::Sleep(1);
	SetFinised();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]����¼������ֹͣ !\n",DeviceID));
}

std::string RecordSetTask::GetObjectName()
{
	return std::string("RecordSetTask");
}
string RecordSetTask::GetTaskName()
{
	return "����¼������";
}
