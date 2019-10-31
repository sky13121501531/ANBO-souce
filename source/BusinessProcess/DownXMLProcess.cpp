///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����DownXMLProcess.cpp
// �����ߣ�jiangcheng
// ����ʱ�䣺2009-05-21
// ������������ȡͨ�Ų������ָ�XML��ʽ��������ָ�������Ӧ������ʵ���������������ģ�鴦��
///////////////////////////////////////////////////////////////////////////////////////////
#include "DownXMLProcess.h"
#include "../Communications/CommunicationMgr.h"
#include "../Communications/SysMsgSender.h"
#include "../Foundation/OSFunction.h"
#include "../Task/TranslateDownXML.h"
#include "../Task/TaskFactory.h"
#include "../Task/XMLTask.h"
#include "../Foundation/AppLog.h"
#include "ace/OS.h"
#include "BusinessLayoutMgr.h"
#include "TaskMonitor.h"
#include "TaskRealTimeExecute.h"
#include "../DBAccess/DBManager.h"

DownXMLProcess::DownXMLProcess()
{
	bFlag = false;
}

DownXMLProcess::~DownXMLProcess()
{

}

int DownXMLProcess::Init()
{
	bFlag = true;
	return 0;
}

int DownXMLProcess::open(void*)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)����XML�����߳̿�ʼִ�� !\n"));

	msg_queue()->high_water_mark(1024*1024);
	msg_queue()->low_water_mark(1024*1024);
	this->activate();
	return 0;
}

int DownXMLProcess::svc()
{
	ACE_Message_Block *mb = 0;

	while (bFlag)
	{
		OSFunction::Sleep(0,50);
		try 
		{
			ACE_Time_Value OutTime(ACE_OS::gettimeofday()+ACE_Time_Value(2));
			if ((COMMUNICATIONMGR::instance()->AccessOrderReceive()->getq(mb,&OutTime)) != -1 && mb != NULL)//�Ӷ�����ȡ��xml
			{
				if (mb->msg_type() == ACE_Message_Block::MB_BREAK)//�Ƿ�Ҫ�ж���Ϣ
				{
					mb->release();//�ͷ��ڴ�
					break;
				}
				if (-1 == ProcessMessage(mb))//����xml
				{
					mb->release();
					break;
				}
				mb->release();
			}
		}
		catch (...)
		{
		}

	}

	bFlag = false;
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)����XML�����߳�ִֹͣ�� !\n"));

	return 0;
}

int DownXMLProcess::ProcessMessage(ACE_Message_Block* mb)
{
	std::string strStdXml,strSrcXml;
	bool invalidxml=true;
	if(mb->length()>0)
	{
		strSrcXml=std::string(mb->rd_ptr(),mb->length());
		invalidxml=false;
	}
	if (invalidxml || TranslateDownXML::TranslateTaskXML(strSrcXml,strStdXml) == false)
	{
		DBMANAGER::instance()->DeleteTask(UNKNOWN,strSrcXml);

		string msg = "����XMLת������:" + strSrcXml;
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
		return 0;
	}

	TaskFactory *pTaskFactory = NULL;
	XMLTask *pXMLTask = NULL;

	pTaskFactory = new TaskFactory;

	if (NULL == pTaskFactory || strStdXml.empty())
	{
		std::string msg = "�����޷��������񹤳�����";
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
	else
		pXMLTask = pTaskFactory->CreateTask(strStdXml);//�����յ�xml������Ӧ������

	if (NULL == pXMLTask)
	{
		std::string msg = "�����޷������������";
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
	else
	{
		BUSINESSLAYOUTMGR::instance()->AddTask(pXMLTask);//̫��ʱ�䣬ԭ�򣿣�
	}

	if (NULL != pTaskFactory)//�ͷ��ڴ�
	{
		delete pTaskFactory;
		pTaskFactory = NULL;
	}

	return 0;
}
int DownXMLProcess::Stop()
{
	bFlag = false;
	ACE_Message_Block* pMsgBreak;
	ACE_NEW_NORETURN(pMsgBreak, ACE_Message_Block (0, ACE_Message_Block::MB_BREAK) );
	msg_queue()->enqueue_head(pMsgBreak);
	this->wait();

	return 0;
}