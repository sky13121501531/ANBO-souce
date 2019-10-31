///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：SMSReceiver.h
// 创建者：jiangcheng
// 创建时间：2009-05-20
// 内容描述：接收应用系统下达XML格式的命令
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