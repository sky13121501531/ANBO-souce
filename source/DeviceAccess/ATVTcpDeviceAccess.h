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
	//频谱时用于设置xml
	static float GetOpenTVStandardFreq(float freq);


	/**	各测量任务总接口
	*/
private:
	virtual bool GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetSpectrumRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetTsQueryRetXML(const std::string& strCmdMsg,std::string& strRetMsg);


	/**	发送任务命令接口
	*/
private:
	virtual int  SendCmdForChannelScan(MSG_TYPE msg_type,void* info,ChannelScanRetMessage_Handle pRetObj, int infolen);
	//virtual int  SendCmdForSpecScan(MSG_TYPE msg_type,void* info, RadioSpecRetMessage_Handle pRetObj,int infolen)	{return -1 ; }
	//virtual int  SendCmdComForQuality(MSG_TYPE msg_type,void* info,RadioQuaRetMessage_Handle pRetObj,int infolen)	{return -1 ; }


	/**	ATV测量指标接口
	*/
private:
	bool GetFreqLevel(float fFreq, float &fLevel);
	bool QualityRealtimeQueryTV(int freq,float & f_level);		//电视实时指标

	float GetImageLevel(float Freq);					//图像载波电平
	float GetAudioLevel(float Freq);					//伴音载波电平
	float GetI2AOffLevel(float Freq);					//图像伴音比
	float GetCN(float Freq);							//载噪比
	float GetFreqOffSet(float Freq);					//频偏
	float GetSlope(float Freq);							//斜率

	//GetCN() 中调用
	static float GetAlignedFreq(float freq);
	bool GetBaseCN(float fFreq, float &fCN);




	/**	调频相关接口
	*/
private:
	virtual bool TurnFreq(TunerConfig_Obj& tunerinfo);
	bool SetVideoRate(const MediaConfig_Obj& rateinfo);			//视频码率
	//bool SetOSD(int chan,const SubtitleConfig_Obj& osdinfo);	//设置OSD
	//bool SetSysTime();											//动态设置系统时间
	bool ChannelscanEx(int startfreq,int endfreq,int scanstep,SpecialRadioRetMessage_Obj& ChanScanRet);

	bool SetOSDFor6U(string dvbtype, string name);
	bool SetEncodeFor6U(string Bps, string width = "", string height = "");
	bool TurnFreqFor6U(int freq, Modulation_TV_e mode = MODULATION_TV_PALDK);
	bool SetTimeOSDFor6U(string dvbtype, int enable);
	bool LockFreqFor6U(LockFreq_Handle lockFreqArr, VSScan_Result_Obj& retObj);
	bool Channelscan(string startFreq, string endFreq, string scanStep, VSScan_Result_Obj& retObj);
	bool setThresholdInfo();			//板卡报警灵敏度设置函数
	bool getQualityInfoFor6U(VSTuner_Quality_Result_Obj& retObj);			//获取指标
	bool getAllQualityInfoFor6UInCard(VSTuner_AllQuality_Result_Obj& retObj){return true;};			//获取全部通道指标
	bool getChanFixInfoFor6U();																			//获取单通道频道锁定状态
	bool getAllChanFixInfoFor6UInCard(VSTuner_AllChanFix_Result_Obj& retObj);							//获取一张板卡的全部频道锁定状态
	bool setCardSystemTimeFor6U();
	bool setScanControlParam();
	bool setVolumeFor6U(int enable);
	bool changeChan(std::string dvbtype, int freq, std::string Bps, std::string name, int timeEnable, int volumeEnabel, int bgEnable);

private:
	float fOldAnalyser[5];

	float mImageLevel;  //图像电平,单位:dbuv
	float mAudioLevel;  //伴音电平,单位:dbuv
	float mI2AOffLevel; //图像载波与伴音载波的电平差,单位:dbuv
	float mCN;          //载噪比,单位:dbuv
	float mFreqOffSet;  //载频频偏,单位:kHz
	float mSlope;       //斜率

	float mAnalyserImageOffset; //指标电平偏移量
	float mAnalyserAudioOffset; //指标伴音偏移量
	float mAnalyserI2AOffset;   //指标图像伴音比偏移量
	float mAnalyserCnOffset;    //指标载噪比偏移量
	float mAnalyserOffOffset;   //指标频偏偏移量
	float mAnalyserSlopeOffset; //指标斜率偏移量

	vector<int> v_mTurnedFreq;  //调频成功过的频率列表

	Encode_Param_t mLastEncoderParam;		//标识上一次设置的编码器参数
	
};
































#endif