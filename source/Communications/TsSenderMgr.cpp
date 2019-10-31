
#include "TsSenderMgr.h"
#include "../Foundation/PropManager.h"
#include <list>
#include <iostream>

using namespace std;

TsSenderMgr::TsSenderMgr()
{
}

TsSenderMgr::~TsSenderMgr()
{
}
int TsSenderMgr::Start()
{
	std::list<int> DeviceList;
	PROPMANAGER::instance()->GetAllDeviceList(DeviceList);	
	//��ʼ��TS���ݷ����߳�
	for (std::list<int>::iterator ptr = DeviceList.begin();ptr!=DeviceList.end();++ptr)
	{
		TsSender* tsSender = new TsSender(*ptr);
		if (tsSender != NULL)
		{
			TsSenderMap.insert(make_pair(*ptr,tsSender));
			tsSender->Start();
		}
	}
	//�����ֲ�ͨ��
	//wz_0217
	std::list<int> virdevlist; //����ͨ���б�
	PROPMANAGER::instance()->GetVirDevList(virdevlist);
	if (virdevlist.empty())
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)��ȡ���ֲ�����ͨ��Ϊ�� !\n"));
	}

	std::list<int>::iterator iter = virdevlist.begin();
	for(;iter != virdevlist.end();++iter)//ÿһ������ͨ��������һ��tssender
	{
		int deviceId = *iter;
		TsSender* tsSender = new TsSender(deviceId);
		if (tsSender != NULL)
		{
			TsSenderMap.insert(make_pair(deviceId,tsSender));
			tsSender->Start();
		}
	}
	//wz_0217

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)��Ƶ���ݷ����̳߳�����[%d]���߳� !\n",(int)TsSenderMap.size()));
	return 0;
}

int TsSenderMgr::Stop()
{	
	//ֹͣ���е�ȡ�����߳�
	std::map<int,TsSender*>::iterator ptr = TsSenderMap.begin();
	for (;ptr!=TsSenderMap.end();++ptr)
	{
		if (ptr->second != NULL)
		{
			ptr->second->Stop();
			delete ptr->second;
			ptr->second = NULL;
		}
	}
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)��Ƶ���ݷ����̳߳�ֹͣ[%d]���߳� !\n",(int)TsSenderMap.size()));
	TsSenderMap.clear();
	return 0;
}

void TsSenderMgr::SetSendSwitch(int deviceid,bool sendswitch)
{
	map<int,TsSender*>::iterator ptr= TsSenderMap.begin();
	for (;ptr!=TsSenderMap.end();++ptr)
	{
		if (ptr->first == deviceid && ptr->second != NULL)
		{
			(ptr->second)->SetSendSwitch(sendswitch);
			break;
		}
	}
}

bool TsSenderMgr::AddClient(int deviceid,sVedioUserInfo newclient)
{
	map<int,TsSender*>::iterator ptr= TsSenderMap.begin();
	for (;ptr!=TsSenderMap.end();++ptr)
	{
		if (ptr->first == deviceid && ptr->second != NULL)
		{
			(ptr->second)->AddClient(newclient);
			break;
		}
	}
	return true;
}

bool TsSenderMgr::HasClient(int deviceid)
{
	map<int,TsSender*>::iterator ptr= TsSenderMap.begin();
	for (;ptr!=TsSenderMap.end();++ptr)
	{
		if (ptr->first == deviceid && ptr->second != NULL)
		{
			return (ptr->second)->HasClient();
		}
	}
	return false;
}

bool TsSenderMgr::GetAllClient(std::vector<sVedioUserInfo>& uservec)
{
	map<int,TsSender*>::iterator ptr= TsSenderMap.begin();
	for (;ptr!=TsSenderMap.end();++ptr)
	{
		if (ptr->second != NULL)
		{
			(ptr->second)->GetAllClient(uservec);
		}
	}
	return true;
}

bool TsSenderMgr::StopAllClient(int deviceid)
{
	map<int,TsSender*>::iterator ptr= TsSenderMap.begin();
	for (;ptr!=TsSenderMap.end();++ptr)
	{
		if (ptr->first == deviceid && ptr->second != NULL)
		{
			(ptr->second)->StopAllClient();
		}
	}
	return true;
}