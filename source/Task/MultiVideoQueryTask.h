///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����MultiVideoQueryTask.h
// ��������:�໭����Ƶ��ѯ����
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once 

#include "DeviceIndependentTask.h"


class MultiVideoQueryTask : public DeviceIndependentTask
{
public:
	MultiVideoQueryTask();
	MultiVideoQueryTask(std::string strXML);
	virtual ~MultiVideoQueryTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
};