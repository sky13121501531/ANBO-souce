
#include "RecordParamSetTask.h"
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

RecordParamSetTask::RecordParamSetTask() : DeviceIndependentTask()
{

}

RecordParamSetTask::RecordParamSetTask(std::string strXML) : DeviceIndependentTask(strXML)
{
}

RecordParamSetTask::~RecordParamSetTask()
{
}
void RecordParamSetTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]��Ƶת��¼��Ĭ�ϲ�����������ִ�� !\n",DeviceID));
	bRun = true;

	SetRunning(); 

	RetValue = RUN_SUCCESS;

	SendXML(TranslateUpXML::TranslateRecordParamSet(this));//����xml

	SetFinised(); 

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]��Ƶת��¼��Ĭ�ϲ�����������ֹͣ !\n",DeviceID));
}

string RecordParamSetTask::GetTaskName()
{
	return std::string("��Ƶת��¼��Ĭ�ϲ�����������");
}
std::string RecordParamSetTask::GetObjectName()
{
	return std::string("RecordParamSetTask");
}

