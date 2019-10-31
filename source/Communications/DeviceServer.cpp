#include "DeviceServer.h"
#include <string>
#include <sstream>
#include <iostream>
#include "../Foundation/OSFunction.h"
#include "../Foundation/TypeDef.h"
#include "../Foundation/PropManager.h"
#include "CommunicationMgr.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include <ace/OS.h>
#include "../Foundation/AppLog.h"

using namespace std;
DeviceServer::DeviceServer(void)
{
	bFlag=false;
}

DeviceServer::~DeviceServer(void)
{
}

int DeviceServer::Open( ACE_Addr &addr )
{
	if (peer_acceptor.open(addr) == -1)//�����������˿�
	{
		return -1;
	}

	return 0;
}

int DeviceServer::handle_input( ACE_HANDLE handle )
{
	ACE_SOCK_Stream client;											//����socket
	ACE_INET_Addr clientAddr;	//�ͻ��˵�ַ��Ϣ
	ACE_Time_Value AcceptTimeOut(5);
	
	if (this->peer_acceptor.accept(client,&clientAddr,&AcceptTimeOut) == -1)		//���ܿͻ�������
	{
		ACE_DEBUG((LM_ERROR,"(%T | %t)Error in connection\n"));
	}
	else
	{
		BUSINESSLAYOUTMGR::instance()->AddDeviceSocket(client);
	}
	return 0;
}

ACE_HANDLE DeviceServer::get_handle( void ) const
{
	return peer_acceptor.get_handle();
}

int DeviceServer::handle_close( ACE_HANDLE handle, ACE_Reactor_Mask close_mask )
{
	close_mask = ACE_Event_Handler::ALL_EVENTS_MASK | ACE_Event_Handler::DONT_CALL;
	COMMUNICATIONMGR::instance()->AccessMyReactor()->GetReactor()->remove_handler(this,close_mask);
	this->peer_acceptor.close();

	delete this;
	return 0;
}

int DeviceServer::Stop()
{
	ACE_Reactor_Mask close_mask = ACE_Event_Handler::ALL_EVENTS_MASK | ACE_Event_Handler::DONT_CALL;
	COMMUNICATIONMGR::instance()->AccessMyReactor()->GetReactor()->remove_handler(this,close_mask);
	this->peer_acceptor.close();

	return 0;
}
