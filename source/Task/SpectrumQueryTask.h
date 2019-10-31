#pragma once 

#include "DeviceIndependentTask.h"

class SpectrumQueryTask : public DeviceIndependentTask
{
public:
	SpectrumQueryTask();
	SpectrumQueryTask(std::string strXML);
	virtual ~SpectrumQueryTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
};