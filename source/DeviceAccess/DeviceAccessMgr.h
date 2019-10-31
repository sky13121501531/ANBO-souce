///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：DeviceAccessMgr.h
// 创建者：jiangcheng
// 创建时间：2009-06-09
// 内容描述：硬件设备访问管理类
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "ace/Synch.h"
#include "ace/Singleton.h"
#include "ace/Message_Queue.h"
#include "ace/SOCK_Stream.h"
#include <map>
#include <vector>
#include "../DeviceAccess/CardType.h"

class DeviceAccess;
class DeviceAccessMgr
{
public:
	DeviceAccessMgr();
	virtual ~DeviceAccessMgr();
public:
	//输入通道号和下发的XML命令，得到硬件设备回复的XML信息
	bool SendTaskMsg(int deviceID, const std::string& strCmdMsg,std::string& strRetMsg);
	//输入通道号和Ts流文件节点地址，得到访问该流的socket句柄信息
	bool GetTsReceiveHandle(int deviceID,const std::string& strAddress,ACE_SOCK_Stream& streamHandle);
	//输入Ts流文件地址，得到访问该流的socket句柄信息
	bool GetTsReceiveHandle(const std::string& strAddress,ACE_SOCK_Stream& streamHandle);
	//判断该通道是否锁定
	bool CheckDeviceIDLock(int deviceID);
	//判断该频率是否能锁定
	bool CheckFreqLock(int freq);

	bool Start();
	
	//获取指定通道信道指标参数
//	bool GetRadioTunerQulity(int deviceID,VSTuner_Quality_Result_Obj& radiotunQ);

	//设置卡时间
	bool SetSysTime(int deviceID);	
	//设置板卡的报警灵敏度
	bool setThresholdInfo(int deviceID);
	bool getQualityInfoFor6U(int deviceID, VSTuner_Quality_Result_Obj& retObj);			//获取指标
	bool getAllQualityInfoFor6UInCard(int deviceID, VSTuner_AllQuality_Result_Obj& retObj);			//获取全部通道指标
	bool getChanFixInfoFor6U(int deviceID);																			//获取单通道频道锁定状态
	bool getAllChanFixInfoFor6UInCard(int deviceID, VSTuner_AllChanFix_Result_Obj& retObj);							//获取一张板卡的全部频道锁定状态
	bool LockFreqFor6U(int deviceID, LockFreq_Handle lockFreqArr, VSScan_Result_Obj& retObj);
	bool defaultSpectRum(int deviceid, std::vector<int>& freqVec);
private:
	void Init();
	void UnInit();
private:
	std::map<int,DeviceAccess*> deviceAccessMap;
	ACE_Thread_Mutex getTsMutex;
};

typedef ACE_Singleton<DeviceAccessMgr,ACE_Mutex>  DEVICEACCESSMGR;