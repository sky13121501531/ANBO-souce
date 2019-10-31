///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����RealTimeVideoServer.cpp
// �����ߣ�jiangcheng
// ����ʱ�䣺2009-05-20
// ����������ʵʱ��Ƶ�������˴�����
///////////////////////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4996)
#include "RealTimeVideoServer.h"
#include "CommunicationMgr.h"
#include "TsSenderMgr.h"
#include <string>
#include <sstream>
#include <iostream>
#include "../Foundation/OSFunction.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TypeDef.h"
#include "../Foundation/PropManager.h"

using namespace std;

const int MAXDEVICEIDNUM = 2;

RealTimeVideoServer::RealTimeVideoServer()
{
	bFlag = false;
}

RealTimeVideoServer::~RealTimeVideoServer()
{

}

int RealTimeVideoServer::Open(ACE_Addr &addr)
{
	if (peer_acceptor.open(addr) == -1)//����ʵʱ��Ƶ�������˿�
	{
		return -1;
	}
	else
	{
		ACE_DEBUG((LM_DEBUG,"(%T | %t)��Ƶ����ɹ�����................\n"));
	}
	return 0;
}

int RealTimeVideoServer::handle_input(ACE_HANDLE handle)
{
	ACE_SOCK_Stream client;//����socket
	ACE_INET_Addr clientAddr;//�ͻ��˵�ַ��Ϣ

	if (this->peer_acceptor.accept(client,&clientAddr) == -1)//���ܿͻ�������
	{
		ACE_DEBUG((LM_ERROR,"(%T | %t)�����û����Ӵ���\n"));
		client.close();
		return 0;
	}
	
	const int PKGSIZE = 188;
	int value =PKGSIZE*5000;	

	//����socket���ͽ��ջ���
	client.set_option(SOL_SOCKET,SO_SNDBUF,(char*)&value,sizeof(value));
	client.set_option(SOL_SOCKET,SO_RCVBUF,(char*)&value,sizeof(value));

	list<int> userdeviceilist;
	if (false == RecvDeviceID(client,userdeviceilist)) //�����û�����,�õ�ͨ����
	{
		ACE_DEBUG((LM_DEBUG,"(%T | %t)�û�(%s:%d):��������ʧ��\n",clientAddr.get_host_addr(),clientAddr.get_port_number()));
		client.close();
		return 0;
	}
	
	if (userdeviceilist.empty())
	{
		ACE_DEBUG((LM_DEBUG,"(%T | %t)�û�(%s:%d):��ͨ������\n",clientAddr.get_host_addr(),clientAddr.get_port_number()));
		client.close();
		return 0;
	}
	//�����û�ͨ������
	list<int>::iterator ptr_user = userdeviceilist.begin();
	for (;ptr_user!=userdeviceilist.end();++ptr_user)
	{
		if (false == PROPMANAGER::instance()->IsDeviceAvaiable(*ptr_user) &&
			false == PROPMANAGER::instance()->IsRoundChannel(*ptr_user))
		{
			ACE_DEBUG((LM_DEBUG,"(%T | %t)�û�(%s:%d):ͨ��[%d]����\n",clientAddr.get_host_addr(),clientAddr.get_port_number(),(*ptr_user)));
			client.close();
			return 0;
		}
	}
	
	if (false == SendHttpHeader(client))//���ͻظ�����
	{
		ACE_DEBUG((LM_DEBUG,"(%T | %t)�û�(%s:%d):���ͻظ�����ʧ��\n",clientAddr.get_host_addr(),clientAddr.get_port_number()));
		client.close();
		return 0;
	}
	//�������ӵĿͻ�����Ϣ
	ptr_user = userdeviceilist.begin();
	for (;ptr_user!=userdeviceilist.end();++ptr_user)
	{
		sVedioUserInfo tempuserinfo;
		tempuserinfo.client		= client;
		tempuserinfo.DeviceID	= StrUtil::Int2Str(*ptr_user);
		tempuserinfo.IP			= clientAddr.get_host_addr();
		tempuserinfo.Port		= StrUtil::Int2Str(clientAddr.get_port_number());

		//���ͻ���ӵ���Ӧ�����ݷ��Ͷ���
		TSSENDERMGR::instance()->AddClient(*ptr_user,tempuserinfo);
		ACE_DEBUG((LM_DEBUG,"(%T | %t)�û�����(%s:%d):ͨ��[%d]\n",clientAddr.get_host_addr(),clientAddr.get_port_number(),*ptr_user));
	}

	return 0;
}

int RealTimeVideoServer::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
	//�Ƴ���Ƶ������
	close_mask = ACE_Event_Handler::ALL_EVENTS_MASK | ACE_Event_Handler::DONT_CALL;
	COMMUNICATIONMGR::instance()->AccessMyReactor()->GetReactor()->remove_handler(this,close_mask);
	this->peer_acceptor.close();

	delete this;
	return 0;
}

ACE_HANDLE RealTimeVideoServer::get_handle(void) const
{
	return this->peer_acceptor.get_handle();
}

int RealTimeVideoServer::Stop()
{
	ACE_Reactor_Mask close_mask = ACE_Event_Handler::ALL_EVENTS_MASK | ACE_Event_Handler::DONT_CALL;
	COMMUNICATIONMGR::instance()->AccessMyReactor()->GetReactor()->remove_handler(this,close_mask);
	this->peer_acceptor.close();

	return 0;
}
bool RealTimeVideoServer:: RecvDeviceID(ACE_SOCK_Stream& newclient,list<int>& deviceIDlist)
{
	int countTimes = 10;//���տͻ��˷������ݵĴ���
	char RecvBuf[4096] = {0};
	int RecvCount = -1;
	try
	{
		ACE_Time_Value TimeOut(0,100);
		while(--countTimes)
		{
			ssize_t RecvCount = newclient.recv(RecvBuf,4096,&TimeOut);//��������
		
			if(RecvCount < 4 || RecvCount == SOCKET_ERROR)//socket����
			{
				OSFunction::Sleep(0,10);
				continue;
			}
			
			char szSeps[] = "\n";

			//wz_110125:���cpTokenΪNULLʱ��string dsp(cpToken)���ֵ��쳣
			char *cpToken = strtok(RecvBuf, szSeps);
			if (cpToken == NULL)
			{
				return false;
			}
			string dsp(cpToken);

			dsp = dsp.substr(dsp.find_first_of("/"));
			dsp = dsp.substr(0,dsp.find_first_of(" "));
			std::stringstream stream(dsp);
			string strTemp;
			//�ͻ�ͨ�������ӻ���û������ӵ�ͨ���ţ�http://192.168.100.56:8080/1/2
			while (getline(stream,strTemp,'/'))
			{
				if (strTemp != "")
				{
					int deviceid = StrUtil::Str2Int(strTemp);//�õ����ӵ�ͨ����
					deviceIDlist.push_back(deviceid);
				}
			}
			break;
		}
	}
	catch(...)
	{
		;
	}
	return true;
}
bool RealTimeVideoServer::SendHttpHeader(ACE_SOCK_Stream& newclient)
{
	std::string HttpHeader = "HTTP/1.1 200 OK\r\n";//http��Ӧͷ
	HttpHeader += std::string("Content-Type: video/mpeg\r\n") + std::string("Cache-Control: no-cache\r\nContent-Length: 0-\r\n") + std::string("\r\n");

	ACE_Message_Block *MBHttpHeader = new ACE_Message_Block(HttpHeader.length());
	memcpy(MBHttpHeader->wr_ptr(),HttpHeader.c_str(),HttpHeader.length());
	MBHttpHeader->wr_ptr(HttpHeader.length());

	ACE_Time_Value TimeOut(0,100);
	int nCount = newclient.send_n(MBHttpHeader,&TimeOut);//����http��Ӧͷ����
	if(nCount<=0)
		return false;	
	return true;
}