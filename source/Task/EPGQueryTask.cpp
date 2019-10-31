
#include "EPGQueryTask.h"
#include "TranslateUpXML.h"
#include "../Foundation/OSFunction.h"
#include "../BusinessProcess/PSISIParser.h"
#include "../BusinessProcess/PSISIMgr.h"

EPGQuery::EPGQuery() : DeviceIndependentTask()
{
	
}

EPGQuery::EPGQuery(std::string strXML) : DeviceIndependentTask(strXML)
{

}

EPGQuery::~EPGQuery()
{

}

string EPGQuery::GetTaskName()
{
	return std::string("EPG��ѯ����");
}

void EPGQuery::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]EPG��ѯ����ִ�� !\n",DeviceID));

	bRun = true;
	SetRunning();

	PSISIMGR::instance()->Init(DVBType,GetObjectName());
	PSISIMGR::instance()->CreateEPGXML(DVBType);

	std::string strFileName;    //EPG�ļ�·��
	OSFunction::GetFileName(DVBType,GetObjectName(),strFileName); //�������EPG�ļ�
	//std::string strFileName = "EPGQuery20111215112815.zip";
    
	SetRetValue(RUN_SUCCESS);
	SendXML(TranslateUpXML::TranslateEPGQuery(this,strFileName)); //���͵�ǰ��
	SetFinised();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]EPG��ѯ����ֹͣ !\n",DeviceID));
}

std::string EPGQuery::GetObjectName()
{
	return std::string("EPGQuery");
}
