#pragma once

#include "DeviceRelatedTask.h"
using namespace std;

class SpectrumScanTask : public DeviceRelatedTask
{
public:
	SpectrumScanTask();
	SpectrumScanTask(std::string strXML);
	virtual ~SpectrumScanTask();
public:
	virtual void Run(void);
	virtual std::string GetObjectName();
	virtual std::string GetTaskName();
	virtual bool IsRealTimeTask(){return true;};	//实时任务

	int CreateRADIOTcpData(string RtnXML,unsigned char* TcpData,int size_n);
	int CreateTVTcpData(string RtnXML,unsigned char* TcpData,int size_n);

private:
	float mStartFreq;               //频谱扫描的开始频点
	float mEndFreq;                 //频谱扫描的结束频点
	float mStepFreq;                //频谱扫描的步长
	float mNowStartFreq;            //当前频谱段的开始频点
	float mNowEndFreq;              //当前频谱段的结束频点
	float mNowFreq;                 //当前频谱段内的当前的频点
	float mVbw;
	float mRbw;
	float mReferDbu;
	int mIntevalLen;                //每段的长度
	string mTaskType;
	
};