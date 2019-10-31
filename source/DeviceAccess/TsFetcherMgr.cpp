
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
	//初始化TS数据获得线程
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
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)视频数据获取线程池启动[%d]个线程 !\n",(int)TsFetcherMap.size()));
	return 0;
}

int TsFetcherMgr::Stop()
{	
	//停止所有的取数据线程
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
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)视频数据获取线程池停止[%d]个线程 !\n",(int)TsFetcherMap.size()));
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
			ptr->second->SetTsDeviceXml(devicexml);//设置任务XML
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
			ptr->second->SendTsDeviceXml();//重发任务XML
		}
	}
	return true;
}

bool TsFetcherMgr::SetTsSendTask(int deviceid,TsSender* task)
{
	//设置特定通道号的线程的视频任务指针
	map<int,TsFetcher*>::iterator ptr= TsFetcherMap.begin();
	for (;ptr!=TsFetcherMap.end();++ptr)
	{
		if (ptr->first == deviceid && ptr->second != NULL)
		{
			ptr->second->SetTsSendTask(task);//设置视频任务指针
			break;
		}
	}
	return true;
}

bool TsFetcherMgr::SetTsRoundTask(int deviceid, TsSender* task)
{
	//设置特定通道号的线程的录播视频任务指针
	//wz_0217
	map<int,TsFetcher*>::iterator ptr= TsFetcherMap.begin();
	for (;ptr!=TsFetcherMap.end();++ptr)
	{
		if (deviceid == ptr->first && ptr->second != NULL)
		{
			ptr->second->SetTsRoundTask(task);//设置视频录播任务指针
		}
	}
	//wz_0217

	return true;
}
bool TsFetcherMgr::SetRecordTask(int deviceid,ACE_Task<ACE_MT_SYNCH>* task)
{
	//设置特定通道号的线程的录像任务指针
	map<int,TsFetcher*>::iterator ptr= TsFetcherMap.begin();
	for (;ptr!=TsFetcherMap.end();++ptr)
	{
		if (ptr->first == deviceid && ptr->second != NULL)
		{
			ptr->second->SetRecordTask(task);//设置录像任务指针
			break;
		}
	}
	return true;
}
bool TsFetcherMgr::DelRecordTask(int deviceid,ACE_Task<ACE_MT_SYNCH>* task)
{
	//删除特定通道号的线程的录像任务指针
	map<int,TsFetcher*>::iterator ptr= TsFetcherMap.begin();
	for (;ptr!=TsFetcherMap.end();++ptr)
	{
		if (ptr->first == deviceid && ptr->second != NULL)
		{
			ptr->second->DelRecordTask(task);//设置录像任务指针
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