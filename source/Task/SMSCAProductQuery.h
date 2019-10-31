#pragma once
#include "DeviceIndependentTask.h"
class SMSCAProductQuery:public DeviceIndependentTask
{
public:
	SMSCAProductQuery(void);
	SMSCAProductQuery(std::string strXML);
	virtual~SMSCAProductQuery(void);
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
private:
	bool CreateRXml(std::string URL,std::string XmlFromSMS, std::string& RXml);
};
