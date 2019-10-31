///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：DeviceManager.h
// 创建者：gaoxd
// 创建时间：2011-04-22
// 内容描述：设备管理（重启）类
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ace/Task.h"
#include "../Foundation/TypeDef.h"
#include <string>

class DeviceManager : public ACE_Task<ACE_MT_SYNCH>
{
public:
	DeviceManager();
	~DeviceManager();
public:
	int Start();
	int Stop();
	int open(void*);
	virtual int svc();

private:
	bool SetNextRunTime();
	bool RebootDevice();
	
private:
	bool bFlag;

	std::string ManageType;				//ManageType 取值范围 week,day,single;
	std::string ManageMonthday;
	std::string ManageWeekday;
	std::string ManageSingleday;
	std::string ManageTime;

	std::string mNextRunTime;	
	eCheckType meCheckType;
};