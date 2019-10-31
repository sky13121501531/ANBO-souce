
#include "RecordCapabilityQueryTask.h"
#include "TranslateDownXML.h"
#include "TranslateUpXML.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/TypeDef.h"
#include "../Communications/TsSenderMgr.h"
#include <vector>
#include <iostream>

using namespace std;

RecordCapabilityQueryTask::RecordCapabilityQueryTask() : DeviceIndependentTask()
{

}

RecordCapabilityQueryTask::RecordCapabilityQueryTask(std::string strXML) : DeviceIndependentTask(strXML)
{
}

RecordCapabilityQueryTask::~RecordCapabilityQueryTask()
{
}
void RecordCapabilityQueryTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]录像路数查询任务执行 !\n",DeviceID));
	bRun = true;

	SetRunning(); 
	RetValue = RUN_SUCCESS;

	SendXML(TranslateUpXML::TranslateRecordCapabilityQuery(this));//发送xml

	SetFinised(); 

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]录像路数查询任务停止 !\n",DeviceID));
}

string RecordCapabilityQueryTask::GetTaskName()
{
	return std::string("录像路数查询任务");
}
std::string RecordCapabilityQueryTask::GetObjectName()
{
	return std::string("RecordCapabilityQueryTask");
}

