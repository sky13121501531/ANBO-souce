#pragma once

#include "DeviceIndependentTask.h"

class CASStaticsQueryTask : public DeviceIndependentTask
{
public:
	CASStaticsQueryTask();
	CASStaticsQueryTask(std::string strXML);
	virtual ~CASStaticsQueryTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
};