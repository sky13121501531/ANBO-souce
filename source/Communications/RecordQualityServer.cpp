
#include "RecordQualityServer.h"
#include "CommunicationMgr.h"
#include "QualitySender.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TypeDef.h"
#include "../DBAccess/DBManager.h"
#include "../Foundation/XmlParser.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include "../Alarm/CheckLevelForUnLock.h"
#include "./SysMsgSender.h"
#include "RecordQualitySender.h"
#include "ace/OS.h"
#include <string>
#include <sstream>
#include <iostream>


using namespace std;

RecordQualityServer::RecordQualityServer()
{
	bFlag = false;
}

RecordQualityServer::~RecordQualityServer()
{

}

int RecordQualityServer::Open(ACE_Addr &addr)
{
	if (peer_acceptor.open(addr) == -1)//监听端口
		return -1;

	return 0;
}

int RecordQualityServer::handle_input(ACE_HANDLE handle)
{
	ACE_SOCK_Stream client;
	ACE_INET_Addr clientAddr;//保存客户端连接地址信息

	if (this->peer_acceptor.accept(client,&clientAddr) == -1)//接收客户端连接
	{
		ACE_DEBUG((LM_ERROR,"(%T | %t)Error in connection\n"));
	}
	else
	{
		const int PKGSIZE = 1024;
		int value =PKGSIZE*2;	
		//设置socket接收发送缓冲大小
		client.set_option(SOL_SOCKET,SO_SNDBUF,(char*)&value,sizeof(value));
		client.set_option(SOL_SOCKET,SO_RCVBUF,(char*)&value,sizeof(value));

		string TaskXml = "";
		Sleep(50);

		if(false == RecvTaskXml(client,TaskXml))//tcp方式接受指标任务
		{
			string msg = string("录像指标用户服务器接受用户发送的命令错误");
			SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);

			return 0;
		}
		SYSMSGSENDER::instance()->SendMsg(TaskXml);

		XmlParser parser(TaskXml.c_str());
		pXMLNODE Node= parser.GetNodeFirstChild(parser.GetRootNode());

		string Freq;
		parser.GetAttrNode(Node,"Freq",Freq);
		CHECKLEVELFORUNLOCK::instance()->AddQualityFreq(Freq);
		//保存客户端连接信息
		sVedioUserInfo tempuserinfo;
		tempuserinfo.client = client;
		tempuserinfo.Freq = Freq;
		tempuserinfo.DeviceID = StrUtil::Int2Str(0);
		tempuserinfo.IP = clientAddr.get_host_addr();
		tempuserinfo.Port = StrUtil::Int2Str(clientAddr.get_port_number());

		RECORDQUALITYSENDER::instance()->AddClient(tempuserinfo);//添见指标连接用户
		string msg = string("用户(") + string(clientAddr.get_host_addr()) + string(":") \
			+ StrUtil::Int2Str(clientAddr.get_port_number()) + string(")连接录像指标服务器");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}

	return 0;
}

int RecordQualityServer::handle_close(ACE_HANDLE handle, ACE_Reactor_Mask close_mask)
{
	close_mask = ACE_Event_Handler::ALL_EVENTS_MASK | ACE_Event_Handler::DONT_CALL;
	//从反应器中移除指标服务器
	COMMUNICATIONMGR::instance()->AccessMyReactor()->GetReactor()->remove_handler(this,close_mask);
	this->peer_acceptor.close();

	delete this;
	return 0;
}

ACE_HANDLE RecordQualityServer::get_handle(void) const
{
	return this->peer_acceptor.get_handle();
}

int RecordQualityServer::Stop()
{
	ACE_Reactor_Mask close_mask = ACE_Event_Handler::ALL_EVENTS_MASK | ACE_Event_Handler::DONT_CALL;
	COMMUNICATIONMGR::instance()->AccessMyReactor()->GetReactor()->remove_handler(this,close_mask);
	this->peer_acceptor.close();

	return 0;
}

bool RecordQualityServer::RecvTaskXml(ACE_SOCK_Stream& newclient,std::string& taskxml)
{
	int countTimes = 2;//接收客户端发送数据的次数??? 100
	char RecvBuf[1024] = {0};
	int RecvCount = -1;
	try
	{
		while(--countTimes)
		{
			ACE_Time_Value TimeOut(2);
			ssize_t RecvCount = newclient.recv(RecvBuf,1024,&TimeOut);

			if(RecvCount < 4 || RecvCount == SOCKET_ERROR)
			{
				OSFunction::Sleep(0,10);
				continue;
			}
			char* p = RecvBuf;
			//找到xml字符串的开始之处
			for (int i=0;i<RecvCount;++i)
			{
				if (*p == '<')
				{//只判断一个'<'是否足够？
					char* q = p+1;
					if (*q == '?')
						break;
					string msg = string("&&&&&&&&&&& 开头是22 < 但不是指标任务");
					SYSMSGSENDER::instance()->SendMsg(msg);
				}
				p++;
			}

			taskxml = p;
			return true;
		}
	}
	catch(...)
	{
		;
	}
	return false;
}
