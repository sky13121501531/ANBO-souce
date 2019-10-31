#pragma once

#include "DeviceIndependentTask.h"

class CASEntitlementQueryTask : public DeviceIndependentTask
{
public:
	CASEntitlementQueryTask();
	CASEntitlementQueryTask(std::string strXML);
	virtual ~CASEntitlementQueryTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
};