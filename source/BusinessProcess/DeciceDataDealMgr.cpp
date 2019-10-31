#include "DeciceDataDealMgr.h"
#include "DeciceDataDeal.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/OSFunction.h"
#include "ace/Synch.h"
#include "ace/OS.h"
#include "../Alarm/AlarmMgr.h"
#include <fstream>
#include <string>
extern time_t g_freqlocktime[64];
extern bool g_freqlockval[64];
extern bool g_freqneedlock[64];
extern bool g_unalarmchanscanval[64];
extern bool g_alarmchanscanval[64];

DeciceDataDealMgr::DeciceDataDealMgr()
{
	memset(g_freqlocktime,0,sizeof(time_t)*64);
	memset(g_freqlockval,true,sizeof(bool)*64);
	memset(g_freqneedlock,false,sizeof(bool)*64);
	memset(g_unalarmchanscanval,false,sizeof(bool)*64);
	memset(g_alarmchanscanval,false,sizeof(bool)*64);
	bFlag = false;
}

DeciceDataDealMgr::~DeciceDataDealMgr()
{

}

int DeciceDataDealMgr::open(void*)
{
	msg_queue()->high_water_mark(1024*10240);
	msg_queue()->low_water_mark(1024*10240);
	activate();
	return 0;
}

int DeciceDataDealMgr::svc()
{
	bFlag = true;
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)表数据接收线程开始执行 !\n"));
	while (bFlag)
	{		
		OSFunction::Sleep(0,10);

		ACE_Guard<ACE_Thread_Mutex> guard(TaskExecuteMutex);

		std::vector<DeciceDataDeal*>::iterator ptr = mDeciceDataDealVec.begin();
		for (;ptr!=mDeciceDataDealVec.end();)
		{
			if ((*ptr)!=NULL && (*ptr)->TaskFinished())
			{
				(*ptr)->Stop();
				delete (*ptr);
				(*ptr) = NULL;
				ptr = mDeciceDataDealVec.erase(ptr);
			}
			else
			{
				++ptr;
			}
		}
	}
	return 0;
}

bool DeciceDataDealMgr::AddDeviceSocket(ACE_SOCK_Stream socket)
{
	ACE_Guard<ACE_Thread_Mutex> guard(TaskExecuteMutex);
	DeciceDataDeal* pDeciceDataDeal = new DeciceDataDeal(socket);
	if (pDeciceDataDeal != NULL)
	{
		pDeciceDataDeal->open(0);
		mDeciceDataDealVec.push_back(pDeciceDataDeal);
	}
	if(mDeciceDataDealVec.size()>50)
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)mDeciceDataDealVec.size:%d\n",mDeciceDataDealVec.size()));
	}
	return true;
}
int DeciceDataDealMgr::Stop()
{
	bFlag = false;

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)表数据接收线程停止执行 !\n"));

	this->wait();
	return 0;
}