
#include "Reactor.h"
#include "XMLReceiverServer.h"
#include "RealTimeVideoServer.h"
#include "RealTimeQualityServer.h"
#include "RecordQualityServer.h"
#include "DeviceServer.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/PropManager.h"
#include "ace/Synch.h"

MyReactor::MyReactor(void)
{
	pXMLReceiverServer = NULL;
	pRealTimeVideoServer = NULL;
	pRealTimeQualityServer = NULL;
	pDeviceServer = NULL;
	pRecordQualityServer=NULL;
}

MyReactor::~MyReactor(void)
{

}

int MyReactor::Init()
{
	if (NULL == pXMLReceiverServer)
		pXMLReceiverServer = new XMLReceiverServer();

	if(NULL == pRealTimeVideoServer)
		pRealTimeVideoServer = new RealTimeVideoServer();

	if (NULL == pRealTimeQualityServer)
		pRealTimeQualityServer = new RealTimeQualityServer();

	if (NULL == pDeviceServer)
		pDeviceServer = new DeviceServer();

	if(NULL == pRecordQualityServer)
		pRecordQualityServer=new RecordQualityServer();

	//XML�������IP��Port
	std::string XMLServerIP = PROPMANAGER::instance()->GetXmlServerIP();
	unsigned int  XMLServerPort = StrUtil::Str2Int(PROPMANAGER::instance()->GetXmlServerPort());
	ACE_INET_Addr  XMLServerAddr(XMLServerPort,XMLServerIP.c_str());

	////��Ƶ����IP,Port
	std::string videoServerIP = PROPMANAGER::instance()->GetHttpVideoIp();
	unsigned int videoServerPort = StrUtil::Str2Int(PROPMANAGER::instance()->GetHttpVideoPort());
	ACE_INET_Addr videoServerAddr(videoServerPort,videoServerIP.c_str());
	ACE_DEBUG((LM_DEBUG,"(%T | %t)��Ƶ����:%s:%d!\n",videoServerIP.c_str(),videoServerPort));

	//ʵʱָ�����IP,Port
	std::string QualityServerIP = PROPMANAGER::instance()->GetQualityIp();
	unsigned int QualityServerPort = StrUtil::Str2Int(PROPMANAGER::instance()->GetQualityPort());
	ACE_INET_Addr QualityServerAddr(QualityServerPort,QualityServerIP.c_str());

	//¼��ָ�����IP,Port
	std::string RecordQualityServerIP = PROPMANAGER::instance()->GetRecordQualityIp();
	unsigned int RecordQualityServerPort = StrUtil::Str2Int(PROPMANAGER::instance()->GetRecordQualityPort());
	ACE_INET_Addr RecordQualityServerAddr(RecordQualityServerPort,RecordQualityServerIP.c_str());

	//�忨��Ϣ����IP,Port
	std::string DeviceServerIP = PROPMANAGER::instance()->GetDeviceIp();
	unsigned int DeviceServerPort = StrUtil::Str2Int(PROPMANAGER::instance()->GetDevicePort());
	ACE_INET_Addr DeviceServerAddr(DeviceServerPort,DeviceServerIP.c_str());

	//��ʼ��
	if (pXMLReceiverServer->Open(XMLServerAddr) == -1 ||
		pRealTimeVideoServer->Open(videoServerAddr) == -1 ||
		pRealTimeQualityServer->Open(QualityServerAddr) == -1 ||
		pDeviceServer->Open(DeviceServerAddr) == -1 ||
		pRecordQualityServer->Open(RecordQualityServerAddr)== -1)
	{
		ACE_DEBUG((LM_DEBUG,"(%T |  %t)��Ƶ/ָ��/�忨����˵�ַ��ʼ������!\n"));
		return -1;
	}

	//��Ӧ��ע���¼�
	if (ACE_Reactor::instance()->register_handler(pXMLReceiverServer,ACE_Event_Handler::ACCEPT_MASK) == -1 ||
		ACE_Reactor::instance()->register_handler(pRealTimeVideoServer,ACE_Event_Handler::ACCEPT_MASK) == -1 ||
		ACE_Reactor::instance()->register_handler(pRealTimeQualityServer,ACE_Event_Handler::ACCEPT_MASK) == -1 ||
		ACE_Reactor::instance()->register_handler(pDeviceServer,ACE_Event_Handler::ACCEPT_MASK) == -1 ||
		ACE_Reactor::instance()->register_handler(pRecordQualityServer,ACE_Event_Handler::ACCEPT_MASK) == -1)
	{
		ACE_DEBUG((LM_DEBUG,"(%T | %t)��Ƶ/ָ��/�忨������¼�ע��ʧ��!\n"));
		return -1;
	}
	return 0;
}

int MyReactor::open(void*)
{	
	//������Ӧ��
	ACE_DEBUG((LM_DEBUG,"(%T | %t)��Ӧ����ʼ����(��Ƶ/ָ��/�忨����)!\n"));
	this->activate();

	return 0;
}

int MyReactor::svc()
{
	ACE_Reactor::instance()->run_reactor_event_loop();
	return 0;
}

int MyReactor::Stop()
{
	ACE_Reactor::instance()->end_event_loop();
	
	pXMLReceiverServer->Stop();
	pRealTimeVideoServer->Stop();
	pRealTimeQualityServer->Stop();
	pDeviceServer->Stop();
	pRecordQualityServer->Stop();

	ACE_DEBUG((LM_DEBUG,"(%T | %t)��Ӧ��ֹͣ��(��Ƶ/ָ��/�忨����)!\n"));

	return 0;
}

ACE_Reactor* MyReactor::GetReactor()
{
	return ACE_Reactor::instance();
}