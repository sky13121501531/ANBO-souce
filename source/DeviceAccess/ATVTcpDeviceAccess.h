#ifndef ATV_TCP_ACCESS_H
#define	ATV_TCP_ACCESS_H

#include "./TCPDeviceAccess.h"
#include <string>
using namespace std;

class AtvTcpDeviceAccess : public TCPDeviceAccess
{
public:
	AtvTcpDeviceAccess(int deviceid,std::string strIP,int nPort);
	virtual ~AtvTcpDeviceAccess()	{ };

public:
	//Ƶ��ʱ��������xml
	static float GetOpenTVStandardFreq(float freq);


	/**	�����������ܽӿ�
	*/
private:
	virtual bool GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetSpectrumRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetTsQueryRetXML(const std::string& strCmdMsg,std::string& strRetMsg);


	/**	������������ӿ�
	*/
private:
	virtual int  SendCmdForChannelScan(MSG_TYPE msg_type,void* info,ChannelScanRetMessage_Handle pRetObj, int infolen);
	//virtual int  SendCmdForSpecScan(MSG_TYPE msg_type,void* info, RadioSpecRetMessage_Handle pRetObj,int infolen)	{return -1 ; }
	//virtual int  SendCmdComForQuality(MSG_TYPE msg_type,void* info,RadioQuaRetMessage_Handle pRetObj,int infolen)	{return -1 ; }


	/**	ATV����ָ��ӿ�
	*/
private:
	bool GetFreqLevel(float fFreq, float &fLevel);
	bool QualityRealtimeQueryTV(int freq,float & f_level);		//����ʵʱָ��

	float GetImageLevel(float Freq);					//ͼ���ز���ƽ
	float GetAudioLevel(float Freq);					//�����ز���ƽ
	float GetI2AOffLevel(float Freq);					//ͼ�������
	float GetCN(float Freq);							//�����
	float GetFreqOffSet(float Freq);					//Ƶƫ
	float GetSlope(float Freq);							//б��

	//GetCN() �е���
	static float GetAlignedFreq(float freq);
	bool GetBaseCN(float fFreq, float &fCN);




	/**	��Ƶ��ؽӿ�
	*/
private:
	virtual bool TurnFreq(TunerConfig_Obj& tunerinfo);
	bool SetVideoRate(const MediaConfig_Obj& rateinfo);			//��Ƶ����
	//bool SetOSD(int chan,const SubtitleConfig_Obj& osdinfo);	//����OSD
	//bool SetSysTime();											//��̬����ϵͳʱ��
	bool ChannelscanEx(int startfreq,int endfreq,int scanstep,SpecialRadioRetMessage_Obj& ChanScanRet);

	bool SetOSDFor6U(string dvbtype, string name);
	bool SetEncodeFor6U(string Bps, string width = "", string height = "");
	bool TurnFreqFor6U(int freq, Modulation_TV_e mode = MODULATION_TV_PALDK);
	bool SetTimeOSDFor6U(string dvbtype, int enable);
	bool LockFreqFor6U(LockFreq_Handle lockFreqArr, VSScan_Result_Obj& retObj);
	bool Channelscan(string startFreq, string endFreq, string scanStep, VSScan_Result_Obj& retObj);
	bool setThresholdInfo();			//�忨�������������ú���
	bool getQualityInfoFor6U(VSTuner_Quality_Result_Obj& retObj);			//��ȡָ��
	bool getAllQualityInfoFor6UInCard(VSTuner_AllQuality_Result_Obj& retObj){return true;};			//��ȡȫ��ͨ��ָ��
	bool getChanFixInfoFor6U();																			//��ȡ��ͨ��Ƶ������״̬
	bool getAllChanFixInfoFor6UInCard(VSTuner_AllChanFix_Result_Obj& retObj);							//��ȡһ�Ű忨��ȫ��Ƶ������״̬
	bool setCardSystemTimeFor6U();
	bool setScanControlParam();
	bool setVolumeFor6U(int enable);
	bool changeChan(std::string dvbtype, int freq, std::string Bps, std::string name, int timeEnable, int volumeEnabel, int bgEnable);

private:
	float fOldAnalyser[5];

	float mImageLevel;  //ͼ���ƽ,��λ:dbuv
	float mAudioLevel;  //������ƽ,��λ:dbuv
	float mI2AOffLevel; //ͼ���ز�������ز��ĵ�ƽ��,��λ:dbuv
	float mCN;          //�����,��λ:dbuv
	float mFreqOffSet;  //��ƵƵƫ,��λ:kHz
	float mSlope;       //б��

	float mAnalyserImageOffset; //ָ���ƽƫ����
	float mAnalyserAudioOffset; //ָ�����ƫ����
	float mAnalyserI2AOffset;   //ָ��ͼ�������ƫ����
	float mAnalyserCnOffset;    //ָ�������ƫ����
	float mAnalyserOffOffset;   //ָ��Ƶƫƫ����
	float mAnalyserSlopeOffset; //ָ��б��ƫ����

	vector<int> v_mTurnedFreq;  //��Ƶ�ɹ�����Ƶ���б�

	Encode_Param_t mLastEncoderParam;		//��ʶ��һ�����õı���������
	
};
































#endif