
#include "SpectrumQueryTask.h"
#include "TranslateDownXML.h"
#include "ace/Log_Msg.h"
#include "../Foundation/TimeUtil.h"
#include "../DBAccess/DBManager.h"
#include "../Foundation/StrUtil.h"
#include "TranslateUpXML.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/OSFunction.h"

SpectrumQueryTask::SpectrumQueryTask() : DeviceIndependentTask()
{

}

SpectrumQueryTask::SpectrumQueryTask(std::string strXML) : DeviceIndependentTask(strXML)
{
}

SpectrumQueryTask::~SpectrumQueryTask()
{
}

void SpectrumQueryTask::Run()
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]Ƶ�ײ�ѯ����ִ�� !\n",DeviceID));
	SetRunning();
	XmlParser psr;
	psr.Set_xml(strStandardXML);
	RetValue = RUN_SUCCESS;
	std::string taskid,equcode,startdatetime,enddatetime,reporttime;
	std::string STD;
	pXMLNODE childnode=psr.GetNodeFromPath("Msg/SpectrumQuery");
	try
	{
		psr.GetAttrNode(childnode,string("EquCode"),equcode);        //��ȡQuality�ڵ��ReportTime����
		psr.GetAttrNode(childnode,string("TaskID"),taskid);                //��ȡQuality�ڵ��TaskID����
		psr.GetAttrNode(childnode,string("StartDateTime"),startdatetime);  //��ȡQuality�ڵ��StartDateTime����
		psr.GetAttrNode(childnode,string("EndDateTime"),enddatetime);      //��ȡQuality�ڵ��EndDateTime����

	}
	catch(...)
	{
		string error = "��ȡSpectrumQuery�ڵ�����ʧ��";
		APPLOG::instance()->WriteLog( OTHER,LOG_EVENT_ERROR,error,LOG_OUTPUT_BOTH );
		SetFinised();
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]Ƶ�ײ�ѯ����ֹͣ !\n",DeviceID));
	}
	vector<eSpecInfo> vecSpecInfo;
	//��ѯָ����������Ϣ
	if( DBMANAGER::instance()->QuerySpecInfo(DVBType,taskid,vecSpecInfo,startdatetime,enddatetime) ==false )
	{
		RetValue = DATABASEERROR;
	}
	
	string sxml = TranslateUpXML::TranslateSpectrumQuery(this,vecSpecInfo);
	SendXML(sxml);
	// cout<<"\n���Ƿ��͵�Ƶ������<<<<<<<<<<<<<<<"<<sxml<<"<<<<<<<<<<<<<<<\n";

	
	//��������͵�ǰ��ϵͳ
	// OSFunction::Sleep(3); //�ȴ����ͽ���
	
	SetFinised();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]Ƶ�ײ�ѯ����ֹͣ !\n",DeviceID));
}

std::string SpectrumQueryTask::GetTaskName()
{
	return "Ƶ�ײ�ѯ����";
}

std::string SpectrumQueryTask::GetObjectName()
{
	return std::string("QualityQueryTask");
}