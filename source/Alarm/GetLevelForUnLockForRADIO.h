#pragma once

///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����GetLevelForUnLockForRADIO.h
// �����ߣ�gaoxd
// ����ʱ�䣺2010-09-06
// ������������ȡFM��ƽֵ�������ز������ź�
///////////////////////////////////////////////////////////////////////////////////////////
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include <string>
#include <vector>
#include "../Foundation/TypeDef.h"
#include <ace/Task.h>
#include "../DeviceAccess/RADIOHTTPDeviceAccess.h"
class AlarmSender;

#define LIST_MAX_SIZE 100
#define LEVEL_LOW_VALUE -10
#define LEVEL_HEIGH_VALUE 10

class GetLevelForUnLockForRADIO:public ACE_Task<ACE_MT_SYNCH>
{
public:
	GetLevelForUnLockForRADIO(void);
	~GetLevelForUnLockForRADIO(void);

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
	bool CheckSendAlarm();
	int GetLevel(std::string strfreq);
	time_t m_ChannelscanTime;
	AlarmSender* pAlarmSender;
	std::map<std::string,sCheckParam> RadioNosignalAlarm;
	std::map<std::string,sCheckParam> RadioNosignalAlarmSended;
protected:
	RADIOHTTPDeviceAccess * Device;

	std::vector<sSignalCheck>  TempInfo;
	std::vector<sSignalCheck>  AlarmRecordInfo;
	
	ACE_Thread_Mutex CheckMutex;
	bool bFlag;
};
typedef ACE_Singleton<GetLevelForUnLockForRADIO,ACE_Mutex>  GETLEVELFORUNLOCKFORRADIO;
