///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：QualitySender.h
// 创建者：gaoxd
// 创建时间：2009-09-09
// 内容描述：实时指标数据发送线程
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ace/Task.h"
#include "ace/Singleton.h"
#include "../Foundation/TypeDef.h"
#include <vector>

class QualitySender : public ACE_Task<ACE_MT_SYNCH>
{
public:
	QualitySender(int deviceid);
	QualitySender();
	~QualitySender();
	
public:
	int Start();

	int open(void*);

	virtual int svc();

	int Stop();

	bool AddClient(sVedioUserInfo newclient);//添加用户
	bool HasClient(string msgid);			//判断是否有视频用户 

private:
	bool ClearAllClient();//清空视频用户
	bool ClearInvalidClient(); //清理无效socket
	int ProcessMessage(ACE_Message_Block* mb);
	bool RecvTaskXml2();		
	bool RecvTaskXml();	//添加数据接收，适应安播项目的多个实时指标任务通过同一个Socket执行的业务逻辑
private:
	std::vector<sVedioUserInfo> ClientVec;
	ACE_Thread_Mutex QueueMutex;
	bool bFlag;
	int DeviceID;
};

typedef ACE_Singleton<QualitySender,ACE_Mutex>  REALTIMEQUALITYSENDER;