
///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：HTTPDeviceAccess.h
// 创建者：gaoxd
// 创建时间：2010-07-06
// 内容描述：硬件设备访问类，基于HTTP协议
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
	bool getQualityInfoFor6U(VSTuner_Quality_Result_Obj& retObj){return true;};			//获取指标
	bool LockFreqFor6U(LockFreq_Handle lockFreqArr, VSScan_Result_Obj& retObj){return true;};
	bool getAllQualityInfoFor6UInCard(VSTuner_AllQuality_Result_Obj& retObj){return true;};			//获取全部通道指标
	bool getChanFixInfoFor6U(){return true;};																			//获取单通道频道锁定状态
	bool getAllChanFixInfoFor6UInCard(VSTuner_AllChanFix_Result_Obj& retObj){return true;};							//获取一张板卡的全部频道锁定状态

	bool SendXmlTaskToDevice(const std::string& strCmdMsg,std::string& strRetMsg);
	bool SendXmlTaskToDeviceNoBlock(const std::string& strCmdMsg,std::string& strRetMsg);
	bool SetEncoderSwitch(bool enable = false){return true;};
	bool defaultSpectRum(std::vector<int>& freqVec){return true;};
};
