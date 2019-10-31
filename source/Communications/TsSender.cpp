
#include "TsSender.h"
#include "../Foundation/OSFunction.h"
#include "ace/Synch.h"
#include "ace/OS.h"
#include <string>

#include "../DeviceAccess/TsFetcherMgr.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
//wz_0217
#include <list>
#include <map>
#include "../Foundation/PropManager.h"
//wz_0217

TsSender::TsSender(int deviceid)
{
	DeviceID = deviceid;//通道号
	ClientVec.clear();
	bFlag = false;
	bSendSwitch = true;
}

TsSender::~TsSender()
{

}
int TsSender::Start()
{
	//发送线程开始
	open(0);
	return 0;
}

int TsSender::open(void*)
{
	bFlag = true;
	//设置发送队列缓冲区大小
	msg_queue()->high_water_mark(188*2048);
	msg_queue()->low_water_mark(188*2048);

	activate();

	return 0;
}

int TsSender::svc()
{
	//wz_0217
	eDVBType dvbtype;
	bool ret = PROPMANAGER::instance()->IsRoundChannel(DeviceID);	//判断是否轮播通道
	if (!ret)
	{
		TSFETCHERMGR::instance()->SetTsSendTask(DeviceID,this);//从与通道号对应的ts流获取线程关联
	}
	else
	{
		PROPMANAGER::instance()->GetVirDevType(DeviceID, dvbtype);

		std::list<int> rounddevidlist;	//轮播通道列表
		PROPMANAGER::instance()->GetTaskDeviceList("StreamRealtimeRoundQueryTask",dvbtype,rounddevidlist);

		std::list<int>::iterator rounditer = rounddevidlist.begin();
		for(;rounditer != rounddevidlist.end();++rounditer)
		{
			TSFETCHERMGR::instance()->SetTsRoundTask(*rounditer, this);//从与通道号对应的ts流获取线程关联
		}
	}
	//wz_0217

	ACE_Message_Block *mb = 0;
	int NoDataCount = 0;
	while (bFlag)
	{
		try 
		{
			if (ClientVec.empty() == true || bSendSwitch == false)
			{
				msg_queue()->flush();
				OSFunction::Sleep(0,10);
				continue;
			}

			ACE_Time_Value OutTime(ACE_OS::gettimeofday()+ACE_Time_Value(2));

			if (getq(mb,&OutTime) != -1 && mb != NULL)
			{
				NoDataCount = 0;
				if (mb->msg_type() == ACE_Message_Block::MB_BREAK)//判断是否是终止发送消息
				{
					mb->release();
					break;	
				}
				if (-1 == ProcessMessage(mb))//发送ts数据到客户端
				{
					mb->release();
					break;
				}
				mb->release();
			}
			else if (++NoDataCount >= 30)//没有取到数据
			{
				NoDataCount = 0;
				ClearAllClient();			//删除所有连接用户
				OSFunction::Sleep(0,10);
			}
		}
		catch (...)
		{
		}
	}
	bFlag = false;
	return 0;
}

int TsSender::ProcessMessage(ACE_Message_Block* mb)
{
	if (mb == NULL || mb->length() < 0)
		return -1;

	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);

	ACE_Time_Value nowaittime (ACE_Time_Value::zero);
	std::vector<sVedioUserInfo>::iterator ptr = ClientVec.begin();
	for (;ptr!=ClientVec.end();)
	{
		int sendnum = (*ptr).client.send_n(mb->rd_ptr(),mb->length(),&nowaittime);//发送数据

		if (sendnum < 0)
		{
			++(*ptr).fail_num;
		}
		else
		{
			(*ptr).fail_num = 0;
		}
		//移除无效用户
		if ((*ptr).fail_num >= 30)
		{
			(*ptr).client.close();
			ptr = ClientVec.erase(ptr);
			
			ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]视频用户退出 !\n",DeviceID));
			continue;
		}
		++ptr;
	}
	return 0;
}

void TsSender::SetSendSwitch(bool sendswitch)
{
	bSendSwitch = sendswitch;

	/*if (sendswitch == true)
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]开始发送数据 !\n",DeviceID));
	}
	else
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]停止发送数据 !\n",DeviceID));
	}*/
}

bool TsSender::AddClient(sVedioUserInfo newclient)
{
	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);
	ClientVec.push_back(newclient);//添加客户端连接

	return true;
}
bool TsSender::HasClient()
{
	return !ClientVec.empty();
}

bool TsSender::GetAllClient(std::vector<sVedioUserInfo>& uservec)
{
	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);
	for (size_t num = 0; num < ClientVec.size();++num)
	{
		uservec.push_back(ClientVec[num]);//获得用户连接
	}

	return true;
}
bool TsSender::StopAllClient()
{
	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);
	for (std::vector<sVedioUserInfo>::iterator ptr = ClientVec.begin();ptr!=ClientVec.end();++ptr)
	{
		(*ptr).client.close();//关闭所有视频连接socket
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]视频用户被强制退出 !\n",DeviceID));
	}
	this->ClientVec.clear();
	bSendSwitch = true;
	return true;
}
bool TsSender::ClearAllClient()
{
	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);
	for (std::vector<sVedioUserInfo>::iterator ptr = ClientVec.begin();ptr!=ClientVec.end();++ptr)
	{
		(*ptr).client.close();//关闭所有视频连接socket
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]无数据，视频用户强制退出 !\n",DeviceID));
	}
	this->ClientVec.clear();
	bSendSwitch = true;
	return true;
}
int TsSender::Stop()
{
	bFlag = false;
	ACE_Message_Block* pMsgBreak;
	ACE_NEW_NORETURN(pMsgBreak, ACE_Message_Block (0, ACE_Message_Block::MB_BREAK) );
	msg_queue()->flush();
	msg_queue()->enqueue_head(pMsgBreak);
	this->wait();

	return 0;
}