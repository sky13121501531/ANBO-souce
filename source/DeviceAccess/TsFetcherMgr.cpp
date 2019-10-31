
#include "TsFetcherMgr.h"
#include "TCPTsFetcher.h"
#include "UDPTsFetcher.h"
#include "../Foundation/PropManager.h"
#include <list>

TsFetcherMgr::TsFetcherMgr()
{
}

TsFetcherMgr::~TsFetcherMgr()
{
}
int TsFetcherMgr::Start()
{
	std::list<int> DeviceList;
	PROPMANAGER::instance()->GetAllDeviceList(DeviceList);	
	//��ʼ��TS���ݻ���߳�
	for (std::list<int>::iterator ptr = DeviceList.begin();ptr!=DeviceList.end();++ptr)
	{
		std::string tsprotocol = "";
		PROPMANAGER::instance()->GetDeviceTsProtocol(*ptr,tsprotocol);
		
		TsFetcher* tsFetcher = NULL;
		if (tsprotocol == "tcp")
		{
			tsFetcher = new TCPTsFetcher(*ptr);
		}
		else if (tsprotocol == "udp")
		{
			tsFetcher = new UDPTsFetcher(*ptr);
		}

		
		if (tsFetcher != NULL)
		{
			TsFetcherMap.insert(make_pair(*ptr,tsFetcher));
			tsFetcher->Start();
		}
	}
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)��Ƶ���ݻ�ȡ�̳߳�����[%d]���߳� !\n",(int)TsFetcherMap.size()));
	return 0;
}

int TsFetcherMgr::Stop()
{	
	//ֹͣ���е�ȡ�����߳�
	std::map<int,TsFetcher*>::iterator ptr = TsFetcherMap.begin();
	for (;ptr!=TsFetcherMap.end();++ptr)
	{
		if (ptr->second != NULL)
		{
			ptr->second->Stop();
			delete ptr->second;
			ptr->second = NULL;
		}
	}
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)��Ƶ���ݻ�ȡ�̳߳�ֹͣ[%d]���߳� !\n",(int)TsFetcherMap.size()));
	TsFetcherMap.clear();
	return 0;
}

bool TsFetcherMgr::SetTsDeviceXml(int deviceid,std::string devicexml)
{
	map<int,TsFetcher*>::iterator ptr= TsFetcherMap.begin();
	for (;ptr!=TsFetcherMap.end();++ptr)
	{
		if (ptr->first == deviceid && ptr->second != NULL)
		{
			ptr->second->SetTsDeviceXml(devicexml);//��������XML
			break;
		}
	}
	return true;
}
bool TsFetcherMgr::ReSendAllDeviceXml(int deviceid)
{
	map<int,TsFetcher*>::iterator ptr= TsFetcherMap.begin();
	for (;ptr!=TsFetcherMap.end();++ptr)
	{
		if (ptr->first == deviceid && ptr->second != NULL)
		{
			ptr->second->SendTsDeviceXml();//�ط�����XML
		}
	}
	return true;
}

bool TsFetcherMgr::SetTsSendTask(int deviceid,TsSender* task)
{
	//�����ض�ͨ���ŵ��̵߳���Ƶ����ָ��
	map<int,TsFetcher*>::iterator ptr= TsFetcherMap.begin();
	for (;ptr!=TsFetcherMap.end();++ptr)
	{
		if (ptr->first == deviceid && ptr->second != NULL)
		{
			ptr->second->SetTsSendTask(task);//������Ƶ����ָ��
			break;
		}
	}
	return true;
}

bool TsFetcherMgr::SetTsRoundTask(int deviceid, TsSender* task)
{
	//�����ض�ͨ���ŵ��̵߳�¼����Ƶ����ָ��
	//wz_0217
	map<int,TsFetcher*>::iterator ptr= TsFetcherMap.begin();
	for (;ptr!=TsFetcherMap.end();++ptr)
	{
		if (deviceid == ptr->first && ptr->second != NULL)
		{
			ptr->second->SetTsRoundTask(task);//������Ƶ¼������ָ��
		}
	}
	//wz_0217

	return true;
}
bool TsFetcherMgr::SetRecordTask(int deviceid,ACE_Task<ACE_MT_SYNCH>* task)
{
	//�����ض�ͨ���ŵ��̵߳�¼������ָ��
	map<int,TsFetcher*>::iterator ptr= TsFetcherMap.begin();
	for (;ptr!=TsFetcherMap.end();++ptr)
	{
		if (ptr->first == deviceid && ptr->second != NULL)
		{
			ptr->second->SetRecordTask(task);//����¼������ָ��
			break;
		}
	}
	return true;
}
bool TsFetcherMgr::DelRecordTask(int deviceid,ACE_Task<ACE_MT_SYNCH>* task)
{
	//ɾ���ض�ͨ���ŵ��̵߳�¼������ָ��
	map<int,TsFetcher*>::iterator ptr= TsFetcherMap.begin();
	for (;ptr!=TsFetcherMap.end();++ptr)
	{
		if (ptr->first == deviceid && ptr->second != NULL)
		{
			ptr->second->DelRecordTask(task);//����¼������ָ��
			break;
		}
	}
	return true;
}

void TsFetcherMgr::SetSendSwitch( int deviceid,bool sendswitch )
{
	map<int,TsFetcher*>::iterator ptr= TsFetcherMap.find(deviceid);
	if(ptr!=TsFetcherMap.end())
	{
		ptr->second->SetSendSwitch(sendswitch);
	}
}

void TsFetcherMgr::SetReSendSwitch(int deviceid,bool sendswitch)
{
	map<int,TsFetcher*>::iterator ptr= TsFetcherMap.find(deviceid);
	if(ptr!=TsFetcherMap.end())
	{
		ptr->second->SetReSendSwitch(sendswitch);
	}
}

void TsFetcherMgr::IncreaseTaskNum( int deviceid )
{
	map<int,TsFetcher*>::iterator ptr= TsFetcherMap.find(deviceid);
	if(ptr!=TsFetcherMap.end())
	{
		ptr->second->IncreaseTaskNum();
	}
}

void TsFetcherMgr::DecreaseTaskNum( int deviceid )
{
	map<int,TsFetcher*>::iterator ptr= TsFetcherMap.find(deviceid);
	if(ptr!=TsFetcherMap.end())
	{
		ptr->second->DecreaseTaskNum();
	}
}

bool TsFetcherMgr::RebootCard(int deviceid)
{
	map<int,TsFetcher*>::iterator ptr= TsFetcherMap.find(deviceid);
	if(ptr!=TsFetcherMap.end())
	{
		ptr->second->RebootCard();
	}
	return true;
}
void TsFetcherMgr::StopHistoryTask(int deviceid)
{
	{
		map<int,TsFetcher*>::iterator ptr= TsFetcherMap.find(deviceid);
		if(ptr!=TsFetcherMap.end())
		{
			ptr->second->StopHistoryTask();
		}
		return;
	}
}