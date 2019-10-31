#include "QualitySender.h"
#include "CommunicationMgr.h"
#include "../Foundation/OSFunction.h"
#include "ace/Synch.h"
#include "ace/OS.h"
#include <string>
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/XmlParser.h"
#include "./SysMsgSender.h"

QualitySender::QualitySender(int deviceid)
{
	ClientVec.clear();
	bFlag = false;
	DeviceID = deviceid;
}
QualitySender::QualitySender()
{
}
QualitySender::~QualitySender()
{

}
int QualitySender::Start()
{
	//�����߳�
	open(0);
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ʵʱָ�����ݷ����߳����� !\n"));

	return 0;
}

int QualitySender::open(void*)
{
	bFlag = true;
	//���û�����
	msg_queue()->high_water_mark(1024*10240);
	msg_queue()->low_water_mark(1024*10240);

	activate();

	return 0;
}

int QualitySender::svc()
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
			ClearInvalidClient();
			RecvTaskXml();
			ACE_Time_Value OutTime(ACE_OS::gettimeofday()+ACE_Time_Value(10));
	
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
			else if (++NoDataCount >= 90)
			{				
				NoDataCount = 0;
				ClearAllClient();//ɾ�����������û�	
				OSFunction::Sleep(0,10);
			}//����ͼ����һ���µ����� ���ࣿ������
		}
		catch (...)
		{
		}
	}
	bFlag = false;
	return 0;
}

int QualitySender::ProcessMessage(ACE_Message_Block* mb)
{
	if (mb == NULL || mb->length() < 0)
		return -1;

	ACE_Time_Value sendtimeout(3);//��ʱʱ��

	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);

	if(ClientVec.empty())
	{  
		OSFunction::Sleep(0,100);
		return 0;
	}
	std::vector<sVedioUserInfo>::iterator ptr = ClientVec.begin();
	for (;ptr!=ClientVec.end();)
	{
		int msgid=0;
		memcpy(&msgid,mb->rd_ptr(),sizeof(msgid));
		if(msgid==StrUtil::Str2Int((*ptr).MsgID))
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
				/*{
					(*ptr).client.close();
					ptr = ClientVec.erase(ptr);
				}*/
				break;
			}
			////�Ƴ���Ч�û�
			//if ((*ptr).fail_num >= 2/*10*/)//�޸���10��Ϊ2,����ҳ���ֹͣ,����ִ��ʱ�����2012-5-23 hjw
			//{
			//	(*ptr).client.close();
			//	ptr = ClientVec.erase(ptr);

			//	string msg = "ʵʱָ���û��˳�";
			//	SYSMSGSENDER::instance()->SendMsg(msg);
			//	ptr = ClientVec.begin();
			//	continue;
			//}
			break;
		}
		++ptr;
	}
	return 0;
}

bool QualitySender::AddClient(sVedioUserInfo newclient)
{
	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);
	ClientVec.push_back(newclient);
	return true;
}
bool QualitySender::HasClient(string msgid)
{
	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);
	for (std::vector<sVedioUserInfo>::iterator ptr = ClientVec.begin();ptr!=ClientVec.end();++ptr)
	{
		if((*ptr).MsgID==msgid)
			return true;
	}
	return false;
}

bool QualitySender::ClearAllClient()
{
	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);
	for (std::vector<sVedioUserInfo>::iterator ptr = ClientVec.begin();ptr!=ClientVec.end();++ptr)
	{
		(*ptr).client.close();

		string msg = "ָ�����ݷ���ʧ�ܣ�ָ���û�ǿ���˳�";
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSTINFO,OTHER,LOG_EVENT_DEBUG,true,false);
	}
	this->ClientVec.clear();
	return true;
}
int QualitySender::Stop()
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

bool QualitySender::RecvTaskXml2()
{
	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);

	std::vector<sVedioUserInfo>::iterator ptr = ClientVec.begin();
	for (;ptr!=ClientVec.end();++ptr)
	{
		// cout<<"\n<<<<���յ��ͻ�������Ϣ��"<< (*ptr).client.get_handle()<<"\tTCP����sum="<<ClientVec.size()<<endl;

		if ((*ptr).client.get_handle() == NULL)
		{
			(*ptr).client.close();
			ptr = ClientVec.erase(ptr);//�Ƴ����û�

			string msg = "ʵʱָ���û��˳�";
			SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSTINFO,OTHER,LOG_EVENT_DEBUG,true,false);
			break;
		}
		else //socket 
		{
			try{
				char sendBuf[1] = {0};
				ACE_Time_Value TimeOut(3);// ???(0,50000)
				ssize_t RecvCount = (*ptr).client.send(sendBuf,1,&TimeOut);
				// cout<<"\n<<<<���յ��ͻ�������Ϣ33311��"<< (*ptr).client.get_handle()<<sendBuf<<endl;
				if(RecvCount == SOCKET_ERROR)
				{
					(*ptr).client.close();
					ptr = ClientVec.erase(ptr);//�Ƴ����û�
					break;
				}
			}
			catch(...)
			{
				;
			}
		}
		//try
		//{
		//	char RecvBuf[1024] = {0};
		//	ACE_Time_Value TimeOut(3);// ???(0,50000)
		//	ssize_t RecvCount = (*ptr).client.recv(RecvBuf,1024,&TimeOut);
		//	cout<<"\n<<<<���յ��ͻ�������Ϣ222��"<< (*ptr).client.get_handle()<<RecvBuf<<endl;
		//	if(RecvCount < 4 || RecvCount == SOCKET_ERROR)
		//		continue;

		//	char* p = RecvBuf;
		//	for (int i=0;i<RecvCount;++i)//�ҵ�xml�ַ����Ŀ�ʼ֮��
		//	{
		//		if (*p == '<')
		//			break;
		//		p++;
		//	}

		//	string TaskXml = p;
		//	XmlParser parser(TaskXml.c_str());
		//	parser.SetAttrNode("MsgID",ptr->MsgID,parser.GetRootNode());
		//	parser.SaveToString(TaskXml);
		//	ACE_Message_Block *mbXML = new ACE_Message_Block(TaskXml.length());
		//	memcpy(mbXML->wr_ptr(),TaskXml.c_str(),TaskXml.length());
		//	mbXML->wr_ptr(TaskXml.length());


		//	if (mbXML != NULL)
		//	{
		//		ACE_Time_Value OutTime(ACE_OS::time(0)+1);
		//		COMMUNICATIONMGR::instance()->AccessOrderReceive()->putq(mbXML,&OutTime);
		//	}
		//}
		//catch(...)
		//{
		//	;
		//}
	}
	return true;
}

bool QualitySender::RecvTaskXml()
{
	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);

	std::vector<sVedioUserInfo>::iterator ptr = ClientVec.begin();
	for (;ptr!=ClientVec.end();++ptr)
	{
		if ((*ptr).client.get_handle() == NULL)
		{
			return false;
		}
		try
		{
			char RecvBuf[4096] = {0};
			ACE_Time_Value TimeOut(1);
			ssize_t RecvCount = (*ptr).client.recv(RecvBuf,4096,&TimeOut);

			if(RecvCount < 4 || RecvCount == SOCKET_ERROR)
			{
				++(*ptr).fail_num;
				return false;
			}
			(*ptr).fail_num = 0;
			char* p = RecvBuf;
			for (int i=0;i<RecvCount;++i)//�ҵ�xml�ַ����Ŀ�ʼ֮��
			{
				if (*p == '<')
					break;
				p++;
			}

			string TaskXml = p;
			XmlParser parser(TaskXml.c_str());
			parser.SetAttrNode("MsgID",ptr->MsgID,parser.GetRootNode());
			parser.SaveToString(TaskXml);
			ACE_Message_Block *mbXML = new ACE_Message_Block(TaskXml.length());
			memcpy(mbXML->wr_ptr(),TaskXml.c_str(),TaskXml.length());
			mbXML->wr_ptr(TaskXml.length());


			if (mbXML != NULL)
			{
				ACE_Time_Value OutTime(ACE_OS::time(0)+1);
				COMMUNICATIONMGR::instance()->AccessOrderReceive()->putq(mbXML,&OutTime);
			}
		}
		catch(...)
		{
			;
		}
	}
	return true;
}



bool QualitySender::ClearInvalidClient()
{
	ACE_Guard<ACE_Thread_Mutex> guard(QueueMutex);
	for (std::vector<sVedioUserInfo>::iterator ptr = ClientVec.begin();ptr!=ClientVec.end();++ptr)
	{
		if((*ptr).fail_num>60)
		{
			(*ptr).client.close();
			ptr = ClientVec.erase(ptr);
			string msg = "ָ��Socket,����ʧ��ֱ���˳���Socket�û�ǿ���˳�";
			SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSTINFO,OTHER,LOG_EVENT_DEBUG,true,false);
			break;
		}
	}
	return true;
}