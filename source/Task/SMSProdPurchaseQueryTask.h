#pragma once
#include "DeviceIndependentTask.h"
class SMSProdPurchaseQueryTask:public DeviceIndependentTask
{
public:
	SMSProdPurchaseQueryTask(void);
	SMSProdPurchaseQueryTask(std::string strXML);
	virtual~SMSProdPurchaseQueryTask(void);
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
private:
	bool CreateRXml(std::string URL,std::string XmlFromSMS, std::string& RXml);
};
