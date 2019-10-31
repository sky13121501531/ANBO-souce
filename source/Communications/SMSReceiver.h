///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����SMSReceiver.h
// �����ߣ�jiangcheng
// ����ʱ�䣺2009-05-20
// ��������������Ӧ��ϵͳ�´�XML��ʽ������
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include "DownOrderReceive.h"
#include <string>
#include <vector>

class SMSReceiver : public DownOrderReceive
{
public:
	SMSReceiver();
	~SMSReceiver();
public:
	int svc();
private:
	std::string ReceiveXML();
private:
	long lastMsgID;

public:
	void insertFileName(std::string strFileName);
	bool getSmsFileName(std::string taskName, std::string& retFileName);
private:
	std::vector<std::string> m_vecSmsFileName;
	ACE_Thread_Mutex m_visitMutex;
};
typedef ACE_Singleton<SMSReceiver,ACE_Mutex>  SMSRECEIVER;