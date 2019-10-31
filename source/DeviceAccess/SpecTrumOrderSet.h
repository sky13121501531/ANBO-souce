///////////////////////////////////////////////////////////////////////////////////////////
// ÎÄ¼þÃû£ºUdpAlarmRecvThreadMgr.h
//////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ace/Task.h"
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include <map>


class SpecTrumOrderSet : public ACE_Task<ACE_MT_SYNCH>
{
public:
	SpecTrumOrderSet();
	virtual ~SpecTrumOrderSet();
public:
	int Start();
	int open(void*);

	virtual int svc();
	int Stop();
private:
	bool bFlag;
	ACE_Thread_Mutex SpecTrumCardMutex;
};
typedef  ACE_Singleton<SpecTrumOrderSet,ACE_Mutex>  SPECTRUMORDERSET;
