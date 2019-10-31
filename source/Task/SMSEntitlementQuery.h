#pragma once
#include "DeviceIndependentTask.h"
class SMSEntitlementQuery:public DeviceIndependentTask
{
public:
	SMSEntitlementQuery(void);
	SMSEntitlementQuery(std::string strXML);
	virtual~SMSEntitlementQuery(void);
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
private:
	bool CreateRXml(std::string URL,std::string XmlFromSMS, std::string& RXml);
};
