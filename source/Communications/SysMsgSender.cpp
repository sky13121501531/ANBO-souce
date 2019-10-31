
#include "SysMsgSender.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/AppLog.h"

SysMsgSender::SysMsgSender()
{
	bFlag = false;
	bSendFlag  =true;
	Print_Level = 0;
	WriteLog_Level = 0;

	//string RemoteIP = "225.1.1.10";
	//int RemotePort = 30000;

	//RemoteAddr.set(htons(RemotePort),inet_addr(RemoteIP.c_str()),0,0);

	//LocalAddr.set_port_number((u_short)0);
	//MsgSender.open(LocalAddr);
}

SysMsgSender::~SysMsgSender()
{
	MsgSender.close();
}

int SysMsgSender::open(void*)
{
	//�����߳�
	this->activate();
	return 0;
}

int SysMsgSender::Start()
{
	bFlag = true;
	this->open(0);
	return 0;
}

int SysMsgSender::svc()
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ϵͳ��Ϣ�����߳̿�ʼִ�� !\n"));

	ACE_Message_Block *mb = 0;
	ACE_Time_Value OutTime(ACE_OS::gettimeofday()+ACE_Time_Value(2));
	ACE_Time_Value SendOutTime(0,30);

	while (bFlag)
	{
		OSFunction::Sleep(0,100);
		try 
		{
			//if (getq(mb,&OutTime) != -1 && mb != NULL)
			//{
			//	MsgSender.send(mb->rd_ptr(),mb->length(),RemoteAddr,0,&SendOutTime);//��������
			//	mb->release();
			//}
		}
		catch (...)
		{
			ACE_DEBUG ((LM_DEBUG,"(%T | %t)ϵͳ��Ϣ�����쳣 !\n"));
		}
	}

	bFlag = false;
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ϵͳ��Ϣ�����߳�ִֹͣ�� !\n"));

	return 0;
}
bool SysMsgSender::SendMsg(std::string msg,enumDVBType DVBType,enumSysMsgType sysmsgtype,eModuleType Module,eLogType LogType,bool forceprint,bool forcelog)
{
	if (msg.empty())
		return false;

	if (bSendFlag == false)
		return true;

	//����̨���
	if ((forceprint == true) || (Print_Level==0 && sysmsgtype == VS_MSG_SYSALARM) ||				\
		(Print_Level==1 && (sysmsgtype == VS_MSG_SYSALARM || sysmsgtype == VS_MSG_PROALARM)) ||		\
		(Print_Level==2))
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)%s\n",msg.c_str()));
	}
	//д��־
	if ((forcelog == true) || (WriteLog_Level==0 && sysmsgtype == VS_MSG_SYSALARM) ||				\
		(WriteLog_Level==1 && (sysmsgtype == VS_MSG_SYSALARM || sysmsgtype == VS_MSG_PROALARM)) ||	\
		(WriteLog_Level==2))
	{
		APPLOG::instance()->WriteLog(Module,LogType,msg,LOG_OUTPUT_FILE);
	}

	//�鲥����
	//std::string strmsgtype = "VS_MSG_SYSTINFO";
	//switch (sysmsgtype)
	//{
	//case VS_MSG_SYSTINFO:
	//	strmsgtype = "VS_MSG_SYSTINFO";
	//	break;
	//case VS_MSG_PROALARM:
	//	strmsgtype = "VS_MSG_PROALARM";
	//	break;
	//case VS_MSG_SYSALARM:
	//	strmsgtype = "VS_MSG_SYSALARM";
	//	break;
	//default:
	//	strmsgtype = "VS_MSG_SYSTINFO";
	//	break;
	//}
	//
	//std::string strdvbtype = OSFunction::GetStrDVBType(DVBType);
	//if (strdvbtype == "UNKNOWN")
	//{
	//	strdvbtype = "SYSTEM";
	//}

	//string sysinfo = std::string("[") + strmsgtype + std::string("] ") +  std::string("[") + strdvbtype + std::string("] ") + msg;

	//ACE_Time_Value OutTime(ACE_OS::gettimeofday()+ACE_Time_Value(2));
	//ACE_Message_Block *mb = new ACE_Message_Block(sysinfo.length()+1);

	//memcpy(mb->wr_ptr(),sysinfo.c_str(),sysinfo.length());
	//mb->wr_ptr(sysinfo.length());
	//this->putq(mb,&OutTime);

	return true;
}

bool SysMsgSender::SetSendFlag(bool flag)
{
	bSendFlag = flag;
	return true;
}

int SysMsgSender::Stop()
{
	bFlag = false;
	this->wait();

	return 0;
}