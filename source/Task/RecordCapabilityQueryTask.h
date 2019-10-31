#pragma once

#include "DeviceIndependentTask.h"

class RecordCapabilityQueryTask : public DeviceIndependentTask
{
public:
	RecordCapabilityQueryTask();
	RecordCapabilityQueryTask(std::string strXML);
	virtual ~RecordCapabilityQueryTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
};