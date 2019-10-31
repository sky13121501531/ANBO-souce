#include "RecordQualitySender.h"
#include "CommunicationMgr.h"
#include "./SysMsgSender.h"
#include "../Foundation/OSFunction.h"
#include "ace/Synch.h"
#include "ace/OS.h"
#include <string>
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/XmlParser.h"
#include "../Alarm/CheckLevelForUnLock.h"

RecordQualitySender::RecordQualitySender(int deviceid)
{
	ClientVec.clear();
	bFlag = false;
	DeviceID = deviceid;
}
RecordQualitySender::RecordQualitySender()
{
}
RecordQualitySender::~RecordQualitySender()
{

}
int RecordQualitySender::Start()
{
	//�����߳�
	open(0);
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ʵʱָ�����ݷ����߳����� !\n"));

	return 0;
}

int RecordQualitySender::open(void*)
{
	bFlag = true;
	//���û�����
	msg_queue()->high_water_mark(1024*1024);
	msg_queue()->low_water_mark(1024*1024);

	activate();

	return 0;
}

int RecordQualitySender::svc()
{
	ACE_Message_Block *mb = 0;
	int NoDataCount = 0;
	while (bFlag)
	{
		try 
		{
			if(ClientVec.empty())
			{
				OSFunction::Sleep(0,100);
				continue;
			}
			ClearInvalidClient();//�Ƴ���Ч�ͻ�
			
			ACE_Time_Value OutTime(ACE_OS::gettimeofday()+ACE_Time_Value(6));

			if (getq(mb,&OutTime) != -1 && mb != NULL)
			{
				NoDataCount = 0;
				if (mb->msg_type() == ACE_Message_Block::MB_BREAK)
				{
					mb->release();
					break;
				}
				if (-1 == ProcessMessage(mb))
				{
					mb->release();
					break;
				}
				mb->release();
			}
			else if (++NoDataCount >= 30)
			{				
				NoDataCount = 0;
				ClearAllClient();//ɾ�����������û�	
				OSFunction::Sleep(0,100);
			}
		}
		catch (...)
		{
		}
	}
	bFlag = false;
	return 0;
}

int RecordQualitySender::ProcessMessage(ACE_Message_Block* mb)
{
	if (mb == NULL || mb->length() < 0)
		return -1;

	ACE_Time_Value sendtimeout(3);//��ʱʱ��

	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);

	std::vector<sVedioUserInfo>::iterator ptr = ClientVec.begin();
	for (;ptr!=ClientVec.end();)
	{
		int iFreq=0;
		memcpy(&iFreq,mb->rd_ptr(),sizeof(iFreq));
		int tempFreq=(int)(1000*StrUtil::Str2Float((*ptr).Freq));
		if(iFreq==tempFreq)
		{
			int sendnum = (*ptr).client.send_n(mb->rd_ptr()+4,mb->length()-4,&sendtimeout);//��������

			if (sendnum < 0)
			{
				(*ptr).fail_num = (*ptr).fail_num+61;
				string msg = "ʵʱָ�귢��ʧ��";
				SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSTINFO,OTHER,LOG_EVENT_DEBUG,true,false);
			}
			else
			{
				(*ptr).fail_num = 0;
			}
		}
		++ptr;
	}
	return 0;
}

bool RecordQualitySender::AddClient(sVedioUserInfo newclient)
{
	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);
	ClientVec.push_back(newclient);
	return true;
}
bool RecordQualitySender::HasClient(string msgid)
{
	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);
	for (std::vector<sVedioUserInfo>::iterator ptr = ClientVec.begin();ptr!=ClientVec.end();++ptr)
	{
		if((*ptr).MsgID==msgid)
			return true;
	}
	return false;
}

bool RecordQualitySender::ClearAllClient()
{
	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);
	for (std::vector<sVedioUserInfo>::iterator ptr = ClientVec.begin();ptr!=ClientVec.end();++ptr)
	{
		(*ptr).client.close();
		CHECKLEVELFORUNLOCK::instance()->RemoveQualityFreq((*ptr).Freq);

		string msg = "¼��ָ�����ݷ���ʧ�ܣ�ָ���û�ǿ���˳�";
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSTINFO,OTHER,LOG_EVENT_DEBUG,true,false);
	}
	this->ClientVec.clear();
	return true;
}
int RecordQualitySender::Stop()
{
	bFlag = false;
	ACE_Message_Block* pMsgBreak;
	ACE_NEW_NORETURN(pMsgBreak, ACE_Message_Block (0, ACE_Message_Block::MB_BREAK) );
	msg_queue()->flush();
	msg_queue()->enqueue_head(pMsgBreak);
	this->wait();

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ʵʱָ�����ݷ����߳�ֹͣ !\n"));

	return 0;
}



bool RecordQualitySender::ClearInvalidClient()
{
	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);
	for (std::vector<sVedioUserInfo>::iterator ptr = ClientVec.begin();ptr!=ClientVec.end();++ptr)
	{
		if((*ptr).fail_num>60)
		{
			CHECKLEVELFORUNLOCK::instance()->RemoveQualityFreq((*ptr).Freq);
			(*ptr).client.close();
			ptr = ClientVec.erase(ptr);
			string msg = "ʵʱָ���û��˳�: ָ��Socket,60�����������������ʧ�ܣ�Socket�û�ǿ���˳�";
			SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSTINFO,OTHER,LOG_EVENT_DEBUG,true,false);
			break;
		}
	}
	return true;
}