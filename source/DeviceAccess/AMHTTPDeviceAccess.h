#ifndef __AM_HTTP_DEVICE_ACCESS_H__
#define __AM_HTTP_DEVICE_ACCESS_H__

#include <string>

#include "./HTTPDeviceAccess.h"
using namespace std;

class AMHTTPDeviceAccess : public HTTPDeviceAccess
{
public:
	AMHTTPDeviceAccess(int iDeviceID, std::string strIP, int iPort);
	virtual ~AMHTTPDeviceAccess();

private:
	virtual bool GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetSpectrumRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetTsQueryRetXML(const std::string& strCmdMsg,std::string& strRetMsg);

	bool setCardSystemTime();
	bool SetAlarmThreshold();
	bool setTunerInfo(int chanNo, int freq);
public:
	//bool getSingleQualityInfo();
	//bool getAllQualityInfo();
	bool SetEncoderSwitch(bool enable = false);


	bool getQualityInfoFor6U(VSTuner_Quality_Result_Obj& retObj);			//获取指标
	bool getAllQualityInfoFor6UInCard(VSTuner_AllQuality_Result_Obj& retObj);			//获取全部通道指标
	bool getChanFixInfoFor6U(){return false;};																			//获取单通道频道锁定状态
	bool getAllChanFixInfoFor6UInCard(VSTuner_AllChanFix_Result_Obj& retObj){return false;};						//获取一张板卡的全部频道锁定状态
	bool defaultSpectRum(std::vector<int>& freqVec){return true;};
};

#endif