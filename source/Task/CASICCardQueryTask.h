#pragma once

#include "DeviceIndependentTask.h"

class CASICCardQueryTask : public DeviceIndependentTask
{
public:
	CASICCardQueryTask();
	CASICCardQueryTask(std::string strXML);
	virtual ~CASICCardQueryTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
};