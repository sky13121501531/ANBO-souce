///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：DeciceDataDealMgr.h
// 创建者：gaoxd
// 创建时间：2009-12-14
// 内容描述：硬件数据处理线程管理
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