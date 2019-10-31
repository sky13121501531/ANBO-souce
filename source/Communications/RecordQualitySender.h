
#pragma once

#include "ace/Task.h"
#include "ace/Singleton.h"
#include "../Foundation/TypeDef.h"
#include <vector>

class RecordQualitySender : public ACE_Task<ACE_MT_SYNCH>
{
public:
	RecordQualitySender(int deviceid);
	RecordQualitySender();
	~RecordQualitySender();

public:
	int Start();

	int open(void*);

	virtual int svc();

	int Stop();

	bool AddClient(sVedioUserInfo newclient);//添加用户
	bool HasClient(string msgid);			//判断是否有视频用户 

private:
	bool ClearAllClient();//清空视频用户
	bool ClearInvalidClient();
	int ProcessMessage(ACE_Message_Block* mb);
private:
	std::vector<sVedioUserInfo> ClientVec;
	ACE_Thread_Mutex QueueMutex;
	bool bFlag;
	int DeviceID;
};

typedef ACE_Singleton<RecordQualitySender,ACE_Mutex>  RECORDQUALITYSENDER;