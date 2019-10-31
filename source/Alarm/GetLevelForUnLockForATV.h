#pragma once

///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����GetLevelForUnLockForRADIO.h
// �����ߣ�gaoxd
// ����ʱ�䣺2010-09-06
// ������������ȡATV��ƽֵ�������ز������ź�
///////////////////////////////////////////////////////////////////////////////////////////
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include <string>
#include <vector>
#include "../Foundation/TypeDef.h"
#include <ace/Task.h>
#include "../DeviceAccess/TCPDeviceAccess.h"
#include "../DeviceAccess/HTTPDeviceAccess.h"

class GetLevelForUnLockForATV:public ACE_Task<ACE_MT_SYNCH>
{
public:
	GetLevelForUnLockForATV(void);
	~GetLevelForUnLockForATV(void);

public:
	bool Start();
	virtual int open(void*);
	virtual int Stop();
	virtual int svc();
public:
	bool AddRecordInfo(sSignalCheck Info);
	bool RemoveRecordInfo(sSignalCheck Info);

protected:
	bool InitDeviceSock();
	int GetLevel(std::string strfreq);
	
protected:
	HTTPDeviceAccess * Device;

	std::vector<sSignalCheck>  TempInfo;
	std::vector<sSignalCheck>  AlarmRecordInfo;
	
	ACE_Thread_Mutex CheckMutex;
	bool bFlag;
};
typedef ACE_Singleton<GetLevelForUnLockForATV,ACE_Mutex>  GETLEVELFORUNLOCKFORATV;
