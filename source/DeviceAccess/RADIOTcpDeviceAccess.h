#ifndef RADIO_TCP_ACCESS_H
#define	RADIO_TCP_ACCESS_H
#include "./TCPDeviceAccess.h"
#include <string>
using namespace std;

class RadioTcpDeviceAccess : public TCPDeviceAccess
{
public:
	RadioTcpDeviceAccess(int deviceid,std::string strIP,int nPort);
	virtual ~RadioTcpDeviceAccess()	{ };


	bool ChannelscanEx(int startfreq,int endfreq,int scanstep,SpecialRadioRetMessage_Obj& ChanScanRet);
	/**	�����������ܽӿ�
	*/
private:
	virtual bool GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetSpectrumRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetTsQueryRetXML(const std::string& strCmdMsg,std::string& strRetMsg);


	/**	��������ָ��ӿ�
	*/
private:
	//virtual int  SendCmdForChannelScan(MSG_TYPE msg_type,void* info,ChannelScanRetMessage_Handle pRetObj, int infolen)	{return -1 ; }	//����Ƶ��ɨ��
	virtual int  SendCmdForSpecScan(MSG_TYPE msg_type,void* info, RadioSpecRetMessage_Handle pRetObj,int infolen);
	virtual int  SendCmdForQuality(MSG_TYPE msg_type,void* info,RadioQuaRetMessage_Handle pRetObj,int infolen);
	//Ƶ��ɨ�����⴦��ӿ�
	int  SendCmdForRadioSpecialChannelScan(MSG_TYPE msg_type,void* info,SpecialRadioRetMessage_Handle pRetObj, int infolen);


	/**	�㲥Ƶ��ɨ���Ƶ��ɨ����ؽӿ�
	*/
private:
	bool SpecScan(int freq,RadioSpecRetMessage_Obj& RadioSpec);
	void CaculateCenterFreq(int startfreq,int endfreq,int *centerfreqs,int &len);		//��������Ƶ��
	int  Findpolars(char spec[1023],int fFreq,int* result);                             //��ȡƵ��ɨ�����㷨
	//Ƶ��ɨ��
	bool GetRadioSpecResult(float fFreq, RadioSpecRetMessage_Obj &rqr);


	/**	�㲥ָ�������ؽӿ�
	*/
public:
	virtual bool GetQuality(float fFreq, RadioQuaRetMessage_Obj &rqr);				//(�ᱻ�ⲿ���ã�����Ϊpublic)

//6U�豸�ӿ�
	bool SetOSDFor6U(string dvbtype, string name);
	bool SetEncodeFor6U(string Bps, string width = "", string height = "");
	bool TurnFreqFor6U(int freq, Modulation_Radio_e mode = MODULATION_RADIO_FM);
	bool SetTimeOSDFor6U(string dvbtype, int enable);
	bool Channelscan(string startFreq, string endFreq, string scanStep, VSScan_Result_Obj& retObj);
	bool setThresholdInfo();			//�忨�������������ú���
	bool LockFreqFor6U(LockFreq_Handle lockFreqArr, VSScan_Result_Obj& retObj){return true;};
	bool getQualityInfoFor6U(VSTuner_Quality_Result_Obj& retObj);			//��ȡ��ͨ��ָ��
	bool getAllQualityInfoFor6UInCard(VSTuner_AllQuality_Result_Obj& retObj);			//��ȡȫ��ͨ��ָ��
	bool getChanFixInfoFor6U(){return true;};																			//��ȡ��ͨ��Ƶ������״̬
	bool getAllChanFixInfoFor6UInCard(VSTuner_AllChanFix_Result_Obj& retObj){return true;};							//��ȡһ�Ű忨��ȫ��Ƶ������״̬

	bool setCardSystemTimeFor6U();
	bool setScanControlParam();
	bool setVolumeFor6U(int enable);
	bool changeChan(std::string dvbtype, int freq, std::string Bps, std::string name, int timeEnable, int volumeEnabel, int bgEnable);
private:
	//bool QueryQuality(int freq,RadioQuaRetMessage_Obj& RadioQua);		//�㲥ʵʱָ��
	bool GetRadioFreqLevel(float fFreq, float &fLevel);							//(��ʱû��)
	vector<string> m_vecChannelID;   //���ݿ�Ƶ����
	//vector<SpectrumInfo> m_LastSpectrumVec; //��һ�ε�Ƶ������
	
	/**	��Ƶ��ؽӿ�
	*/
private:
	virtual bool TurnFreq(TunerConfig_Obj& tunerinfo);			//��Ƶ
	bool SetAudioRate(const MediaConfig_Obj& rateinfo);			//��Ƶ����
	Encode_Param_t mLastEncoderParam;		//��ʶ��һ�����õı���������
};




































#endif