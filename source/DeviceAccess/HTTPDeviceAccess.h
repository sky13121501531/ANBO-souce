
///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����HTTPDeviceAccess.h
// �����ߣ�gaoxd
// ����ʱ�䣺2010-07-06
// ����������Ӳ���豸�����࣬����HTTPЭ��
///////////////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "DeviceAccess.h"

class HTTPDeviceAccess : public DeviceAccess
{
public:
	HTTPDeviceAccess(int deviceid,const std::string& strIP,int nPort);
	~HTTPDeviceAccess(void);
private:
	HTTPDeviceAccess(void);

	
	virtual bool GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg) { return true ; }
	virtual bool GetSpectrumRetXML(const std::string& strCmdMsg,std::string& strRetMsg){ return true ; }
	virtual bool GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg){ return true ; }
	virtual bool GetTsQueryRetXML(const std::string& strCmdMsg,std::string& strRetMsg){ return true ; }
	
public:
	bool SendTaskMsg(const std::string& strCmdMsg,std::string& strRetMsg);
	bool GetTsReceiveHandle(const std::string& strAddress,ACE_SOCK_Stream& streamHandle);
	bool CheckDeviceIDLock(){return true;};
	bool GetRadioTunerQulity(VSTuner_Quality_Result_Obj &tss){return true;};
	bool SetSysTime(int chan){return true;};
	bool setThresholdInfo(){return true;};
	bool getQualityInfoFor6U(VSTuner_Quality_Result_Obj& retObj){return true;};			//��ȡָ��
	bool LockFreqFor6U(LockFreq_Handle lockFreqArr, VSScan_Result_Obj& retObj){return true;};
	bool getAllQualityInfoFor6UInCard(VSTuner_AllQuality_Result_Obj& retObj){return true;};			//��ȡȫ��ͨ��ָ��
	bool getChanFixInfoFor6U(){return true;};																			//��ȡ��ͨ��Ƶ������״̬
	bool getAllChanFixInfoFor6UInCard(VSTuner_AllChanFix_Result_Obj& retObj){return true;};							//��ȡһ�Ű忨��ȫ��Ƶ������״̬

	bool SendXmlTaskToDevice(const std::string& strCmdMsg,std::string& strRetMsg);
	bool SendXmlTaskToDeviceNoBlock(const std::string& strCmdMsg,std::string& strRetMsg);
	bool SetEncoderSwitch(bool enable = false){return true;};
	bool defaultSpectRum(std::vector<int>& freqVec){return true;};
};
