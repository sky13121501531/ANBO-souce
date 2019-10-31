///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：MultiVideoQueryTask.h
// 内容描述:多画面视频查询任务
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