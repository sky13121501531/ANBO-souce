///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����StreamRealtimeRoundQueryTask.cpp
// �����ߣ�gaoxd
// ����ʱ�䣺2009-06-24
// ����������ʵʱ��Ƶ�ֲ�������
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "DeviceRelatedTask.h"
#include <string>
#include <vector>
#include "ace/Task.h"
using namespace std;
class StreamRealtimeRoundQueryTask : public DeviceRelatedTask
{
public:
	StreamRealtimeRoundQueryTask();
	StreamRealtimeRoundQueryTask(std::string strXML);
	virtual ~StreamRealtimeRoundQueryTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
	virtual bool IsRealTimeTask(){return true;};	//ʵʱ����
	virtual bool IsVedioTask(){return true;};		//����Ƶ����

	bool SetRelatedTask(StreamRealtimeRoundQueryTask* task){RelatedTask = task;return true;};
	bool SetStopSignal(){StopSignal = true;return true;};		//�����������

private:
	bool bOneDevice;
	bool bSendToIdleDev;					//bOneDevice:һ��ͨ���ж�,bSendToIdleDev:�Ƿ������ͨ�����͹���Ƶ����
	int idleDeviceID,busyDeviceID;			//idleDeviceID:����ͨ����busyDeviceID:ִ������ͨ����
	long RoundTime;							//�ֲ����
	std::vector<std::string> DecivcXMLVec;	//Ӳ��������
	size_t ChannelNum;						//�ֲ���Ƶ����
	size_t ChannelIndex;					//������

	bool StopSignal;							//ֹͣ�ź�
	StreamRealtimeRoundQueryTask* RelatedTask;	//��������,���ڴ�����������ֹͣ
};