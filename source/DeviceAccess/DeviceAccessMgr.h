///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����DeviceAccessMgr.h
// �����ߣ�jiangcheng
// ����ʱ�䣺2009-06-09
// ����������Ӳ���豸���ʹ�����
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include "ace/Message_Queue.h"
#include "ace/SOCK_Stream.h"
#include <map>
#include <vector>
#include "../DeviceAccess/CardType.h"

class DeviceAccess;
class DeviceAccessMgr
{
public:
	DeviceAccessMgr();
	virtual ~DeviceAccessMgr();
public:
	//����ͨ���ź��·���XML����õ�Ӳ���豸�ظ���XML��Ϣ
	bool SendTaskMsg(int deviceID, const std::string& strCmdMsg,std::string& strRetMsg);
	//����ͨ���ź�Ts���ļ��ڵ��ַ���õ����ʸ�����socket�����Ϣ
	bool GetTsReceiveHandle(int deviceID,const std::string& strAddress,ACE_SOCK_Stream& streamHandle);
	//����Ts���ļ���ַ���õ����ʸ�����socket�����Ϣ
	bool GetTsReceiveHandle(const std::string& strAddress,ACE_SOCK_Stream& streamHandle);
	//�жϸ�ͨ���Ƿ�����
	bool CheckDeviceIDLock(int deviceID);
	//�жϸ�Ƶ���Ƿ�������
	bool CheckFreqLock(int freq);

	bool Start();
	
	//��ȡָ��ͨ���ŵ�ָ�����
//	bool GetRadioTunerQulity(int deviceID,VSTuner_Quality_Result_Obj& radiotunQ);

	//���ÿ�ʱ��
	bool SetSysTime(int deviceID);	
	//���ð忨�ı���������
	bool setThresholdInfo(int deviceID);
	bool getQualityInfoFor6U(int deviceID, VSTuner_Quality_Result_Obj& retObj);			//��ȡָ��
	bool getAllQualityInfoFor6UInCard(int deviceID, VSTuner_AllQuality_Result_Obj& retObj);			//��ȡȫ��ͨ��ָ��
	bool getChanFixInfoFor6U(int deviceID);																			//��ȡ��ͨ��Ƶ������״̬
	bool getAllChanFixInfoFor6UInCard(int deviceID, VSTuner_AllChanFix_Result_Obj& retObj);							//��ȡһ�Ű忨��ȫ��Ƶ������״̬
	bool LockFreqFor6U(int deviceID, LockFreq_Handle lockFreqArr, VSScan_Result_Obj& retObj);
	bool defaultSpectRum(int deviceid, std::vector<int>& freqVec);
private:
	void Init();
	void UnInit();
private:
	std::map<int,DeviceAccess*> deviceAccessMap;
	ACE_Thread_Mutex getTsMutex;
};

typedef ACE_Singleton<DeviceAccessMgr,ACE_Mutex>  DEVICEACCESSMGR;