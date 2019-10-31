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
	//��Ӿ���ʵ��
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
		ACE_DEBUG((LM_DEBUG,"(%T | %t)""�·�����ָ��Ϊ��!\n"));
		return bRet;
	}
	//SendDeviceMutex.acquire();
	HttpOpe httpOpe(strIPAddress.c_str(),port);//http
	std::string strSubRoot = "perform";
	ACE_Guard<ACE_Thread_Mutex> guard(sendTaskMsgMutex);
	if ((httpOpe.PostHttpMessage(strSubRoot,strCmdMsg,strRetMsg)) && (!strRetMsg.empty()))//post��ʽ����http����
		bRet = true;
	//SendDeviceMutex.release();
	return bRet;
}

bool HTTPDeviceAccess::SendXmlTaskToDeviceNoBlock(const std::string& strCmdMsg,std::string& strRetMsg)
{
	bool bRet = false;

	if (strCmdMsg.empty())
	{
		ACE_DEBUG((LM_DEBUG,"(%T | %t)""�·�����ָ��Ϊ��!\n"));
		return bRet;
	}
	HttpOpe httpOpe(strIPAddress.c_str(),port);//http
	std::string strSubRoot = "perform";

	ACE_Guard<ACE_Thread_Mutex> guard(sendTaskMsgMutex);

	if ((httpOpe.PostHttpMessageNoBlock(strSubRoot,strCmdMsg,strRetMsg)) && (!strRetMsg.empty()))//post��ʽ����http����
		bRet = true;

	return bRet;
}

bool HTTPDeviceAccess::GetTsReceiveHandle(const std::string& strAddress,ACE_SOCK_Stream& streamHandle)
{
	bool bRet = false;

	if (strAddress.empty())
	{
		ACE_DEBUG((LM_DEBUG,"(%T | %t)""��ȡTs����ַΪ��\n"));
		return bRet;
	}

	HttpOpe httpOpe(strIPAddress.c_str(),port);
	ACE_Guard<ACE_Thread_Mutex> guard(sendTaskMsgMutex);

	if (httpOpe.GetRecvHandle(strAddress,streamHandle))//�������socket
		bRet = true;

	return bRet;
}