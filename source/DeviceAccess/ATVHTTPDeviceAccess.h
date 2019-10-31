#ifndef __ATV_HTTP_DEVICE_ACCESS_H__
#define __ATV_HTTP_DEVICE_ACCESS_H__

#include <string>
#include "./HTTPDeviceAccess.h"
using namespace std;

class ATVHTTPDeviceAccess : public HTTPDeviceAccess
{
public:
	ATVHTTPDeviceAccess(int iDeviceID, std::string strIP, int iPort);
	virtual ~ATVHTTPDeviceAccess();

private:
	virtual bool GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetSpectrumRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetTsQueryRetXML(const std::string& strCmdMsg,std::string& strRetMsg);

	bool setCardSystemTime();
	bool setTunerInfo(int chanNo, int freq);
public:
	//bool getSingleQualityInfo();
	//bool getAllQualityInfo();
	bool SetEncoderSwitch(bool enable = false);
	bool SetAlarmThreshold();

	bool getQualityInfoFor6U(VSTuner_Quality_Result_Obj& retObj){return true;};			//��ȡָ��
	bool getAllQualityInfoFor6UInCard(VSTuner_AllQuality_Result_Obj& retObj){return true;};			//��ȡȫ��ͨ��ָ��
	bool getChanFixInfoFor6U();																			//��ȡ��ͨ��Ƶ������״̬
	bool getAllChanFixInfoFor6UInCard(VSTuner_AllChanFix_Result_Obj& retObj);							//��ȡһ�Ű忨��ȫ��Ƶ������״̬
	bool defaultSpectRum(std::vector<int>& freqVec);
	bool Find_str(string str,char** desc,int DescLen);
	void GetATVSpecTrumParmSet(SpectrumParm parmSet,string &MEASureCmd);
	std::string m_ip;
	int m_port;
	ACE_Thread_Mutex SpecTrumReadMutex;
};

#endif