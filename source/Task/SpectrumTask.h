#pragma once

#include "DeviceRelatedTask.h"

class SpectrumTask : public DeviceRelatedTask
{
public:
	SpectrumTask();
	SpectrumTask(std::string strXML);
	virtual ~SpectrumTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
	virtual void SetDeciveID(int deviceid);
	virtual bool AddTaskXml();
	virtual bool DelTaskXml();
protected:
	std::string CreateNewFreqAlarmXml(std::string freq,std::string level);
	bool  CheckNewFreqAlarm(std::string rtnXML,std::vector<std::string> vecChannel,std::vector<std::string> vecNewChannel);
	bool  GetSpecInfo(std::string rtnXML, std::string &taskID,std::vector<string> vecChannel);
private:
	bool CycleTask;
	float ReferLevel;
	
	int m_AMStartFreq;
	int m_AMEndFreq;
};