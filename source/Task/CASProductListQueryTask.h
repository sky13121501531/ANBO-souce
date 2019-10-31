#pragma once

#include "DeviceIndependentTask.h"

class CASProductListQueryTask : public DeviceIndependentTask
{
public:
	CASProductListQueryTask();
	CASProductListQueryTask(std::string strXML);
	virtual ~CASProductListQueryTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
};