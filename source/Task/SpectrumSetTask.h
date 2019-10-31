#pragma once

#include "DeviceIndependentTask.h"

class SpectrumSetTask : public DeviceIndependentTask
{
public:
	SpectrumSetTask();
	SpectrumSetTask(std::string strXML);
	virtual ~SpectrumSetTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
};