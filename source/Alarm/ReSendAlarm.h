#pragma once

#include "ace/Task.h"
#include "../Foundation/TypeDef.h"
#include <string>

typedef struct NODE
{
	char   cFileName[260];//文件夹名
	char   FileName[300];//文件名
	
}node;

class ReSendAlarm : public ACE_Task<ACE_MT_SYNCH>
{
public:
	ReSendAlarm();
	virtual ~ReSendAlarm();
public:
	int open(void*);

	virtual int svc();

	int Stop();
protected:
	int ProcessMessage();//报警xml处理
	int TraverseAlarmFiles();
    int vProResend();
private:
	bool bFlag;
	SYSTEMTIME CurrentTime;

	map<time_t,node*> NodeMap;
	
	int m_ReAlarmFileNum;
};