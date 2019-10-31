
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
	return std::string("EPG查询任务");
}

void EPGQuery::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]EPG查询任务执行 !\n",DeviceID));

	bRun = true;
	SetRunning();

	PSISIMGR::instance()->Init(DVBType,GetObjectName());
	PSISIMGR::instance()->CreateEPGXML(DVBType);

	std::string strFileName;    //EPG文件路径
	OSFunction::GetFileName(DVBType,GetObjectName(),strFileName); //获得最新EPG文件
	//std::string strFileName = "EPGQuery20111215112815.zip";
    
	SetRetValue(RUN_SUCCESS);
	SendXML(TranslateUpXML::TranslateEPGQuery(this,strFileName)); //发送到前端
	SetFinised();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]EPG查询任务停止 !\n",DeviceID));
}

std::string EPGQuery::GetObjectName()
{
	return std::string("EPGQuery");
}
