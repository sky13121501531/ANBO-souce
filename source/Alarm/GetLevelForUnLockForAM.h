#pragma once

///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����GetLevelForUnLockForAM.h
// �����ߣ�gaoxd
// ����ʱ�䣺2010-09-06
// ������������ȡAM��ƽֵ�������ز������ź�
///////////////////////////////////////////////////////////////////////////////////////////
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include <string>
#include <vector>
#include "../Foundation/TypeDef.h"
#include <ace/Task.h>
#include "../DeviceAccess/TCPDeviceAccess.h"

class GetLevelForUnLockForAM:public ACE_Task<ACE_MT_SYNCH>
{
public:
	GetLevelForUnLockForAM(void);
	~GetLevelForUnLockForAM(void);

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
	TCPDeviceAccess * Device;

	std::vector<sSignalCheck>  TempInfo;
	std::vector<sSignalCheck>  AlarmRecordInfo;
	
	ACE_Thread_Mutex CheckMutex;
	bool bFlag;
};
typedef ACE_Singleton<GetLevelForUnLockForAM,ACE_Mutex>  GETLEVELFORUNLOCKFORAM;
