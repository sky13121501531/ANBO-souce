
#include "HDDefAudioParamSetTask.h"
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

HDDefAudioParamSetTask::HDDefAudioParamSetTask() : DeviceIndependentTask()
{

}

HDDefAudioParamSetTask::HDDefAudioParamSetTask(std::string strXML) : DeviceIndependentTask(strXML)
{
}

HDDefAudioParamSetTask::~HDDefAudioParamSetTask()
{
}
void HDDefAudioParamSetTask::Run(void)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]������ƵĬ�ϲ�����������ִ�� !\n",DeviceID));
	bRun = true;

	SetRunning(); 

	RetValue = RUN_SUCCESS;

	SendXML(TranslateUpXML::TranslateHDDefAudioParamSet(this));//����xml

	SetFinised(); 

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]������ƵĬ�ϲ�����������ֹͣ !\n",DeviceID));
}

string HDDefAudioParamSetTask::GetTaskName()
{
	return std::string("������ƵĬ�ϲ�����������");
}
std::string HDDefAudioParamSetTask::GetObjectName()
{
	return std::string("HDDefAudioParamSetTask");
}

