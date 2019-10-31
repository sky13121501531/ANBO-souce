#include "HTTPDeviceAccess.h"
#include "HttpOpe.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/TypeDef.h"
#include "../Foundation/PropManager.h"

HTTPDeviceAccess::HTTPDeviceAccess(void)
{
}

HTTPDeviceAccess::~HTTPDeviceAccess(void)
{
}
HTTPDeviceAccess::HTTPDeviceAccess(int deviceid,const std::string& strIP,int nPort):DeviceAccess(deviceid,strIP,nPort)
{

}
bool HTTPDeviceAccess::SendTaskMsg(const std::string& strCmdMsg,std::string& strRetMsg)
{
	//添加具体实现
	ACE_Guard<ACE_Thread_Mutex> guard(sendTaskMsgMutex);
	XmlParser parser;
	parser.Set_xml(strCmdMsg);
	pXMLNODE root=parser.GetRootNode();
	string dvbtype;
	parser.GetAttrNode(root,"DVBType",dvbtype);
	pXMLNODE childnode=parser.GetNodeFirstChild(root);
	string nodename=parser.GetNodeName(childnode);
	bool Ret=true;
	if ("QualityQuery"==nodename)
	{
		Ret=GetQualityRetXML(strCmdMsg,strRetMsg);
	}
	else if("SpectrumQuery"==nodename)
	{
		Ret=GetSpectrumRetXML(strCmdMsg,strRetMsg);//
	}
	else if ("ChannelScanQuery"==nodename)
	{
		Ret=GetChannelScanRetXML(strCmdMsg,strRetMsg);
	}
	else if ("TSQuery"==nodename)
	{
		Ret=GetTsQueryRetXML(strCmdMsg,strRetMsg);
	}
	return Ret;
}

bool HTTPDeviceAccess::SendXmlTaskToDevice(const std::string& strCmdMsg,std::string& strRetMsg)
{
	bool bRet = false;

	if (strCmdMsg.empty())
	{
		ACE_DEBUG((LM_DEBUG,"(%T | %t)""下发任务指令为空!\n"));
		return bRet;
	}
	//SendDeviceMutex.acquire();
	HttpOpe httpOpe(strIPAddress.c_str(),port);//http
	std::string strSubRoot = "perform";
	ACE_Guard<ACE_Thread_Mutex> guard(sendTaskMsgMutex);
	if ((httpOpe.PostHttpMessage(strSubRoot,strCmdMsg,strRetMsg)) && (!strRetMsg.empty()))//post形式发送http请求
		bRet = true;
	//SendDeviceMutex.release();
	return bRet;
}

bool HTTPDeviceAccess::SendXmlTaskToDeviceNoBlock(const std::string& strCmdMsg,std::string& strRetMsg)
{
	bool bRet = false;

	if (strCmdMsg.empty())
	{
		ACE_DEBUG((LM_DEBUG,"(%T | %t)""下发任务指令为空!\n"));
		return bRet;
	}
	HttpOpe httpOpe(strIPAddress.c_str(),port);//http
	std::string strSubRoot = "perform";

	ACE_Guard<ACE_Thread_Mutex> guard(sendTaskMsgMutex);

	if ((httpOpe.PostHttpMessageNoBlock(strSubRoot,strCmdMsg,strRetMsg)) && (!strRetMsg.empty()))//post形式发送http请求
		bRet = true;

	return bRet;
}

bool HTTPDeviceAccess::GetTsReceiveHandle(const std::string& strAddress,ACE_SOCK_Stream& streamHandle)
{
	bool bRet = false;

	if (strAddress.empty())
	{
		ACE_DEBUG((LM_DEBUG,"(%T | %t)""读取Ts流地址为空\n"));
		return bRet;
	}

	HttpOpe httpOpe(strIPAddress.c_str(),port);
	ACE_Guard<ACE_Thread_Mutex> guard(sendTaskMsgMutex);

	if (httpOpe.GetRecvHandle(strAddress,streamHandle))//获得连接socket
		bRet = true;

	return bRet;
}