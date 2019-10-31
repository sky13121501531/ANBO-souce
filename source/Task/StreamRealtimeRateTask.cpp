
#include "StreamRealtimeRateTask.h"
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

StreamRealtimeRateTask::StreamRealtimeRateTask() : DeviceIndependentTask()
{

}

StreamRealtimeRateTask::StreamRealtimeRateTask(std::string strXML) : DeviceIndependentTask(strXML)
{
}

StreamRealtimeRateTask::~StreamRealtimeRateTask()
{
}
void StreamRealtimeRateTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]实时视频流率任务执行 !\n",DeviceID));
	bRun = true;

	SetRunning(); 
	RetValue = RUN_SUCCESS;

	SendXML(TranslateUpXML::TranslateStreamRealtimeRate(this));//发送xml

	SetFinised(); 

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]实时视频流率任务停止 !\n",DeviceID));
}

string StreamRealtimeRateTask::GetTaskName()
{
	return std::string("实时视频流率任务");
}
std::string StreamRealtimeRateTask::GetObjectName()
{
	return std::string("StreamRealtimeRateTask");
}

