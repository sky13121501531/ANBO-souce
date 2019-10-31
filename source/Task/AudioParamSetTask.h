#pragma once

#include "DeviceIndependentTask.h"

class AudioParamSetTask : public DeviceIndependentTask
{
public:
	AudioParamSetTask();
	AudioParamSetTask(std::string strXML);
	virtual ~AudioParamSetTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
};