///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����DeciceDataDeal.h
// �����ߣ�gaoxd
// ����ʱ�䣺2009-12-14
// ����������Ӳ�����ݴ����߳�
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ace/Task.h"
#include "ace/Sock_Stream.h"
#include <string>
#include <vector>
#include "../DeviceAccess/CardType.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/StrUtil.h"

class DeciceDataDeal : public ACE_Task<ACE_MT_SYNCH>
{
public:
	DeciceDataDeal(ACE_SOCK_Stream socket);
	~DeciceDataDeal();
private:
	DeciceDataDeal();
public:
	int open(void*);
	int Stop();

	virtual int svc();

	bool TaskFinished(){return Finished;};

private:
	bool RecvInfo(char* Info,const int buflen,ssize_t& rcvlen);
	bool RecvInfoEx(char* Info,const int buflen,ssize_t& rcvlen);
	bool ProcessXmlInfo(const std::string& Info);
	bool ProcessProgramAlarm(const char* buf,const int len);
	bool ProcessTableEIT(const std::string& Info);	//EIT �������⴦��
	void CollectAlarmType(int AlarmType,int deviceid,std::vector<sTypeDesc> &vTypeDesc);
private:
	ACE_SOCK_Stream mSocket;
	bool Finished;
};
