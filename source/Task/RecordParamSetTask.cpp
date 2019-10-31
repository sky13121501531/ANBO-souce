
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
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]视频转码录像默认参数设置任务执行 !\n",DeviceID));
	bRun = true;

	SetRunning(); 

	RetValue = RUN_SUCCESS;

	SendXML(TranslateUpXML::TranslateRecordParamSet(this));//发送xml

	SetFinised(); 

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]视频转码录像默认参数设置任务停止 !\n",DeviceID));
}

string RecordParamSetTask::GetTaskName()
{
	return std::string("视频转码录像默认参数设置任务");
}
std::string RecordParamSetTask::GetObjectName()
{
	return std::string("RecordParamSetTask");
}

