///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：AutoAnalysisTimeSetTask.h
// 创建者：zhangyc
// 创建时间：2009-11-27
// 内容描述：数据业务分析时间设置任务类
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "DeviceIndependentTask.h"

class AutoAnalysisTimeSetTask : public DeviceIndependentTask
{
public:
	AutoAnalysisTimeSetTask();
	AutoAnalysisTimeSetTask(std::string strXML);
	virtual ~AutoAnalysisTimeSetTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
private:
	std::string SetTaskXML(std::string strStandardXML,char * taskType);
};