///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����DeciceDataDealMgr.h
// �����ߣ�gaoxd
// ����ʱ�䣺2009-12-14
// ����������Ӳ�����ݴ����̹߳���
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ace/Task.h"
#include "../Foundation/TypeDef.h"
#include <vector>

class DeciceDataDeal;

class DeciceDataDealMgr : public ACE_Task<ACE_MT_SYNCH>
{
public:
	DeciceDataDealMgr();
	~DeciceDataDealMgr();
public:
	int open(void*);
	int Stop();

	virtual int svc();
public:
	bool AddDeviceSocket(ACE_SOCK_Stream socket);

private:
	bool bFlag;
	ACE_Thread_Mutex TaskExecuteMutex;
	std::vector<DeciceDataDeal*> mDeciceDataDealVec;
};