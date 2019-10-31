///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����AutoAnalysisTimeSetTask.h
// �����ߣ�zhangyc
// ����ʱ�䣺2009-11-27
// ��������������ҵ�����ʱ������������
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