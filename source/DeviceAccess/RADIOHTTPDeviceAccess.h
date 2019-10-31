#ifndef __RADIO_HTTP_DEVICE_ACCESS_H__
#define __RADIO_HTTP_DEVICE_ACCESS_H__

#include <string>

#include "./HTTPDeviceAccess.h"
using namespace std;

class RADIOHTTPDeviceAccess : public HTTPDeviceAccess
{
public:
	RADIOHTTPDeviceAccess(int iDeviceID, std::string strIP, int iPort);
	virtual ~RADIOHTTPDeviceAccess();

private:
	virtual bool GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetSpectrumRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetTsQueryRetXML(const std::string& strCmdMsg,std::string& strRetMsg);

	bool setCardSystemTime();
	bool SetAlarmThreshold();
	bool setTunerInfo(int chanNo, int freq);
	bool Spectrumscan(int startfreq, int endfreq, int scanstep, vector<SpectrumInfoEx>  &vecSpectrumValue);
	std::string getipforRADIO(){return m_RADIOip;};
	int getportforRADIO(){return m_RADIOport;};
public:
	//bool getSingleQualityInfo();
	//bool getAllQualityInfo();
	bool SetEncoderSwitch(bool enable = false);

	bool getQualityInfoFor6U(VSTuner_Quality_Result_Obj& retObj);			//获取指标
	bool getAllQualityInfoFor6UInCard(VSTuner_AllQuality_Result_Obj& retObj);			//获取全部通道指标
	bool getChanFixInfoFor6U(){return true;};																			//获取单通道频道锁定状态
	bool getAllChanFixInfoFor6UInCard(VSTuner_AllChanFix_Result_Obj& retObj){return true;};							//获取一张板卡的全部频道锁定状态
	bool ChannelscanEx(int startfreq,int endfreq,int scanstep,SpecialRadioRetMessage_Obj& ChanScanRet);
	bool defaultSpectRum(std::vector<int>& freqVec){return true;};
    bool Find_str(string str,char** desc,int DescLen);
	void GetSpecRetStr(std::string strMore,std::string &strTemp);
private:
	std::string m_RADIOip;
	int m_RADIOport;
};

#endif