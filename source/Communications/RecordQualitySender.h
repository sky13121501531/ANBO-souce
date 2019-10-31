
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

	bool AddClient(sVedioUserInfo newclient);//����û�
	bool HasClient(string msgid);			//�ж��Ƿ�����Ƶ�û� 

private:
	bool ClearAllClient();//�����Ƶ�û�
	bool ClearInvalidClient();
	int ProcessMessage(ACE_Message_Block* mb);
private:
	std::vector<sVedioUserInfo> ClientVec;
	ACE_Thread_Mutex QueueMutex;
	bool bFlag;
	int DeviceID;
};

typedef ACE_Singleton<RecordQualitySender,ACE_Mutex>  RECORDQUALITYSENDER;