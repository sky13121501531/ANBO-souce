///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����HttpOpe.cpp
// �����ߣ�jiangcheng
// ����ʱ�䣺2009-06-05
// ������������Ӳ��Http����ͨ����
///////////////////////////////////////////////////////////////////////////////////////////
#include "HttpOpe.h"
#include "ace/Time_Value.h"
#include "ace/Log_Msg.h"
#include "../Foundation/StrUtil.h"
#include "../Communications/SysMsgSender.h"

#include <iostream>

HttpOpe::HttpOpe(const char *hostName, int port) : remote_addr_(port,hostName)
{

}

HttpOpe::~HttpOpe()
{
	
}

bool HttpOpe::PostHttpMessage(const std::string& strSubRoot,const std::string& strCmdMsg,std::string& strRetMsg)
{
	bool bRet = false;

	//modify by gxd 2010-10-12
//	std::string tempxml;
//	StrUtil::ConvertGBKToUtf8(strCmdMsg.c_str(),tempxml);

	ACE_SOCK_Stream client_stream_;
	ACE_SOCK_Connector connector_;

	ACE_Time_Value TimeOut(180);
	ACE_Time_Value NoBlockTimeOut(1, 0);
	
	char ipbuf[50] = {0};

	remote_addr_.get_host_addr(ipbuf,remote_addr_.get_addr_size());

	if (connector_.connect (client_stream_, remote_addr_, &NoBlockTimeOut) == -1)//����Ӳ��������
	{
		string msg = string("����[") + string(ipbuf) + string(":") + StrUtil::Int2Str(remote_addr_.get_port_number()) + std::string("]ʧ��");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);

		client_stream_.close();
		return false;
	}
	else
	{
		string msg = string("����[") + string(ipbuf) + string(":") + StrUtil::Int2Str(remote_addr_.get_port_number()) + std::string("]�ɹ�");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}


	std::string strHeader = PostHttpHeader(strSubRoot,(int)strCmdMsg.length());//�����������http����ͷ
	std::string strSendMsg = strHeader;
	strSendMsg += strCmdMsg;

	//std::string strSendMsg = strCmdMsg;

	int sendlen = client_stream_.send(strSendMsg.c_str(),strSendMsg.length(),&TimeOut);//����������Ϣ

	char bBuf[1024*10] = {0};
	int readLen = 0;
	std::string strTemp = "";
	while ((readLen = client_stream_.recv(bBuf,1024,&TimeOut))>0)//������Ӧ��Ϣ
	{ 
		strTemp.assign(bBuf,readLen);
		strRetMsg += strTemp;
		readLen = 0;
		memset(bBuf,0,sizeof(bBuf));
	}

	client_stream_.close();

	//���ص���Ӧxml���浽�ַ�����
	size_t xmlPos = strRetMsg.find_first_of("<");
	if (xmlPos != std::string::npos)
	{
		strRetMsg = strRetMsg.substr(xmlPos);
		bRet = true;
	}

	if (strRetMsg.length()>0)
	{
		bRet = true;
	}

	return bRet;
}


bool HttpOpe::PostHttpMessageNoBlock(const std::string& strSubRoot,const std::string& strCmdMsg,std::string& strRetMsg)
{
	bool bRet = false;

	ACE_SOCK_Stream client_stream_;
	ACE_SOCK_Connector connector_;
	//modify by gxd 2010-10-12
	//	std::string tempxml;
	//	StrUtil::ConvertGBKToUtf8(strCmdMsg.c_str(),tempxml);

	ACE_Time_Value TimeOut(50);
	ACE_Time_Value NoBlockTimeOut(0, 800);
	char ipbuf[50] = {0};

	remote_addr_.get_host_addr(ipbuf,remote_addr_.get_addr_size());

	if (connector_.connect (client_stream_, remote_addr_,  &NoBlockTimeOut) == -1)//����Ӳ��������
	{
		string msg = string("����[") + string(ipbuf) + string(":") + StrUtil::Int2Str(remote_addr_.get_port_number()) + std::string("]ʧ��");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);

		client_stream_.close();
		return false;
	}
	else
	{
		string msg = string("����[") + string(ipbuf) + string(":") + StrUtil::Int2Str(remote_addr_.get_port_number()) + std::string("]�ɹ�");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}


	std::string strHeader = PostHttpHeader(strSubRoot,(int)strCmdMsg.length());//�����������http����ͷ
	std::string strSendMsg = strHeader;
	strSendMsg += strCmdMsg;

	//std::string strSendMsg = strCmdMsg;
	client_stream_.send(strSendMsg.c_str(),strSendMsg.length(),&TimeOut);//����������Ϣ

	char bBuf[1024] = {0};
	int readLen = 0;
	std::string strTemp = "";
	while ((readLen = client_stream_.recv(bBuf,1024,&TimeOut)) > 0)//������Ӧ��Ϣ
	{ 
		strTemp.assign(bBuf,readLen);
		strRetMsg += strTemp;
		readLen = 0;
		memset(bBuf,0,sizeof(bBuf));
	}

	client_stream_.close();

	//���ص���Ӧxml���浽�ַ�����
	size_t xmlPos = strRetMsg.find_first_of("<");
	if (xmlPos != std::string::npos)
	{
		strRetMsg = strRetMsg.substr(xmlPos);
		bRet = true;
	}

	if (strRetMsg.length()>0)
	{
		bRet = true;
	}

	return bRet;
}

std::string HttpOpe::PostHttpHeader(const std::string& strSubRoot,int len)
{
	//����post http����ͷ
	std::string strHeader = "";
	strHeader += "POST /";
	strHeader += strSubRoot;
	strHeader += " HTTP/1.1\r\n";
	std::string strAddr = remote_addr_.get_host_addr();
	std::string strPort = StrUtil::Int2Str(remote_addr_.get_port_number());
	strHeader += "Host: ";
	strHeader += strAddr;
	strHeader += ":";
	strHeader += strPort;
	strHeader += "\r\n";
	strHeader += "Cache-Control: no-cache\r\n";
	std::string strLen = StrUtil::Int2Str(len);
	strHeader += "Content-Length: ";
	strHeader += strLen;
	strHeader += " \r\n";
	strHeader += "Content-Type:text/xml \r\n";
	strHeader += "\r\n";

	return strHeader;
}

bool HttpOpe::GetRecvHandle(const std::string& strSubRoot,ACE_SOCK_Stream& socketHandle,bool IsRecvXmlHeader)
{
	bool bRet = false;

	ACE_SOCK_Stream client_stream_;
	ACE_SOCK_Connector connector_;
	char ipbuf[50] = {0};
	ACE_Time_Value TimeOut(5);

	remote_addr_.get_host_addr(ipbuf,remote_addr_.get_addr_size());

	if (connector_.connect (client_stream_, remote_addr_) == -1)//���ӷ�������ַ
	{
		string msg = string("����[") + string(ipbuf) + string(":") + StrUtil::Int2Str(remote_addr_.get_port_number()) + std::string("]ʧ��");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
		return false;
	}
	
	string msg = string("����[") + string(ipbuf) + string(":") + StrUtil::Int2Str(remote_addr_.get_port_number()) + std::string("]�ɹ�");
	SYSMSGSENDER::instance()->SendMsg(msg);

	std::string strHeader = GetHttpHeader(strSubRoot);
	if (client_stream_.send(strHeader.c_str(),strHeader.length(),&TimeOut) > 0)//����httpͷ
		bRet = true;

	socketHandle = client_stream_;//��������socket
	if (IsRecvXmlHeader)
	{
		char headbuf[500] = {0};
		socketHandle.recv(headbuf,500,&TimeOut);//���շ�������Ӧ
	}
	return bRet;
}

std::string HttpOpe::GetHttpHeader(const std::string& strSubRoot)
{
	//����get http����ͷ

	std::string strHeader = "";
	if (strSubRoot.find("http://")!=std::string::npos)
	{
		strHeader += "GET ";
	}
	else
	{
		strHeader += "GET /";
	}

	//20120406�ܾ�sms���Ը���
	strHeader += strSubRoot;
	strHeader += " HTTP/1.1\r\n";
	std::string strAddr = remote_addr_.get_host_addr();
	std::string strPort = StrUtil::Int2Str(remote_addr_.get_port_number());
	strHeader += "Host: ";
	strHeader += strAddr;
	strHeader += ":";
	strHeader += strPort;
	strHeader += " \r\n";
	strHeader += "Cache-Control: no-cache\r\n";
	strHeader += "Content-Type:text/xml \r\n";
	strHeader += "\r\n";

	return strHeader;
}