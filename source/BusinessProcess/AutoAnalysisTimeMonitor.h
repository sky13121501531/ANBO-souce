#pragma once

#include "ace/Task.h"
#include "../Foundation/TypeDef.h"
#include <string>

class AutoAnalysisTimeMonitor:public ACE_Task<ACE_MT_SYNCH>
{
public:
	AutoAnalysisTimeMonitor(void);
	~AutoAnalysisTimeMonitor(void);
public:
	int Start();
	int Stop();
	int open(void*);
	virtual int svc();

private:
	bool SetNextRunTime();
	bool SendTSAnalyzeOrder();
private:
	bool bFlag;

	std::string ManageType;				//ManageType È¡Öµ·¶Î§ week,day,single;
	std::string ManageMonthday;
	std::string ManageWeekday;
	std::string ManageSingleday;
	std::string ManageTime;

	std::string mNextRunTime;	
	eCheckType meCheckType;	
};
