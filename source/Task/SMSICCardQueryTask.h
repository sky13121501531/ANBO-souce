#pragma once
#include "DeviceIndependentTask.h"
class SMSICCardQueryTask:public DeviceIndependentTask
{
public:
	SMSICCardQueryTask(void);
	SMSICCardQueryTask(std::string strXML);
	virtual~SMSICCardQueryTask(void);
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
private:
	bool CreateRXml(std::string URL,std::string XmlFromSMS, std::string& RXml);
};
