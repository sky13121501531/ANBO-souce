///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����QualitySender.h
// �����ߣ�gaoxd
// ����ʱ�䣺2009-09-09
// ����������ʵʱָ�����ݷ����߳�
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ace/Task.h"
#include "ace/Singleton.h"
#include "../Foundation/TypeDef.h"
#include <vector>

class QualitySender : public ACE_Task<ACE_MT_SYNCH>
{
public:
	QualitySender(int deviceid);
	QualitySender();
	~QualitySender();
	
public:
	int Start();

	int open(void*);

	virtual int svc();

	int Stop();

	bool AddClient(sVedioUserInfo newclient);//����û�
	bool HasClient(string msgid);			//�ж��Ƿ�����Ƶ�û� 

private:
	bool ClearAllClient();//�����Ƶ�û�
	bool ClearInvalidClient(); //������Чsocket
	int ProcessMessage(ACE_Message_Block* mb);
	bool RecvTaskXml2();		
	bool RecvTaskXml();	//������ݽ��գ���Ӧ������Ŀ�Ķ��ʵʱָ������ͨ��ͬһ��Socketִ�е�ҵ���߼�
private:
	std::vector<sVedioUserInfo> ClientVec;
	ACE_Thread_Mutex QueueMutex;
	bool bFlag;
	int DeviceID;
};

typedef ACE_Singleton<QualitySender,ACE_Mutex>  REALTIMEQUALITYSENDER;