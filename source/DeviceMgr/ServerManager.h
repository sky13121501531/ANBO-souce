///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：ServerManager.h
// 创建者：gaoxd
// 创建时间：2011-04-22
// 内容描述：设备管理（重启）类
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ace/Task.h"
#include "../Foundation/TypeDef.h"
#include <string>

class ServerManager : public ACE_Task<ACE_MT_SYNCH>
{
public:
	ServerManager();
	~ServerManager();
public:
	int Start();
	int Stop();
	int open(void*);
	virtual int svc();
	void SetStatus(int Type,std::string action,std::string strTime);
private:
	bool SetNextRunTime();
	bool RebootServer();

private:
	bool bFlag;

	std::string ManageType;				//ManageType 取值范围 week,day,single;
	std::string ManageMonthday;
	std::string ManageWeekday;
	std::string ManageSingledatetime;
	std::string ManageTime;
	std::string Action;
	std::string mNextRunTime;	
	eCheckType meCheckType;
};

typedef  ACE_Singleton<ServerManager,ACE_Mutex>  SEVERMANAGER;