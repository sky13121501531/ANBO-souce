///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����UDPUDPTsFetcher.h
// �����ߣ�gaoxd
// ����ʱ�䣺2010-07-07
// ������������Ƶ���ݻ�ȡ�߳� ����UDPЭ��
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "TsFetcher.h"
#include "ace/Task.h"
//#include "ace/SOCK_Connector.h"
#include <ace/SOCK_Dgram_Mcast.h>
#include <ace/INET_Addr.h>
#include "../Foundation/FileOperater.h"

#include <vector>
#include <string>

class TsSender;

class UDPTsFetcher : public TsFetcher
{
public:
	UDPTsFetcher(int deviceid);
	~UDPTsFetcher();
private:
	UDPTsFetcher();
public:
	int Start();

	int open(void*);

	virtual int svc();

	int Stop();

	bool SetTsDeviceXml(std::string devicexml);

	bool SendTsDeviceXml();
	bool RebootCard();

	void IncreaseTaskNum();
	void DecreaseTaskNum();
	void StopHistoryTask();

	bool SetTsSendTask(TsSender* task);
	bool SetTsRoundTask(TsSender* task);
	void SetSendSwitch(bool sendswitch);		//�����Ƿ�������
	bool SetRecordTask(ACE_Task<ACE_MT_SYNCH>* task);
	bool DelRecordTask(ACE_Task<ACE_MT_SYNCH>* task);
	void SetReSendSwitch(bool sendswitch);
	
private:
	bool FlushHistoryDatum();
	void TimeConvert(time_t curtime,unsigned char *timebuf);//��ϵͳʱ����Ϣ���뵽¼��
	void PutSysTime(ACE_Task<ACE_MT_SYNCH>* task);
private:
	
	std::string DeviceTaskXml;			//Ӳ����ʼ��ָ��
	std::string CurDeviceXml;			//��ǰTSӲ������Ϣ

	ACE_Thread_Mutex TaskPointerMutex;
	ACE_Thread_Mutex TaskMutex;
	ACE_Thread_Mutex TaskNumMutex;
	static ACE_Thread_Mutex TaskRoundMutex;

	TsSender* TsSendTaskPointer;
	TsSender* TsSendRoundTaskPointer;
	std::vector<ACE_Task<ACE_MT_SYNCH>*> RecordTaskPointerVec;
	
	ACE_INET_Addr local_addr;
	ACE_INET_Addr remote_addr;
	ACE_SOCK_Dgram_Mcast DeviceSock;
	//ACE_SOCK_Dgram DeviceSock;

	bool bFlag;
	bool bSendSwtich;
	bool bSendOrder;
	int TaskNum;
	bool ReSendSwitch;
	bool NewPackageHead;		//����Ƶ����ͷ����

	std::string mUdpFileName;
	FileOperater mUdpFile;
	time_t mUdpCurTime; 

};