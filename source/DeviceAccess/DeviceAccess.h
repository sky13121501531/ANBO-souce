///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：DeviceAccess.h
// 创建者：gaoxd
// 创建时间：2010-07-06
// 内容描述：硬件设备访问基类
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
	virtual bool CheckDeviceIDLock() = 0;					//判断该通道是否锁定
	
//	virtual bool GetRadioTunerQulity(VSTuner_Quality_Result_Obj& radiotunQ) = 0; //获取广播信道指标参数

	virtual bool TurnFreq(TunerConfig_Obj& tunerinfo) { return true ; }
	virtual bool TurnFreqFor6U(int freq, Modulation_TV_e mode = MODULATION_TV_PALDK) { return true ; }
	virtual bool SetSysTime(int chan) = 0;
	//设置板卡的报警灵敏度
	virtual bool setThresholdInfo() = 0;
	virtual bool LockFreqFor6U(LockFreq_Handle lockFreqArr, VSScan_Result_Obj& retObj) = 0;
	virtual bool getQualityInfoFor6U(VSTuner_Quality_Result_Obj& retObj) = 0;										//获取单通道指标
	virtual bool getAllQualityInfoFor6UInCard(VSTuner_AllQuality_Result_Obj& retObj) = 0;							//获取一张板卡的全部通道指标
	virtual bool getChanFixInfoFor6U() = 0;																			//获取单通道频道锁定状态
	virtual bool getAllChanFixInfoFor6UInCard(VSTuner_AllChanFix_Result_Obj& retObj) = 0;							//获取一张板卡的全部频道锁定状态
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

