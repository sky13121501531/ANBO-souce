///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����QualityTask.h
// �����ߣ�gaoxd
// ����ʱ�䣺2009-06-25
// ����������ָ��������
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "DeviceRelatedTask.h"

class QualityTask : public DeviceRelatedTask
{
public:
	QualityTask();
	QualityTask(std::string strXML);
	virtual ~QualityTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
	virtual void SetDeciveID(int deviceid);
	virtual bool AddTaskXml();
	virtual bool DelTaskXml();
protected:
    bool  GetQualityInfo(std::string orgXML,std::string rtnXML, std::string &Freq,std::string &taskID,std::vector<eQualityInfo> & vecQualityInfo);
	bool ReadyForAlarm(std::string strXML,std::vector<sCheckParam>& alarmVec);
private:
	std::vector<std::string> mVecStdStr;
};