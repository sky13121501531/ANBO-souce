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
	/**	各测量任务总接口
	*/
private:
	virtual bool GetQualityRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetSpectrumRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetChannelScanRetXML(const std::string& strCmdMsg,std::string& strRetMsg);
	virtual bool GetTsQueryRetXML(const std::string& strCmdMsg,std::string& strRetMsg);


	/**	发送任务指令接口
	*/
private:
	//virtual int  SendCmdForChannelScan(MSG_TYPE msg_type,void* info,ChannelScanRetMessage_Handle pRetObj, int infolen)	{return -1 ; }	//电视频道扫描
	virtual int  SendCmdForSpecScan(MSG_TYPE msg_type,void* info, RadioSpecRetMessage_Handle pRetObj,int infolen);
	virtual int  SendCmdForQuality(MSG_TYPE msg_type,void* info,RadioQuaRetMessage_Handle pRetObj,int infolen);
	//频道扫描特殊处理接口
	int  SendCmdForRadioSpecialChannelScan(MSG_TYPE msg_type,void* info,SpecialRadioRetMessage_Handle pRetObj, int infolen);


	/**	广播频道扫描和频谱扫描相关接口
	*/
private:
	bool SpecScan(int freq,RadioSpecRetMessage_Obj& RadioSpec);
	void CaculateCenterFreq(int startfreq,int endfreq,int *centerfreqs,int &len);		//计算中心频点
	int  Findpolars(char spec[1023],int fFreq,int* result);                             //获取频道扫描结果算法
	//频谱扫描
	bool GetRadioSpecResult(float fFreq, RadioSpecRetMessage_Obj &rqr);


	/**	广播指标测量相关接口
	*/
public:
	virtual bool GetQuality(float fFreq, RadioQuaRetMessage_Obj &rqr);				//(会被外部调用，定义为public)

//6U设备接口
	bool SetOSDFor6U(string dvbtype, string name);
	bool SetEncodeFor6U(string Bps, string width = "", string height = "");
	bool TurnFreqFor6U(int freq, Modulation_Radio_e mode = MODULATION_RADIO_FM);
	bool SetTimeOSDFor6U(string dvbtype, int enable);
	bool Channelscan(string startFreq, string endFreq, string scanStep, VSScan_Result_Obj& retObj);
	bool setThresholdInfo();			//板卡报警灵敏度设置函数
	bool LockFreqFor6U(LockFreq_Handle lockFreqArr, VSScan_Result_Obj& retObj){return true;};
	bool getQualityInfoFor6U(VSTuner_Quality_Result_Obj& retObj);			//获取单通道指标
	bool getAllQualityInfoFor6UInCard(VSTuner_AllQuality_Result_Obj& retObj);			//获取全部通道指标
	bool getChanFixInfoFor6U(){return true;};																			//获取单通道频道锁定状态
	bool getAllChanFixInfoFor6UInCard(VSTuner_AllChanFix_Result_Obj& retObj){return true;};							//获取一张板卡的全部频道锁定状态

	bool setCardSystemTimeFor6U();
	bool setScanControlParam();
	bool setVolumeFor6U(int enable);
	bool changeChan(std::string dvbtype, int freq, std::string Bps, std::string name, int timeEnable, int volumeEnabel, int bgEnable);
private:
	//bool QueryQuality(int freq,RadioQuaRetMessage_Obj& RadioQua);		//广播实时指标
	bool GetRadioFreqLevel(float fFreq, float &fLevel);							//(暂时没用)
	vector<string> m_vecChannelID;   //数据库频道表
	//vector<SpectrumInfo> m_LastSpectrumVec; //上一次的频谱数据
	
	/**	调频相关接口
	*/
private:
	virtual bool TurnFreq(TunerConfig_Obj& tunerinfo);			//调频
	bool SetAudioRate(const MediaConfig_Obj& rateinfo);			//音频码率
	Encode_Param_t mLastEncoderParam;		//标识上一次设置的编码器参数
};




































#endif