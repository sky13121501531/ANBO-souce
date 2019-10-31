
#include "AudioParamSetTask.h"
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

AudioParamSetTask::AudioParamSetTask() : DeviceIndependentTask()
{
	
}

AudioParamSetTask::AudioParamSetTask(std::string strXML) : DeviceIndependentTask(strXML)
{
}

AudioParamSetTask::~AudioParamSetTask()
{
}
void AudioParamSetTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]��Ƶ������������ִ�� !\n",DeviceID));
	bRun = true;
	
	SetRunning(); 

	 RetValue = RUN_SUCCESS;

    SendXML(TranslateUpXML::TranslateAudioParamSet(this));//����xml

	SetFinised(); 

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]��Ƶ������������ֹͣ !\n",DeviceID));
}

string AudioParamSetTask::GetTaskName()
{
	return std::string("��Ƶ������������");
}
std::string AudioParamSetTask::GetObjectName()
{
	return std::string("AudioParamSetTask");
}

