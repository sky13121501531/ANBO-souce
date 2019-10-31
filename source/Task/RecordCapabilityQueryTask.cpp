
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
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]¼��·����ѯ����ִ�� !\n",DeviceID));
	bRun = true;

	SetRunning(); 
	RetValue = RUN_SUCCESS;

	SendXML(TranslateUpXML::TranslateRecordCapabilityQuery(this));//����xml

	SetFinised(); 

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]¼��·����ѯ����ֹͣ !\n",DeviceID));
}

string RecordCapabilityQueryTask::GetTaskName()
{
	return std::string("¼��·����ѯ����");
}
std::string RecordCapabilityQueryTask::GetObjectName()
{
	return std::string("RecordCapabilityQueryTask");
}

