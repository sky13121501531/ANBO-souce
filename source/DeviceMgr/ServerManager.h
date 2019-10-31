///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����ServerManager.h
// �����ߣ�gaoxd
// ����ʱ�䣺2011-04-22
// �����������豸������������
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

	std::string ManageType;				//ManageType ȡֵ��Χ week,day,single;
	std::string ManageMonthday;
	std::string ManageWeekday;
	std::string ManageSingledatetime;
	std::string ManageTime;
	std::string Action;
	std::string mNextRunTime;	
	eCheckType meCheckType;
};

typedef  ACE_Singleton<ServerManager,ACE_Mutex>  SEVERMANAGER;