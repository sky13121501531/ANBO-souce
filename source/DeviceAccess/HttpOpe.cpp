///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：HttpOpe.cpp
// 创建者：jiangcheng
// 创建时间：2009-06-05
// 内容描述：与硬件Http服务通信类
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

	if (connector_.connect (client_stream_, remote_addr_, &NoBlockTimeOut) == -1)//连接硬件服务器
	{
		string msg = string("连接[") + string(ipbuf) + string(":") + StrUtil::Int2Str(remote_addr_.get_port_number()) + std::string("]失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);

		client_stream_.close();
		return false;
	}
	else
	{
		string msg = string("连接[") + string(ipbuf) + string(":") + StrUtil::Int2Str(remote_addr_.get_port_number()) + std::string("]成功");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}


	std::string strHeader = PostHttpHeader(strSubRoot,(int)strCmdMsg.length());//向服务器发送http请求头
	std::string strSendMsg = strHeader;
	strSendMsg += strCmdMsg;

	//std::string strSendMsg = strCmdMsg;

	int sendlen = client_stream_.send(strSendMsg.c_str(),strSendMsg.length(),&TimeOut);//发送命令消息

	char bBuf[1024*10] = {0};
	int readLen = 0;
	std::string strTemp = "";
	while ((readLen = client_stream_.recv(bBuf,1024,&TimeOut))>0)//接收响应消息
	{ 
		strTemp.assign(bBuf,readLen);
		strRetMsg += strTemp;
		readLen = 0;
		memset(bBuf,0,sizeof(bBuf));
	}

	client_stream_.close();

	//返回的响应xml保存到字符串中
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

	if (connector_.connect (client_stream_, remote_addr_,  &NoBlockTimeOut) == -1)//连接硬件服务器
	{
		string msg = string("连接[") + string(ipbuf) + string(":") + StrUtil::Int2Str(remote_addr_.get_port_number()) + std::string("]失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);

		client_stream_.close();
		return false;
	}
	else
	{
		string msg = string("连接[") + string(ipbuf) + string(":") + StrUtil::Int2Str(remote_addr_.get_port_number()) + std::string("]成功");
		SYSMSGSENDER::instance()->SendMsg(msg);
	}


	std::string strHeader = PostHttpHeader(strSubRoot,(int)strCmdMsg.length());//向服务器发送http请求头
	std::string strSendMsg = strHeader;
	strSendMsg += strCmdMsg;

	//std::string strSendMsg = strCmdMsg;
	client_stream_.send(strSendMsg.c_str(),strSendMsg.length(),&TimeOut);//发送命令消息

	char bBuf[1024] = {0};
	int readLen = 0;
	std::string strTemp = "";
	while ((readLen = client_stream_.recv(bBuf,1024,&TimeOut)) > 0)//接收响应消息
	{ 
		strTemp.assign(bBuf,readLen);
		strRetMsg += strTemp;
		readLen = 0;
		memset(bBuf,0,sizeof(bBuf));
	}

	client_stream_.close();

	//返回的响应xml保存到字符串中
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
	//构造post http请求头
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

	if (connector_.connect (client_stream_, remote_addr_) == -1)//连接服务器地址
	{
		string msg = string("连接[") + string(ipbuf) + string(":") + StrUtil::Int2Str(remote_addr_.get_port_number()) + std::string("]失败");
		SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
		return false;
	}
	
	string msg = string("连接[") + string(ipbuf) + string(":") + StrUtil::Int2Str(remote_addr_.get_port_number()) + std::string("]成功");
	SYSMSGSENDER::instance()->SendMsg(msg);

	std::string strHeader = GetHttpHeader(strSubRoot);
	if (client_stream_.send(strHeader.c_str(),strHeader.length(),&TimeOut) > 0)//发送http头
		bRet = true;

	socketHandle = client_stream_;//保存连接socket
	if (IsRecvXmlHeader)
	{
		char headbuf[500] = {0};
		socketHandle.recv(headbuf,500,&TimeOut);//接收服务器响应
	}
	return bRet;
}

std::string HttpOpe::GetHttpHeader(const std::string& strSubRoot)
{
	//构造get http请求头

	std::string strHeader = "";
	if (strSubRoot.find("http://")!=std::string::npos)
	{
		strHeader += "GET ";
	}
	else
	{
		strHeader += "GET /";
	}

	//20120406总局sms调试更改
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