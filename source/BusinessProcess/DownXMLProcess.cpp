///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：DownXMLProcess.cpp
// 创建者：jiangcheng
// 创建时间：2009-05-21
// 内容描述：获取通信层的任务指令（XML格式），解析指令并生成相应的任务实例，交给任务调度模块处理
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
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)下行XML处理线程开始执行 !\n"));

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
			if ((COMMUNICATIONMGR::instance()->AccessOrderReceive()->getq(mb,&OutTime)) != -1 && mb != NULL)//从队列中取出xml
			{
				if (mb->msg_type() == ACE_Message_Block::MB_BREAK)//是否要中断消息
				{
					mb->release();//释放内存
					break;
				}
				if (-1 == ProcessMessage(mb))//处理xml
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
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)下行XML处理线程停止执行 !\n"));

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

		string msg = "任务XML转化错误:" + strSrcXml;
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
		return 0;
	}

	TaskFactory *pTaskFactory = NULL;
	XMLTask *pXMLTask = NULL;

	pTaskFactory = new TaskFactory;

	if (NULL == pTaskFactory || strStdXml.empty())
	{
		std::string msg = "错误：无法生成任务工厂对象！";
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
	else
		pXMLTask = pTaskFactory->CreateTask(strStdXml);//根据收到xml生成相应的任务

	if (NULL == pXMLTask)
	{
		std::string msg = "错误：无法生成任务对象！";
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
	}
	else
	{
		BUSINESSLAYOUTMGR::instance()->AddTask(pXMLTask);//太耗时间，原因？？
	}

	if (NULL != pTaskFactory)//释放内存
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