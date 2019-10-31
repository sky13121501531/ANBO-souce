///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����DeviceAccess.h
// �����ߣ�gaoxd
// ����ʱ�䣺2010-07-06
// ����������Ӳ���豸���ʻ���
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "ace/Synch.h"
#include "ace/SOCK_Stream.h"
#include "../Foundation/TypeDef.h"
#include <string>
#include "CardType.h"
#include <vector>



class DeviceAccess
{
public:
	DeviceAccess(int deviceid, std::string strIP,int nPort);
	virtual ~DeviceAccess(void);
protected:
	DeviceAccess(void);
public:
	virtual bool SendTaskMsg(const std::string& strCmdMsg,std::string& strRetMsg) = 0;
	virtual bool GetTsReceiveHandle(const std::string& strAddress,ACE_SOCK_Stream& streamHandle) = 0;	
	virtual bool CheckDeviceIDLock() = 0;					//�жϸ�ͨ���Ƿ�����
	
//	virtual bool GetRadioTunerQulity(VSTuner_Quality_Result_Obj& radiotunQ) = 0; //��ȡ�㲥�ŵ�ָ�����

	virtual bool TurnFreq(TunerConfig_Obj& tunerinfo) { return true ; }
	virtual bool TurnFreqFor6U(int freq, Modulation_TV_e mode = MODULATION_TV_PALDK) { return true ; }
	virtual bool SetSysTime(int chan) = 0;
	//���ð忨�ı���������
	virtual bool setThresholdInfo() = 0;
	virtual bool LockFreqFor6U(LockFreq_Handle lockFreqArr, VSScan_Result_Obj& retObj) = 0;
	virtual bool getQualityInfoFor6U(VSTuner_Quality_Result_Obj& retObj) = 0;										//��ȡ��ͨ��ָ��
	virtual bool getAllQualityInfoFor6UInCard(VSTuner_AllQuality_Result_Obj& retObj) = 0;							//��ȡһ�Ű忨��ȫ��ͨ��ָ��
	virtual bool getChanFixInfoFor6U() = 0;																			//��ȡ��ͨ��Ƶ������״̬
	virtual bool getAllChanFixInfoFor6UInCard(VSTuner_AllChanFix_Result_Obj& retObj) = 0;							//��ȡһ�Ű忨��ȫ��Ƶ������״̬
	virtual bool defaultSpectRum(std::vector<int>& freqVec) =0;
	bool GetFreqInfo(TunerConfig_Obj& tunerinfo)
	{
		if(tcTunerFreq.chan == -1)
		{
			return false;
		}
		tunerinfo.chan =tcTunerFreq.chan ;
		tunerinfo.freq =tcTunerFreq.freq ;
		return true;
	}
	time_t tLastSpectrum;
	int LastStartFreq ;
	int LastEndFreq;
	int LastScanStep;
	
protected:
	ACE_Thread_Mutex sendTaskMsgMutex;
	int DeviceId;
	TunerConfig_Obj tcTunerFreq;
	std::string strIPAddress;
	int port;
	std::vector<SpectrumInfo> mLastSpectrum;
};

