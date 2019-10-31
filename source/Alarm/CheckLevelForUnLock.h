#pragma once

#include "ace/Synch.h"
#include "ace/Singleton.h"
#include <string>
#include <vector>
#include "../Foundation/TypeDef.h"
#include <ace/Task.h>
#include "../DeviceAccess/TCPDeviceAccess.h"

class CheckLevelForUnLock:public ACE_Task<ACE_MT_SYNCH>
{
public:
	CheckLevelForUnLock(void);
	~CheckLevelForUnLock(void);

public:
	bool Start();
	virtual int open(void*);
	virtual int Stop();
	virtual int svc();
public:
	bool AddRecordInfo(sSignalCheck Info);
	bool RemoveRecordInfo(sSignalCheck Info);

public:
	bool UpdateLevel(sSignalCheck Info);
	void AddQualityFreq(std::string freq);
	void RemoveQualityFreq(std::string freq);
	
protected:
	bool CheckLock(sSignalCheck param);
	bool GetQualityLevel(string freq,int& level);
	void ProcessQuality();
	void InsertLevelToDB();
protected:

	std::vector<sSignalCheck>  AlarmRecordInfo;
	ACE_Thread_Mutex CheckMutex;
	ACE_Thread_Mutex QualityMutex;
	bool bFlag;
	std::vector<std::string> vecFreq; //Â¼ÏñÆµµã

};
typedef ACE_Singleton<CheckLevelForUnLock,ACE_Mutex>  CHECKLEVELFORUNLOCK;
